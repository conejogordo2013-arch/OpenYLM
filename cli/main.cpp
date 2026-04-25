#include "command_parser.h"
#include "compiler_dispatcher.h"

#include <iostream>

int main(int argc, char** argv) {
    openylm::cli::CommandLine command;
    std::string error;

    if (!openylm::cli::parseCommandLine(argc, argv, command, error)) {
        std::cerr << error << '\n';
        return 1;
    }

    if (!openylm::cli::dispatchCompile(command, error)) {
        std::cerr << error << '\n';
        return 1;
    }

    return 0;
}
