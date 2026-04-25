#include "debug_output.h"

#include <iostream>

namespace openylm::cli {

void printDebugBytecode(const bytecode::Program& program) {
    std::cout << bytecode::binaryToDebug(program.code) << '\n';
}

} // namespace openylm::cli
