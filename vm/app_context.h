#pragma once

#include "../bytecode/bytecode.h"
#include "runtime_error.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace openylm::vm {

enum class AppExecutionState {
    Runnable,
    Paused,
    Completed,
    Stopped,
    Faulted,
};

struct AppPermissions {
    bool allowHostCalls = true;
};

struct AppSandbox {
    bool allowFileIo = false;
    bool allowNetwork = false;
    std::unordered_set<std::string> allowedHostCalls;
    std::size_t memoryLimitBytes = 64 * 1024;
    std::size_t cpuLimitPerTick = 8;
    bool active = true;
};

struct AppContext {
    std::string appId;
    bytecode::Program program;
    std::size_t pc = 0;
    std::vector<int> stack;
    std::map<std::uint32_t, int> heap;
    AppPermissions permissions;
    AppSandbox sandbox;
    AppExecutionState state = AppExecutionState::Runnable;
    std::unordered_set<std::size_t> breakpoints;
    std::uint64_t totalInstructions = 0;
    std::uint64_t totalSlices = 0;
    std::uint8_t lastOpcode = 0;
    std::unordered_map<std::size_t, std::uint64_t> hotInstructionHits;
    std::string lastError;

    [[nodiscard]] bool isTerminated() const {
        return state == AppExecutionState::Completed || state == AppExecutionState::Stopped || state == AppExecutionState::Faulted;
    }

    [[nodiscard]] std::size_t estimatedMemoryUsageBytes() const {
        return (stack.size() * sizeof(int)) + (heap.size() * (sizeof(std::uint32_t) + sizeof(int)));
    }
};

} // namespace openylm::vm
