#pragma once

#include "../vm/app_context.h"

namespace openylm::ylvm {

inline bool isIsolatedFrom(const vm::AppContext& a, const vm::AppContext& b) {
    return &a != &b;
}

} // namespace openylm::ylvm
