#pragma once

#include "app_context.h"

#include <cstddef>
#include <vector>

namespace openylm::vm {

class Scheduler {
public:
    explicit Scheduler(std::size_t timeSliceInstructions = 8);

    std::vector<std::size_t> nextRoundRobin(const std::vector<AppContext>& apps);
    [[nodiscard]] std::size_t timeSliceInstructions() const;

private:
    std::size_t cursor_ = 0;
    std::size_t timeSliceInstructions_;
};

} // namespace openylm::vm
