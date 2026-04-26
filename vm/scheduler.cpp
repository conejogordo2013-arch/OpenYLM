#include "scheduler.h"

namespace openylm::vm {

Scheduler::Scheduler(std::size_t timeSliceInstructions) : timeSliceInstructions_(timeSliceInstructions == 0 ? 1 : timeSliceInstructions) {}

std::vector<std::size_t> Scheduler::nextRoundRobin(const std::vector<AppContext>& apps) {
    std::vector<std::size_t> order;
    if (apps.empty()) {
        return order;
    }

    const std::size_t start = cursor_ % apps.size();
    for (std::size_t offset = 0; offset < apps.size(); ++offset) {
        const std::size_t i = (start + offset) % apps.size();
        const auto state = apps[i].state;
        if (state == AppExecutionState::Runnable) {
            order.push_back(i);
        }
    }

    cursor_ = (start + 1) % apps.size();
    return order;
}

std::size_t Scheduler::timeSliceInstructions() const {
    return timeSliceInstructions_;
}

} // namespace openylm::vm
