#pragma once

#include <string>

namespace openylm::cli {

struct CommandLine {
    std::string inputPath;
    std::string outputPath;
    bool debug = false;
    bool testMode = false;
    bool resolveModules = false;
    bool bytecodeDebug = false;
    bool debugBytecode = false;
};

bool parseCommandLine(int argc, char** argv, CommandLine& out, std::string& error);

} // namespace openylm::cli
