#include "jit_engine.h"

namespace openylm::jit {

void JitEngine::observe(vm::AppContext& app, std::size_t instructionPc) {
    if (!enabled_) {
        return;
    }
    ++app.hotInstructionHits[instructionPc];
}

bool JitEngine::isHot(const vm::AppContext& app, std::size_t instructionPc) const {
    if (!enabled_) {
        return false;
    }
    const auto it = app.hotInstructionHits.find(instructionPc);
    return it != app.hotInstructionHits.end() && it->second >= hotThreshold_;
}

} // namespace openylm::jit
