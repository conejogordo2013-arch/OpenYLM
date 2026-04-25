#include "host_registry.h"

#include <iostream>

namespace openylm::host {

void HostRegistry::registerFunction(const std::string& name, Callback callback) {
    callbacks_[name] = std::move(callback);
}

bool HostRegistry::invoke(const std::string& name, const std::vector<std::string>& args) {
    const auto it = callbacks_.find(name);
    if (it == callbacks_.end()) {
        return false;
    }
    it->second(args);
    return true;
}

void registerDefaultRuntimeHosts(HostRegistry& registry) {
    registry.registerFunction("log", [](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::cout << args[0] << '\n';
        }
    });
    registry.registerFunction("print", [](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::cout << args[0] << '\n';
        }
    });
    auto addFn = [](const std::vector<std::string>& args) {
        if (args.size() >= 2) {
            const int a = std::stoi(args[0]);
            const int b = std::stoi(args[1]);
            std::cout << (a + b) << '\n';
        }
    };
    registry.registerFunction("add", addFn);
    registry.registerFunction("math.add", addFn);
}

} // namespace openylm::host
