#pragma once

#include "../vm/multi_app_runtime.h"

#include <string>

namespace openylm::inspector {

class DebuggerUi {
public:
    explicit DebuggerUi(const vm::MultiAppRuntime& runtime) : runtime_(runtime) {}
    std::string render() const;

private:
    const vm::MultiAppRuntime& runtime_;
};

} // namespace openylm::inspector
