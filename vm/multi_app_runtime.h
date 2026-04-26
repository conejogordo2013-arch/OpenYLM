#pragma once

#include "scheduler.h"
#include "../jit/jit_engine.h"
#include "vm.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace openylm::vm {

struct AppStats {
    std::string appId;
    AppExecutionState state = AppExecutionState::Runnable;
    std::size_t pc = 0;
    std::size_t stackSize = 0;
    std::size_t heapSize = 0;
    std::uint64_t executedInstructions = 0;
    std::uint64_t consumedSlices = 0;
    std::uint8_t lastOpcode = 0;
    std::size_t memoryUsageBytes = 0;
    bool sandboxActive = false;
    std::string lastError;
};

class MultiAppRuntime {
public:
    explicit MultiAppRuntime(host::HostBridge& hostBridge);

    void setDebugTrace(bool enabled);
    void loadApp(const std::string& appId, const bytecode::Program& program);
    void setJitEnabled(bool enabled);
    bool runAll(std::string& error);
    bool stepApp(const std::string& appId, std::string& error);
    bool addBreakpoint(const std::string& appId, std::size_t instructionPc);

    bool pauseApp(const std::string& appId);
    bool resumeApp(const std::string& appId);
    bool stopApp(const std::string& appId);
    std::vector<std::string> listApps() const;
    std::optional<AppStats> inspectApp(const std::string& appId) const;
    std::vector<AppStats> monitorSnapshot() const;

private:
    AppContext* findApp(const std::string& appId);
    const AppContext* findApp(const std::string& appId) const;

    host::HostBridge& hostBridge_;
    Scheduler scheduler_;
    std::vector<AppContext> apps_;
    bool debugTrace_ = false;
    jit::JitEngine jit_;
};

} // namespace openylm::vm
