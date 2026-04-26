#include "command_parser.h"

namespace openylm::cli {

bool parseCommandLine(int argc, char** argv, CommandLine& out, std::string& error) {
    out = CommandLine{};

    if (argc < 2) {
        error = "usage: yamll <input.yl> [--test|--validate|--strict|--run-safe] | yamll --run <app.yl> [--jit-enable]";
        return false;
    }

    const std::string first = argv[1];
    if (first == "--test") {
        out.mode = CommandMode::Test;
        return true;
    }
    if (first == "--run" || first == "--debug" || first == "--runtime-view" || first == "--inspect") {
        out.mode = (first == "--run") ? CommandMode::Run : (first == "--debug" ? CommandMode::Debug : CommandMode::RuntimeView);
        out.debug = (out.mode == CommandMode::Debug);

        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--jit-enable") out.jitEnable = true;
            else if (arg == "--run-safe") out.runSafe = true;
            else if (arg == "--strict") out.strict = true;
            else out.runInputs.push_back(arg);
        }

        if (out.runInputs.empty()) {
            error = "missing app path for run/debug/runtime-view";
            return false;
        }
        return true;
    }
    if (first == "--pause" || first == "--resume") {
        if (argc < 3) {
            error = "missing app id";
            return false;
        }
        out.mode = first == "--pause" ? CommandMode::Pause : CommandMode::Resume;
        out.appId = argv[2];
        return true;
    }
    if (first == "--list-apps") {
        out.mode = CommandMode::ListApps;
        return true;
    }

    out.inputPath = argv[1];

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) out.outputPath = argv[++i];
        else if (arg == "--debug") out.debug = true;
        else if (arg == "--resolve-modules") out.resolveModules = true;
        else if (arg == "--bytecode-debug") out.bytecodeDebug = true;
        else if (arg == "--debug-bytecode") out.debugBytecode = true;
        else if (arg == "--validate") out.validateOnly = true;
        else if (arg == "--strict") out.strict = true;
        else if (arg == "--run-safe") out.runSafe = true;
        else if (arg == "--test") out.testModeForInput = true;
        else {
            error = "unknown argument: " + arg;
            return false;
        }
    }

    if (out.testModeForInput) {
        out.mode = CommandMode::Test;
        return true;
    }

    if (!out.validateOnly && out.outputPath.empty() && !out.bytecodeDebug) {
        error = "missing required -o <output>";
        return false;
    }

    return true;
}

} // namespace openylm::cli
