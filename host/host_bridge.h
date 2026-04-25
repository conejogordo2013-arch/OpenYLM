#pragma once

#include <string>
#include <vector>

namespace openylm::host {

class HostBridge {
public:
    virtual ~HostBridge() = default;
    virtual bool invoke(const std::string& name, const std::vector<std::string>& args) = 0;
};

} // namespace openylm::host
