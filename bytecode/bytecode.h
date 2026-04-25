#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace openylm::bytecode {

enum class OpCode : std::uint8_t {
    Nop = 0x00,
    ConstString = 0x01,
    ConstNumber = 0x02,
    Add = 0x03,
    Sub = 0x04,
    Mul = 0x05,
    Div = 0x06,
    LoadVar = 0x07,
    CallHost = 0x08,
    End = 0xFF,
};

struct Program {
    std::vector<std::string> stringTable;
    std::vector<std::uint8_t> code;
};

std::uint32_t internString(Program& program, const std::string& value);

void emitU8(Program& program, std::uint8_t value);
void emitU32(Program& program, std::uint32_t value);
std::uint8_t readU8(const Program& program, std::size_t& pc);
std::uint32_t readU32(const Program& program, std::size_t& pc);

bool serializeToFile(const Program& program, const std::string& path, std::string& error);
bool deserializeFromFile(const std::string& path, Program& out, std::string& error);

std::string binaryToDebug(const std::vector<std::uint8_t>& bytes);
bool debugToBinary(const std::string& text, std::vector<std::uint8_t>& out, std::string& error);

} // namespace openylm::bytecode
