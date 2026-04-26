#include "runtime_view.h"

#include "../bytecode/bytecode.h"

#include <sstream>

namespace openylm::inspector {
namespace {

std::string stateToString(vm::AppExecutionState state) {
    switch (state) {
    case vm::AppExecutionState::Runnable:
        return "RUNNABLE";
    case vm::AppExecutionState::Paused:
        return "PAUSED";
    case vm::AppExecutionState::Completed:
        return "COMPLETED";
    case vm::AppExecutionState::Stopped:
        return "STOPPED";
    case vm::AppExecutionState::Faulted:
        return "FAULTED";
    }
    return "UNKNOWN";
}

} // namespace

std::string renderRuntimeView(const vm::MultiAppRuntime& runtime) {
    std::ostringstream out;
    out << "[OpenYLM Runtime Inspector]\n";
    for (const auto& app : runtime.monitorSnapshot()) {
        const bool faulted = app.state == vm::AppExecutionState::Faulted;
        out << "AppID: " << app.appId << "\n";
        out << "State: " << stateToString(app.state) << "\n";
        out << "PC: " << app.pc << (faulted ? "  <-- failing instruction" : "") << "\n";
        out << "OP: " << bytecode::binaryToDebug({static_cast<std::uint8_t>(app.lastOpcode)}) << "\n";
        out << "STACK: size=" << app.stackSize << "\n";
        out << "MEMORY: " << (app.memoryUsageBytes / 1024) << "KB\n";
        out << "SANDBOX: " << (app.sandboxActive ? "ACTIVE" : "DISABLED") << "\n";
        out << "CPU_INST: " << app.executedInstructions << "\n";
        out << "TIMELINE_SLICES: " << app.consumedSlices << "\n";
        if (!app.lastError.empty()) {
            out << "ERROR:\n" << app.lastError << "\n";
        }
        out << "\n";
    }
    return out.str();
}

} // namespace openylm::inspector
