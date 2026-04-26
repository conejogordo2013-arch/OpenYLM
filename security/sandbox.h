#pragma once

#include "../vm/app_context.h"

#include <string>

namespace openylm::security {

bool validateRuntimeLimits(vm::AppContext& app, std::string& error);
std::size_t allowedInstructionsThisTick(const vm::AppContext& app, std::size_t schedulerSlice);

} // namespace openylm::security
