#pragma once

#include "../vm/app_context.h"

#include <string>

namespace openylm::security {

bool isHostCallAllowed(const vm::AppContext& app, const std::string& functionName);

} // namespace openylm::security
