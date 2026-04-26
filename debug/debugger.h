#pragma once

#include "../vm/multi_app_runtime.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace openylm::debug {

class Debugger {
public:
    explicit Debugger(vm::MultiAppRuntime& runtime);

    bool step(const std::string& appId, std::string& error);
    bool addBreakpoint(const std::string& appId, std::size_t instructionPc, std::string& error);
    std::optional<vm::AppStats> inspect(const std::string& appId) const;

private:
    vm::MultiAppRuntime& runtime_;
};

} // namespace openylm::debug
