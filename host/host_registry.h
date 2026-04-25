#pragma once

#include "host_bridge.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace openylm::host {

using Callback = std::function<void(const std::vector<std::string>&)>;

class HostRegistry : public HostBridge {
public:
    void registerFunction(const std::string& name, Callback callback);
    bool invoke(const std::string& name, const std::vector<std::string>& args) override;

private:
    std::unordered_map<std::string, Callback> callbacks_;
};

void registerDefaultRuntimeHosts(HostRegistry& registry);

} // namespace openylm::host
