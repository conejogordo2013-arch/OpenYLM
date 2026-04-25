#pragma once

#include "../bytecode/bytecode.h"

#include <cstddef>
#include <string>
#include <vector>

namespace openylm::vm {

struct AppContext {
    std::string appId;
    bytecode::Program program;
    std::size_t pc = 0;
    std::vector<int> stack;
    bool finished = false;
};

} // namespace openylm::vm
