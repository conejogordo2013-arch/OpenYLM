#include "permissions.h"

namespace openylm::security {

bool isHostCallAllowed(const vm::AppContext& app, const std::string& functionName) {
    if (!app.permissions.allowHostCalls) {
        return false;
    }

    if (!app.sandbox.active) {
        return true;
    }

    if (app.sandbox.allowedHostCalls.empty()) {
        return true;
    }

    return app.sandbox.allowedHostCalls.count(functionName) > 0;
}

} // namespace openylm::security
