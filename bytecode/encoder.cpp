#include "bytecode.h"

namespace openylm::bytecode {

std::uint32_t internString(Program& program, const std::string& value) {
    for (std::uint32_t i = 0; i < program.stringTable.size(); ++i) {
        if (program.stringTable[i] == value) {
            return i;
        }
    }
    program.stringTable.push_back(value);
    return static_cast<std::uint32_t>(program.stringTable.size() - 1);
}

void emitU8(Program& program, std::uint8_t value) {
    program.code.push_back(value);
}

void emitU32(Program& program, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        program.code.push_back(static_cast<std::uint8_t>((value >> (8 * i)) & 0xFF));
    }
}

} // namespace openylm::bytecode
