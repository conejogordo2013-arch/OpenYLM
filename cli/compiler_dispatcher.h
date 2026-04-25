#pragma once

#include "command_parser.h"

#include <string>

namespace openylm::cli {

bool dispatchCompile(const CommandLine& cmd, std::string& error);

} // namespace openylm::cli
