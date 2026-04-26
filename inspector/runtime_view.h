#pragma once

#include "../vm/multi_app_runtime.h"

#include <string>

namespace openylm::inspector {

std::string renderRuntimeView(const vm::MultiAppRuntime& runtime);

} // namespace openylm::inspector
