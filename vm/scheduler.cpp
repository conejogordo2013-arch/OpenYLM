#include "scheduler.h"

namespace openylm::vm {

std::vector<std::size_t> Scheduler::nextRoundRobin(const std::vector<AppContext>& apps) const {
    std::vector<std::size_t> order;
    for (std::size_t i = 0; i < apps.size(); ++i) {
        if (!apps[i].finished) {
            order.push_back(i);
        }
    }
    return order;
}

} // namespace openylm::vm
