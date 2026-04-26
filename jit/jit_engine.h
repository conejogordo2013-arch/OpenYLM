#pragma once

#include "../vm/app_context.h"

#include <cstddef>

namespace openylm::jit {

class JitEngine {
public:
    void setEnabled(bool enabled) { enabled_ = enabled; }
    [[nodiscard]] bool isEnabled() const { return enabled_; }

    void observe(vm::AppContext& app, std::size_t instructionPc);
    [[nodiscard]] bool isHot(const vm::AppContext& app, std::size_t instructionPc) const;

private:
    bool enabled_ = false;
    std::size_t hotThreshold_ = 6;
};

} // namespace openylm::jit
