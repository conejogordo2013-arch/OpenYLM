#include "bytecode.h"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace openylm::bytecode {

std::string binaryToDebug(const std::vector<std::uint8_t>& bytes) {
    std::ostringstream out;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        out << "/x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]) << "/";
        if (i + 1 < bytes.size()) {
            out << ' ';
        }
    }
    return out.str();
}

bool debugToBinary(const std::string& text, std::vector<std::uint8_t>& out, std::string& error) {
    out.clear();
    std::size_t i = 0;

    while (i < text.size()) {
        while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) {
            ++i;
        }
        if (i >= text.size()) {
            break;
        }
        if (i + 4 >= text.size() || text[i] != '/' || text[i + 1] != 'x') {
            error = "invalid debug token prefix";
            return false;
        }

        const std::string hex = text.substr(i + 2, 2);
        if (i + 4 >= text.size() || text[i + 4] != '/') {
            error = "invalid debug token suffix";
            return false;
        }

        try {
            const auto value = static_cast<std::uint8_t>(std::stoul(hex, nullptr, 16));
            out.push_back(value);
        } catch (...) {
            error = "invalid hex value in debug bytecode";
            return false;
        }
        i += 5;
    }

    return true;
}

} // namespace openylm::bytecode
