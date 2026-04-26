#pragma once

#include <string>
#include <vector>

namespace openylm::cli {

enum class CommandMode {
    Compile,
    Test,
    Run,
    Debug,
    Inspect,
    Pause,
    Resume,
    ListApps,
    RuntimeView,
};

struct CommandLine {
    CommandMode mode = CommandMode::Compile;
    std::string inputPath;
    std::string outputPath;
    bool debug = false;
    bool resolveModules = false;
    bool bytecodeDebug = false;
    bool debugBytecode = false;
    bool jitEnable = false;
    bool validateOnly = false;
    bool strict = false;
    bool runSafe = false;
    bool testModeForInput = false;
    std::vector<std::string> runInputs;
    std::string appId;
};

bool parseCommandLine(int argc, char** argv, CommandLine& out, std::string& error);

} // namespace openylm::cli
