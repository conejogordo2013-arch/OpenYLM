#pragma once

#include "app_context.h"

#include <vector>

namespace openylm::vm {

class Scheduler {
public:
    std::vector<std::size_t> nextRoundRobin(const std::vector<AppContext>& apps) const;
};

} // namespace openylm::vm
