#pragma once

#include "../bytecode/bytecode.h"
#include "../host/host_bridge.h"
#include "../jit/jit_engine.h"
#include "app_context.h"

#include <string>

namespace openylm::vm {

class VM {
public:
    explicit VM(host::HostBridge& hostBridge);
    void setDebugTrace(bool enabled);
    void setJitEngine(jit::JitEngine* jitEngine);

    bool execute(const bytecode::Program& program, std::string& error);
    bool executeStep(AppContext& context, std::string& error);

private:
    bool runArithmetic(bytecode::OpCode opcode, AppContext& context, std::string& error, bool fastPath);

    host::HostBridge& hostBridge_;
    bool debugTrace_ = false;
    jit::JitEngine* jitEngine_ = nullptr;
};

} // namespace openylm::vm
