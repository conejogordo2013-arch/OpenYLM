#include "sandbox.h"

namespace openylm::security {

bool validateRuntimeLimits(vm::AppContext& app, std::string& error) {
    if (!app.sandbox.active) {
        return true;
    }

    if (app.estimatedMemoryUsageBytes() > app.sandbox.memoryLimitBytes) {
        app.state = vm::AppExecutionState::Faulted;
        error = "sandbox memory limit exceeded for app: " + app.appId;
        return false;
    }

    return true;
}

std::size_t allowedInstructionsThisTick(const vm::AppContext& app, std::size_t schedulerSlice) {
    if (!app.sandbox.active) {
        return schedulerSlice;
    }
    if (app.sandbox.cpuLimitPerTick == 0) {
        return 1;
    }
    return app.sandbox.cpuLimitPerTick < schedulerSlice ? app.sandbox.cpuLimitPerTick : schedulerSlice;
}

} // namespace openylm::security
