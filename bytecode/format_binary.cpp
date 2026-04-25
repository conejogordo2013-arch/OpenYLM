#include "bytecode.h"

#include <cstring>
#include <fstream>

namespace openylm::bytecode {

bool serializeToFile(const Program& program, const std::string& path, std::string& error) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        error = "unable to open bytecode output file";
        return false;
    }

    const char magic[4] = {'Y', 'L', 'C', '4'};
    out.write(magic, 4);

    const std::uint32_t strCount = static_cast<std::uint32_t>(program.stringTable.size());
    out.write(reinterpret_cast<const char*>(&strCount), sizeof(strCount));
    for (const auto& str : program.stringTable) {
        const std::uint32_t len = static_cast<std::uint32_t>(str.size());
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        out.write(str.data(), static_cast<std::streamsize>(str.size()));
    }

    const std::uint32_t codeSize = static_cast<std::uint32_t>(program.code.size());
    out.write(reinterpret_cast<const char*>(&codeSize), sizeof(codeSize));
    out.write(reinterpret_cast<const char*>(program.code.data()), static_cast<std::streamsize>(program.code.size()));

    if (!out) {
        error = "failed to write bytecode file";
        return false;
    }

    return true;
}

bool deserializeFromFile(const std::string& path, Program& out, std::string& error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "unable to open bytecode input file";
        return false;
    }

    char magic[4];
    in.read(magic, 4);
    if (in.gcount() != 4 || std::memcmp(magic, "YLC4", 4) != 0) {
        error = "invalid bytecode header";
        return false;
    }

    Program program;

    std::uint32_t strCount = 0;
    in.read(reinterpret_cast<char*>(&strCount), sizeof(strCount));
    if (!in) {
        error = "failed to read string count";
        return false;
    }

    program.stringTable.reserve(strCount);
    for (std::uint32_t i = 0; i < strCount; ++i) {
        std::uint32_t len = 0;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (!in) {
            error = "failed to read string length";
            return false;
        }
        std::string str(len, '\0');
        in.read(&str[0], static_cast<std::streamsize>(len));
        if (!in) {
            error = "failed to read string data";
            return false;
        }
        program.stringTable.push_back(std::move(str));
    }

    std::uint32_t codeSize = 0;
    in.read(reinterpret_cast<char*>(&codeSize), sizeof(codeSize));
    if (!in) {
        error = "failed to read code size";
        return false;
    }
    program.code.resize(codeSize);
    in.read(reinterpret_cast<char*>(program.code.data()), static_cast<std::streamsize>(codeSize));
    if (!in) {
        error = "failed to read code bytes";
        return false;
    }

    out = std::move(program);
    return true;
}

} // namespace openylm::bytecode
