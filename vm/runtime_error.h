#pragma once

#include <cstddef>
#include <string>

namespace openylm::vm {

enum class RuntimeErrorType {
    RuntimeError,
    SandboxViolationError,
    StackOverflowError,
    MemoryLimitExceededError,
    InvalidBytecodeError,
    UndefinedHostCallError,
};

enum class RuntimeSeverity {
    Warning,
    Error,
    Fatal,
};

struct RuntimeErrorDetail {
    RuntimeErrorType type = RuntimeErrorType::RuntimeError;
    std::string message;
    std::string appId;
    std::size_t pc = 0;
    std::string origin;
    RuntimeSeverity severity = RuntimeSeverity::Error;

    std::string format() const;
};

std::string toString(RuntimeErrorType type);
std::string toString(RuntimeSeverity severity);

} // namespace openylm::vm
