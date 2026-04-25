#include "bytecode.h"

#include <stdexcept>

namespace openylm::bytecode {

std::uint8_t readU8(const Program& program, std::size_t& pc) {
    if (pc >= program.code.size()) {
        throw std::runtime_error("bytecode read out of range");
    }
    return program.code[pc++];
}

std::uint32_t readU32(const Program& program, std::size_t& pc) {
    std::uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        value |= static_cast<std::uint32_t>(readU8(program, pc)) << (8 * i);
    }
    return value;
}

} // namespace openylm::bytecode
