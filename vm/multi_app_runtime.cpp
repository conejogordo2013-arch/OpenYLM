#include "multi_app_runtime.h"

namespace openylm::vm {

MultiAppRuntime::MultiAppRuntime(host::HostBridge& hostBridge) : hostBridge_(hostBridge) {}

void MultiAppRuntime::setDebugTrace(bool enabled) {
    debugTrace_ = enabled;
}

void MultiAppRuntime::loadApp(const std::string& appId, const bytecode::Program& program) {
    apps_.push_back(AppContext{appId, program, 0, {}, false});
}

bool MultiAppRuntime::runAll(std::string& error) {
    VM vm(hostBridge_);
    vm.setDebugTrace(debugTrace_);

    while (true) {
        const auto order = scheduler_.nextRoundRobin(apps_);
        if (order.empty()) {
            break;
        }

        for (const auto index : order) {
            if (!vm.executeStep(apps_[index], error)) {
                return false;
            }
        }
    }

    return true;
}

} // namespace openylm::vm
