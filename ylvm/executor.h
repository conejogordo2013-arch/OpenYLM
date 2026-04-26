#pragma once

#include "../vm/vm.h"

namespace openylm::ylvm {

class Executor {
public:
    explicit Executor(host::HostBridge& hostBridge) : vm_(hostBridge) {}

    void setDebug(bool enabled) { vm_.setDebugTrace(enabled); }
    bool executeSingleStep(vm::AppContext& context, std::string& error) { return vm_.executeStep(context, error); }

private:
    vm::VM vm_;
};

} // namespace openylm::ylvm
