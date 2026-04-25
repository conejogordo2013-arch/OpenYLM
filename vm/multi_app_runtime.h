#pragma once

#include "scheduler.h"
#include "vm.h"

#include <string>
#include <vector>

namespace openylm::vm {

class MultiAppRuntime {
public:
    explicit MultiAppRuntime(host::HostBridge& hostBridge);

    void setDebugTrace(bool enabled);
    void loadApp(const std::string& appId, const bytecode::Program& program);
    bool runAll(std::string& error);

private:
    host::HostBridge& hostBridge_;
    Scheduler scheduler_;
    std::vector<AppContext> apps_;
    bool debugTrace_ = false;
};

} // namespace openylm::vm
