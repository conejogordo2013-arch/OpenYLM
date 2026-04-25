#pragma once

#include "../bytecode/bytecode.h"
#include "../host/host_bridge.h"
#include "app_context.h"

#include <string>

namespace openylm::vm {

class VM {
public:
    explicit VM(host::HostBridge& hostBridge);
    void setDebugTrace(bool enabled);

    bool execute(const bytecode::Program& program, std::string& error);
    bool executeStep(AppContext& context, std::string& error);

private:
    host::HostBridge& hostBridge_;
    bool debugTrace_ = false;
};

} // namespace openylm::vm
