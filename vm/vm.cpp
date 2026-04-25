#include "vm.h"

#include <iostream>

namespace openylm::vm {

VM::VM(host::HostBridge& hostBridge) : hostBridge_(hostBridge) {}

void VM::setDebugTrace(bool enabled) {
    debugTrace_ = enabled;
}

bool VM::execute(const bytecode::Program& program, std::string& error) {
    AppContext context{"single-app", program, 0, {}, false};
    while (!context.finished) {
        if (!executeStep(context, error)) {
            return false;
        }
    }
    return true;
}

bool VM::executeStep(AppContext& context, std::string& error) {
    if (context.finished) {
        return true;
    }

    if (context.pc >= context.program.code.size()) {
        context.finished = true;
        return true;
    }

    const std::size_t insPc = context.pc;
    const auto opcodeByte = bytecode::readU8(context.program, context.pc);
    const auto opcode = static_cast<bytecode::OpCode>(opcodeByte);

    if (debugTrace_) {
        std::cout << "APP=" << context.appId << " PC=" << insPc << " | OP=" << bytecode::binaryToDebug({opcodeByte}) << " | STACK=[";
        for (std::size_t i = 0; i < context.stack.size(); ++i) {
            std::cout << context.stack[i] << (i + 1 < context.stack.size() ? "," : "");
        }
        std::cout << "]";
    }

    switch (opcode) {
    case bytecode::OpCode::Nop:
        if (debugTrace_) std::cout << " | NOP\n";
        break;
    case bytecode::OpCode::ConstNumber: {
        const auto value = static_cast<int>(bytecode::readU32(context.program, context.pc));
        context.stack.push_back(value);
        if (debugTrace_) std::cout << " | CONST_NUMBER " << value << "\n";
        break;
    }
    case bytecode::OpCode::Add:
    case bytecode::OpCode::Sub:
    case bytecode::OpCode::Mul:
    case bytecode::OpCode::Div: {
        if (context.stack.size() < 2) {
            error = "arithmetic opcode needs two stack values";
            return false;
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
                error = "division by zero";
                return false;
            }
            context.stack.push_back(a / b);
        }
        if (debugTrace_) std::cout << " | ARITH\n";
        break;
    }
    case bytecode::OpCode::CallHost: {
        const auto hostId = bytecode::readU32(context.program, context.pc);
        const auto argc = bytecode::readU32(context.program, context.pc);
        if (hostId >= context.program.stringTable.size()) {
            error = "host id out of range";
            return false;
        }

        std::vector<std::string> args;
        args.reserve(argc);
        for (std::size_t i = 0; i < argc; ++i) {
            const auto argId = bytecode::readU32(context.program, context.pc);
            if (argId >= context.program.stringTable.size()) {
                error = "argument id out of range";
                return false;
            }
            args.push_back(context.program.stringTable[argId]);
        }

        if (!hostBridge_.invoke(context.program.stringTable[hostId], args)) {
            error = "host function not registered: " + context.program.stringTable[hostId];
            return false;
        }
        if (debugTrace_) std::cout << " | CALL_HOST\n";
        break;
    }
    case bytecode::OpCode::ConstString:
        (void)bytecode::readU32(context.program, context.pc);
        if (debugTrace_) std::cout << " | CONST_STRING\n";
        break;
    case bytecode::OpCode::LoadVar:
        (void)bytecode::readU32(context.program, context.pc);
        if (debugTrace_) std::cout << " | LOAD_VAR\n";
        break;
    case bytecode::OpCode::End:
        context.finished = true;
        if (debugTrace_) std::cout << " | END\n";
        break;
    }

    return true;
}

} // namespace openylm::vm
