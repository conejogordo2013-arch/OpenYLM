#include "multi_app_runtime.h"

#include "../security/sandbox.h"

namespace openylm::vm {

MultiAppRuntime::MultiAppRuntime(host::HostBridge& hostBridge) : hostBridge_(hostBridge), scheduler_(8) {}

void MultiAppRuntime::setDebugTrace(bool enabled) {
    debugTrace_ = enabled;
}

void MultiAppRuntime::setJitEnabled(bool enabled) {
    jit_.setEnabled(enabled);
}

void MultiAppRuntime::loadApp(const std::string& appId, const bytecode::Program& program) {
    AppContext context;
    context.appId = appId;
    context.program = program;
    context.sandbox.allowedHostCalls.insert("log");
    apps_.push_back(context);
}

bool MultiAppRuntime::runAll(std::string& error) {
    VM vm(hostBridge_);
    vm.setDebugTrace(debugTrace_);
    vm.setJitEngine(&jit_);

    while (true) {
        const auto order = scheduler_.nextRoundRobin(apps_);
        if (order.empty()) {
            break;
        }

        for (const auto index : order) {
            auto& app = apps_[index];
            const std::size_t sliceBudget = security::allowedInstructionsThisTick(app, scheduler_.timeSliceInstructions());
            for (std::size_t i = 0; i < sliceBudget; ++i) {
                if (app.state != AppExecutionState::Runnable) {
                    break;
                }
                if (!vm.executeStep(app, error)) {
                    return false;
                }
                if (app.breakpoints.count(app.pc) > 0) {
                    app.state = AppExecutionState::Paused;
                    break;
                }
            }
            ++app.totalSlices;
        }
    }

    return true;
}


bool MultiAppRuntime::addBreakpoint(const std::string& appId, std::size_t instructionPc) {
    auto* app = findApp(appId);
    if (app == nullptr) {
        return false;
    }
    app->breakpoints.insert(instructionPc);
    return true;
}

bool MultiAppRuntime::stepApp(const std::string& appId, std::string& error) {
    auto* app = findApp(appId);
    if (app == nullptr) {
        error = "app not found: " + appId;
        return false;
    }

    if (app->isTerminated()) {
        return true;
    }

    const auto restore = app->state;
    app->state = AppExecutionState::Runnable;
    VM vm(hostBridge_);
    vm.setDebugTrace(debugTrace_);
    vm.setJitEngine(&jit_);
    if (!vm.executeStep(*app, error)) {
        return false;
    }

    if (!app->isTerminated()) {
        app->state = AppExecutionState::Paused;
    }

    if (restore == AppExecutionState::Paused && !app->isTerminated()) {
        app->state = AppExecutionState::Paused;
    }

    return true;
}

bool MultiAppRuntime::pauseApp(const std::string& appId) {
    auto* app = findApp(appId);
    if (app == nullptr || app->isTerminated()) {
        return false;
    }
    app->state = AppExecutionState::Paused;
    return true;
}

bool MultiAppRuntime::resumeApp(const std::string& appId) {
    auto* app = findApp(appId);
    if (app == nullptr || app->isTerminated()) {
        return false;
    }
    app->state = AppExecutionState::Runnable;
    return true;
}

bool MultiAppRuntime::stopApp(const std::string& appId) {
    auto* app = findApp(appId);
    if (app == nullptr || app->isTerminated()) {
        return false;
    }
    app->state = AppExecutionState::Stopped;
    return true;
}

std::vector<std::string> MultiAppRuntime::listApps() const {
    std::vector<std::string> ids;
    ids.reserve(apps_.size());
    for (const auto& app : apps_) {
        ids.push_back(app.appId);
    }
    return ids;
}

std::optional<AppStats> MultiAppRuntime::inspectApp(const std::string& appId) const {
    const auto* app = findApp(appId);
    if (app == nullptr) {
        return std::nullopt;
    }

    AppStats stats;
    stats.appId = app->appId;
    stats.state = app->state;
    stats.pc = app->pc;
    stats.stackSize = app->stack.size();
    stats.heapSize = app->heap.size();
    stats.executedInstructions = app->totalInstructions;
    stats.consumedSlices = app->totalSlices;
    stats.lastOpcode = app->lastOpcode;
    stats.memoryUsageBytes = app->estimatedMemoryUsageBytes();
    stats.sandboxActive = app->sandbox.active;
    stats.lastError = app->lastError;
    return stats;
}

std::vector<AppStats> MultiAppRuntime::monitorSnapshot() const {
    std::vector<AppStats> snapshot;
    snapshot.reserve(apps_.size());
    for (const auto& app : apps_) {
        snapshot.push_back(AppStats{app.appId,
                                    app.state,
                                    app.pc,
                                    app.stack.size(),
                                    app.heap.size(),
                                    app.totalInstructions,
                                    app.totalSlices,
                                    app.lastOpcode,
                                    app.estimatedMemoryUsageBytes(),
                                    app.sandbox.active,
                                    app.lastError});
    }
    return snapshot;
}

AppContext* MultiAppRuntime::findApp(const std::string& appId) {
    for (auto& app : apps_) {
        if (app.appId == appId) {
            return &app;
        }
    }
    return nullptr;
}

const AppContext* MultiAppRuntime::findApp(const std::string& appId) const {
    for (const auto& app : apps_) {
        if (app.appId == appId) {
            return &app;
        }
    }
    return nullptr;
}

} // namespace openylm::vm
