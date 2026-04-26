#include "compiler_dispatcher.h"

#include "../bytecode/bytecode.h"
#include "../compiler/error.h"
#include "../compiler/ir_to_bytecode.h"
#include "../compiler/module_resolver.h"
#include "../compiler/yaml_to_ir.h"
#include "../debug/debugger.h"
#include "../host/host_registry.h"
#include "../inspector/runtime_view.h"
#include "../ir/ir.h"
#include "../runtime/test_runner.h"
#include "../vm/multi_app_runtime.h"
#include "debug_output.h"
#include "file_loader.h"

#include <filesystem>
#include <iostream>

namespace openylm::cli {
namespace {

std::string resolveEntryPath(const std::string& input) {
    namespace fs = std::filesystem;
    fs::path p(input);
    if (fs::is_directory(p)) {
        return (p / "main.yl").string();
    }
    return input;
}

bool resolveAndBuildCombinedIr(const std::string& inputPath,
                               bool debug,
                               bool showResolved,
                               openylm::ir::Program& combined,
                               std::string& error) {
    std::vector<compiler::CompilerError> errors;
    compiler::ResolvedProject resolved;

    const auto entry = resolveEntryPath(inputPath);
    if (!compiler::resolveProject(entry, debug, resolved, errors)) {
        error = errors.empty() ? "module resolution failed" : compiler::formatError(errors.front());
        return false;
    }

    if (showResolved) {
        for (const auto& file : resolved.loadOrder) {
            std::cout << "[resolve] " << file << '\n';
        }
    }

    combined.clear();
    for (const auto& file : resolved.loadOrder) {
        const auto it = resolved.units.find(file);
        if (it != resolved.units.end()) {
            combined.insert(combined.end(), it->second.program.begin(), it->second.program.end());
        }
    }

    return true;
}

bool compileYamlToBytecode(const CommandLine& cmd, const std::string& input, bytecode::Program& out, std::string& error) {
    ir::Program irProgram;
    if (!resolveAndBuildCombinedIr(input, cmd.debug, cmd.resolveModules, irProgram, error)) {
        return false;
    }
    std::vector<compiler::CompilerError> errors;
    if (cmd.strict && irProgram.empty()) {
        error = "strict mode: empty program is not allowed";
        return false;
    }
    if (!compiler::irToBytecode(irProgram, out, cmd.debug, errors)) {
        error = compiler::formatError(errors.front());
        return false;
    }
    return true;
}

std::string stateToString(vm::AppExecutionState state) {
    switch (state) {
    case vm::AppExecutionState::Runnable: return "runnable";
    case vm::AppExecutionState::Paused: return "paused";
    case vm::AppExecutionState::Completed: return "completed";
    case vm::AppExecutionState::Stopped: return "stopped";
    case vm::AppExecutionState::Faulted: return "faulted";
    }
    return "unknown";
}

bool dispatchRunLike(const CommandLine& cmd, std::string& error) {
    host::HostRegistry registry;
    host::registerDefaultRuntimeHosts(registry);

    vm::MultiAppRuntime runtime(registry);
    runtime.setDebugTrace(cmd.mode == CommandMode::Debug);
    runtime.setJitEnabled(cmd.jitEnable && !cmd.runSafe);

    for (const auto& input : cmd.runInputs) {
        bytecode::Program program;
        if (!compileYamlToBytecode(cmd, input, program, error)) {
            return false;
        }
        runtime.loadApp(resolveEntryPath(input), program);
    }

    if (cmd.mode == CommandMode::Debug) {
        debug::Debugger debugger(runtime);
        for (const auto& appId : runtime.listApps()) {
            std::string stepError;
            if (!debugger.step(appId, stepError)) {
                error = stepError;
                return false;
            }
        }
    }

    if (!runtime.runAll(error)) {
        if (cmd.mode == CommandMode::RuntimeView || cmd.mode == CommandMode::Debug) {
            std::cout << inspector::renderRuntimeView(runtime);
        }
        return false;
    }

    if (cmd.mode == CommandMode::RuntimeView) {
        std::cout << inspector::renderRuntimeView(runtime);
        return true;
    }

    for (const auto& app : runtime.monitorSnapshot()) {
        std::cout << "[monitor] app=" << app.appId << " state=" << stateToString(app.state)
                  << " pc=" << app.pc << " cpu_instructions=" << app.executedInstructions
                  << " slices=" << app.consumedSlices << " stack=" << app.stackSize
                  << " heap=" << app.heapSize << " mem_bytes=" << app.memoryUsageBytes
                  << " sandbox=" << (app.sandboxActive ? "active" : "off") << '\n';
    }

    return true;
}

} // namespace

bool dispatchCompile(const CommandLine& cmd, std::string& error) {
    if (cmd.mode == CommandMode::Test) {
        if (!cmd.inputPath.empty()) {
            std::string report;
            const bool ok = runtime::runSingleTest(cmd.inputPath, cmd.debug, report);
            std::cout << report;
            if (!ok) {
                error = "test failure";
            }
            return ok;
        }
        int failed = 0;
        std::string report;
        const bool ok = runtime::runAllTests("tests", cmd.debug, report, failed);
        std::cout << report;
        if (!ok) {
            error = "test failures: " + std::to_string(failed);
        }
        return ok;
    }

    if (cmd.mode == CommandMode::Run || cmd.mode == CommandMode::Debug || cmd.mode == CommandMode::RuntimeView) {
        return dispatchRunLike(cmd, error);
    }

    if (cmd.mode == CommandMode::Pause || cmd.mode == CommandMode::Resume || cmd.mode == CommandMode::ListApps || cmd.mode == CommandMode::Inspect) {
        error = "process-local runtime control commands require a live runtime session; use --run/--debug/--runtime-view";
        return false;
    }

    const auto inExt = extensionOf(resolveEntryPath(cmd.inputPath));
    const auto outExt = extensionOf(cmd.outputPath);

    if (cmd.validateOnly) {
        ir::Program irProgram;
        if (!resolveAndBuildCombinedIr(cmd.inputPath, cmd.debug, cmd.resolveModules, irProgram, error)) {
            return false;
        }
        if (cmd.strict && irProgram.empty()) {
            error = "strict mode validation failed: empty program";
            return false;
        }
        std::cout << "[validate] OK " << cmd.inputPath << '\n';
        return true;
    }

    if (inExt == ".yl" && cmd.outputPath.empty() && cmd.bytecodeDebug) {
        ir::Program irProgram;
        if (!resolveAndBuildCombinedIr(cmd.inputPath, cmd.debug, cmd.resolveModules, irProgram, error)) {
            return false;
        }
        bytecode::Program bytecodeProgram;
        std::vector<compiler::CompilerError> errors;
        if (!compiler::irToBytecode(irProgram, bytecodeProgram, cmd.debug, errors)) {
            error = compiler::formatError(errors.front());
            return false;
        }
        printDebugBytecode(bytecodeProgram);
        return true;
    }

    if (inExt == ".yl" && outExt == ".ir") {
        ir::Program program;
        if (!resolveAndBuildCombinedIr(cmd.inputPath, cmd.debug, cmd.resolveModules, program, error)) {
            return false;
        }
        if (!ir::serializeToFile(program, cmd.outputPath, error)) {
            return false;
        }
        return true;
    }

    if (inExt == ".yl" && outExt == ".ylc") {
        ir::Program irProgram;
        if (!resolveAndBuildCombinedIr(cmd.inputPath, cmd.debug, cmd.resolveModules, irProgram, error)) {
            return false;
        }

        bytecode::Program bytecodeProgram;
        std::vector<compiler::CompilerError> errors;
        if (!compiler::irToBytecode(irProgram, bytecodeProgram, cmd.debug, errors)) {
            error = compiler::formatError(errors.front());
            return false;
        }
        if (!bytecode::serializeToFile(bytecodeProgram, cmd.outputPath, error)) {
            return false;
        }
        if (cmd.bytecodeDebug || cmd.debugBytecode) {
            printDebugBytecode(bytecodeProgram);
        }
        return true;
    }

    if (inExt == ".ir" && outExt == ".ylc") {
        ir::Program irProgram;
        if (!ir::deserializeFromFile(cmd.inputPath, irProgram, error)) {
            return false;
        }

        bytecode::Program bytecodeProgram;
        std::vector<compiler::CompilerError> errors;
        if (!compiler::irToBytecode(irProgram, bytecodeProgram, cmd.debug, errors)) {
            error = compiler::formatError(errors.front());
            return false;
        }
        if (!bytecode::serializeToFile(bytecodeProgram, cmd.outputPath, error)) {
            return false;
        }
        if (cmd.bytecodeDebug || cmd.debugBytecode) {
            printDebugBytecode(bytecodeProgram);
        }
        return true;
    }

    error = "unsupported conversion: " + inExt + " -> " + outExt;
    return false;
}

} // namespace openylm::cli
