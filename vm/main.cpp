#include "../bytecode/bytecode.h"
#include "../host/host_registry.h"
#include "multi_app_runtime.h"
#include "vm.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: yvm <program.ylc> [program2.ylc ...] [--debug]\n";
        return 1;
    }

    bool debug = false;
    std::vector<std::string> files;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--debug") {
            debug = true;
        } else {
            files.push_back(arg);
        }
    }

    openylm::host::HostRegistry registry;
    openylm::host::registerDefaultRuntimeHosts(registry);

    std::string error;
    if (files.size() == 1) {
        openylm::bytecode::Program program;
        if (!openylm::bytecode::deserializeFromFile(files[0], program, error)) {
            std::cerr << error << '\n';
            return 1;
        }

        openylm::vm::VM vm(registry);
        vm.setDebugTrace(debug);
        if (!vm.execute(program, error)) {
            std::cerr << error << '\n';
            return 1;
        }
        return 0;
    }

    openylm::vm::MultiAppRuntime runtime(registry);
    runtime.setDebugTrace(debug);

    for (const auto& file : files) {
        openylm::bytecode::Program program;
        if (!openylm::bytecode::deserializeFromFile(file, program, error)) {
            std::cerr << error << '\n';
            return 1;
        }
        runtime.loadApp(file, program);
    }

    if (!runtime.runAll(error)) {
        std::cerr << error << '\n';
        return 1;
    }

    return 0;
}
