#include "command_parser.h"

namespace openylm::cli {

bool parseCommandLine(int argc, char** argv, CommandLine& out, std::string& error) {
    out = CommandLine{};

    if (argc == 2 && std::string(argv[1]) == "--test") {
        out.testMode = true;
        return true;
    }

    if (argc < 2) {
        error = "usage: yamll <input.(yl|ir|project_dir)> -o <output.(ir|ylc)> [--debug] [--resolve-modules] [--bytecode-debug] [--debug-bytecode] | yamll --test";
        return false;
    }

    out.inputPath = argv[1];

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            out.outputPath = argv[++i];
        } else if (arg == "--debug") {
            out.debug = true;
        } else if (arg == "--test") {
            out.testMode = true;
        } else if (arg == "--resolve-modules") {
            out.resolveModules = true;
        } else if (arg == "--bytecode-debug") {
            out.bytecodeDebug = true;
        } else if (arg == "--debug-bytecode") {
            out.debugBytecode = true;
        } else {
            error = "unknown argument: " + arg;
            return false;
        }
    }

    if (!out.testMode && out.outputPath.empty() && !out.bytecodeDebug) {
        error = "missing required -o <output>";
        return false;
    }

    return true;
}

} // namespace openylm::cli
