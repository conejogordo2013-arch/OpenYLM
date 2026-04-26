#pragma once

#include "../vm/app_context.h"

#include <string>

namespace openylm::ylvm {

inline vm::AppContext createIsolatedContext(const std::string& appId, const bytecode::Program& program) {
    vm::AppContext context;
    context.appId = appId;
    context.program = program;
    return context;
}

} // namespace openylm::ylvm
