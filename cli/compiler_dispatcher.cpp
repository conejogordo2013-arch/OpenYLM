#include "compiler_dispatcher.h"

#include "../bytecode/bytecode.h"
#include "../compiler/error.h"
#include "../compiler/ir_to_bytecode.h"
#include "../compiler/module_resolver.h"
#include "../compiler/yaml_to_ir.h"
#include "../ir/ir.h"
#include "../runtime/test_runner.h"
#include "file_loader.h"
#include "debug_output.h"

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
        if (it == resolved.units.end()) {
            continue;
        }
        combined.insert(combined.end(), it->second.program.begin(), it->second.program.end());
    }

    return true;
}

} // namespace

bool dispatchCompile(const CommandLine& cmd, std::string& error) {
    if (cmd.testMode) {
        int failed = 0;
        std::string report;
        const bool ok = runtime::runAllTests("tests", cmd.debug, report, failed);
        std::cout << report;
        if (!ok) {
            error = "test failures: " + std::to_string(failed);
        }
        return ok;
    }

    const auto inExt = extensionOf(resolveEntryPath(cmd.inputPath));
    const auto outExt = extensionOf(cmd.outputPath);

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
        if (cmd.debug) {
            std::cout << "[debug] writing IR to " << cmd.outputPath << '\n';
        }
        if (!ir::serializeToFile(program, cmd.outputPath, error)) {
            return false;
        }
        if (cmd.bytecodeDebug) {
            bytecode::Program tmp;
            std::vector<compiler::CompilerError> errors;
            if (!compiler::irToBytecode(program, tmp, cmd.debug, errors)) {
                error = compiler::formatError(errors.front());
                return false;
            }
            printDebugBytecode(tmp);
        }
        return true;
    }

    if (inExt == ".yl" && outExt == ".ylc") {
        ir::Program irProgram;
        if (!resolveAndBuildCombinedIr(cmd.inputPath, cmd.debug, cmd.resolveModules, irProgram, error)) {
            return false;
        }

        if (cmd.debug) {
            std::cout << "[debug] IR nodes: " << irProgram.size() << '\n';
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

        if (cmd.debug) {
            std::cout << "[debug] loaded IR nodes: " << irProgram.size() << '\n';
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
