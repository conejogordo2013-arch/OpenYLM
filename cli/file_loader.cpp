#include "file_loader.h"

namespace openylm::cli {

std::string extensionOf(const std::string& path) {
    const auto idx = path.find_last_of('.');
    if (idx == std::string::npos) {
        return {};
    }
    return path.substr(idx);
}

} // namespace openylm::cli
