#include "runtime_error.h"

namespace openylm::vm {

std::string toString(RuntimeErrorType type) {
    switch (type) {
    case RuntimeErrorType::RuntimeError: return "RuntimeError";
    case RuntimeErrorType::SandboxViolationError: return "SandboxViolationError";
    case RuntimeErrorType::StackOverflowError: return "StackOverflowError";
    case RuntimeErrorType::MemoryLimitExceededError: return "MemoryLimitExceededError";
    case RuntimeErrorType::InvalidBytecodeError: return "InvalidBytecodeError";
    case RuntimeErrorType::UndefinedHostCallError: return "UndefinedHostCallError";
    }
    return "RuntimeError";
}

std::string toString(RuntimeSeverity severity) {
    switch (severity) {
    case RuntimeSeverity::Warning: return "WARNING";
    case RuntimeSeverity::Error: return "ERROR";
    case RuntimeSeverity::Fatal: return "FATAL";
    }
    return "ERROR";
}

std::string RuntimeErrorDetail::format() const {
    return "[RuntimeError]\nType: " + toString(type) +
           "\nApp: " + appId +
           "\nPC: " + std::to_string(pc) +
           "\nOrigin: " + origin +
           "\nMessage: " + message +
           "\nSeverity: " + toString(severity);
}

} // namespace openylm::vm
