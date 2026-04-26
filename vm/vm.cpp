#include "vm.h"

#include "../security/permissions.h"
#include "../security/sandbox.h"

#include <iostream>

namespace openylm::vm {
namespace {

bool fail(AppContext& context,
          std::string& error,
          RuntimeErrorType type,
          RuntimeSeverity severity,
          const std::string& message,
          std::size_t pc) {
    context.state = AppExecutionState::Faulted;
    RuntimeErrorDetail detail;
    detail.type = type;
    detail.message = message;
    detail.appId = context.appId;
    detail.pc = pc;
    detail.origin = context.appId;
    detail.severity = severity;
    context.lastError = detail.format();
    error = context.lastError;
    return false;
}

} // namespace

VM::VM(host::HostBridge& hostBridge) : hostBridge_(hostBridge) {}

void VM::setDebugTrace(bool enabled) {
    debugTrace_ = enabled;
}

void VM::setJitEngine(jit::JitEngine* jitEngine) {
    jitEngine_ = jitEngine;
}

bool VM::execute(const bytecode::Program& program, std::string& error) {
    AppContext context{"single-app", program};
    while (!context.isTerminated()) {
        if (!executeStep(context, error)) {
            return false;
        }
    }
    return context.state != AppExecutionState::Faulted;
}

bool VM::runArithmetic(bytecode::OpCode opcode, AppContext& context, std::string& error, bool /*fastPath*/) {
    if (context.stack.size() < 2) {
        return fail(context, error, RuntimeErrorType::RuntimeError, RuntimeSeverity::Error,
                    "arithmetic opcode needs two stack values", context.pc);
    }

    const int b = context.stack.back();
    context.stack.pop_back();
    const int a = context.stack.back();
    context.stack.pop_back();

    if (opcode == bytecode::OpCode::Add) context.stack.push_back(a + b);
    else if (opcode == bytecode::OpCode::Sub) context.stack.push_back(a - b);
    else if (opcode == bytecode::OpCode::Mul) context.stack.push_back(a * b);
    else {
        if (b == 0) {
            return fail(context, error, RuntimeErrorType::RuntimeError, RuntimeSeverity::Error,
                        "division by zero", context.pc);
        }
        context.stack.push_back(a / b);
    }

    if (context.stack.size() > 4096) {
        return fail(context, error, RuntimeErrorType::StackOverflowError, RuntimeSeverity::Fatal,
                    "stack depth exceeded limit", context.pc);
    }

    return true;
}

bool VM::executeStep(AppContext& context, std::string& error) {
    if (context.isTerminated() || context.state == AppExecutionState::Paused) {
        return true;
    }

    if (context.pc >= context.program.code.size()) {
        context.state = AppExecutionState::Completed;
        return true;
    }

    if (!security::validateRuntimeLimits(context, error)) {
        context.lastError = error;
        return false;
    }

    const std::size_t insPc = context.pc;
    bytecode::OpCode opcode = bytecode::OpCode::Nop;
    std::uint8_t opcodeByte = 0;
    try {
        opcodeByte = bytecode::readU8(context.program, context.pc);
        opcode = static_cast<bytecode::OpCode>(opcodeByte);
    } catch (const std::exception& ex) {
        return fail(context, error, RuntimeErrorType::InvalidBytecodeError, RuntimeSeverity::Fatal, ex.what(), insPc);
    }
    context.lastOpcode = opcodeByte;

    if (jitEngine_ != nullptr) {
        jitEngine_->observe(context, insPc);
    }
    const bool hotPath = jitEngine_ != nullptr && jitEngine_->isHot(context, insPc);

    if (debugTrace_) {
        std::cout << "APP=" << context.appId << " PC=" << insPc << " | OP=" << bytecode::binaryToDebug({opcodeByte}) << " | STACK=[";
        for (std::size_t i = 0; i < context.stack.size(); ++i) {
            std::cout << context.stack[i] << (i + 1 < context.stack.size() ? "," : "");
        }
        std::cout << "]";
        if (hotPath) std::cout << " | JIT=HOT";
    }

    switch (opcode) {
    case bytecode::OpCode::Nop:
        if (debugTrace_) std::cout << " | NOP\n";
        break;
    case bytecode::OpCode::ConstNumber: {
        try {
            const auto value = static_cast<int>(bytecode::readU32(context.program, context.pc));
            context.stack.push_back(value);
            if (context.stack.size() > 4096) {
                return fail(context, error, RuntimeErrorType::StackOverflowError, RuntimeSeverity::Fatal,
                            "stack depth exceeded limit", insPc);
            }
        } catch (const std::exception& ex) {
            return fail(context, error, RuntimeErrorType::InvalidBytecodeError, RuntimeSeverity::Fatal, ex.what(), insPc);
        }
        if (debugTrace_) std::cout << " | CONST_NUMBER\n";
        break;
    }
    case bytecode::OpCode::Add:
    case bytecode::OpCode::Sub:
    case bytecode::OpCode::Mul:
    case bytecode::OpCode::Div:
        if (!runArithmetic(opcode, context, error, hotPath)) {
            return false;
        }
        if (debugTrace_) std::cout << " | ARITH\n";
        break;
    case bytecode::OpCode::CallHost: {
        std::uint32_t hostId = 0;
        std::uint32_t argc = 0;
        try {
            hostId = bytecode::readU32(context.program, context.pc);
            argc = bytecode::readU32(context.program, context.pc);
        } catch (const std::exception& ex) {
            return fail(context, error, RuntimeErrorType::InvalidBytecodeError, RuntimeSeverity::Fatal, ex.what(), insPc);
        }

        if (hostId >= context.program.stringTable.size()) {
            return fail(context, error, RuntimeErrorType::InvalidBytecodeError, RuntimeSeverity::Fatal,
                        "host id out of range", insPc);
        }
        const auto& hostName = context.program.stringTable[hostId];
        if (!security::isHostCallAllowed(context, hostName)) {
            return fail(context, error, RuntimeErrorType::SandboxViolationError, RuntimeSeverity::Error,
                        "unauthorized host call: " + hostName, insPc);
        }

        std::vector<std::string> args;
        args.reserve(argc);
        for (std::size_t i = 0; i < argc; ++i) {
            try {
                const auto argId = bytecode::readU32(context.program, context.pc);
                if (argId >= context.program.stringTable.size()) {
                    return fail(context, error, RuntimeErrorType::InvalidBytecodeError, RuntimeSeverity::Fatal,
                                "argument id out of range", insPc);
                }
                args.push_back(context.program.stringTable[argId]);
            } catch (const std::exception& ex) {
                return fail(context, error, RuntimeErrorType::InvalidBytecodeError, RuntimeSeverity::Fatal, ex.what(), insPc);
            }
        }

        if (!hostBridge_.invoke(hostName, args)) {
            return fail(context, error, RuntimeErrorType::UndefinedHostCallError, RuntimeSeverity::Error,
                        "host function not registered: " + hostName, insPc);
        }
        if (debugTrace_) std::cout << " | CALL_HOST\n";
        break;
    }
    case bytecode::OpCode::ConstString:
    case bytecode::OpCode::LoadVar:
        try {
            (void)bytecode::readU32(context.program, context.pc);
        } catch (const std::exception& ex) {
            return fail(context, error, RuntimeErrorType::InvalidBytecodeError, RuntimeSeverity::Fatal, ex.what(), insPc);
        }
        if (debugTrace_) std::cout << " | META\n";
        break;
    case bytecode::OpCode::End:
        context.state = AppExecutionState::Completed;
        if (debugTrace_) std::cout << " | END\n";
        break;
    default:
        return fail(context, error, RuntimeErrorType::InvalidBytecodeError, RuntimeSeverity::Fatal,
                    "unknown opcode encountered", insPc);
    }

    if (!security::validateRuntimeLimits(context, error)) {
        context.lastError = error;
        return fail(context, error, RuntimeErrorType::MemoryLimitExceededError, RuntimeSeverity::Fatal, error, insPc);
    }

    ++context.totalInstructions;
    return true;
}

} // namespace openylm::vm
