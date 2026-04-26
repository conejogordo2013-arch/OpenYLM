#include "debugger.h"

namespace openylm::debug {

Debugger::Debugger(vm::MultiAppRuntime& runtime) : runtime_(runtime) {}

bool Debugger::step(const std::string& appId, std::string& error) {
    return runtime_.stepApp(appId, error);
}

bool Debugger::addBreakpoint(const std::string& appId, std::size_t instructionPc, std::string& error) {
    if (!runtime_.addBreakpoint(appId, instructionPc)) {
        error = "app not found: " + appId;
        return false;
    }
    return true;
}

std::optional<vm::AppStats> Debugger::inspect(const std::string& appId) const {
    return runtime_.inspectApp(appId);
}

} // namespace openylm::debug
