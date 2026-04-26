#pragma once

#include "../vm/app_context.h"

#include <sstream>
#include <string>

namespace openylm::debug {

inline std::string renderAppState(const vm::AppContext& app) {
    std::ostringstream out;
    out << "[APP " << app.appId << "] PC=" << app.pc << " STACK_SIZE=" << app.stack.size() << " HEAP_SIZE=" << app.heap.size();
    return out.str();
}

} // namespace openylm::debug
