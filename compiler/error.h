#pragma once

#include <string>

namespace openylm::compiler {

enum class Severity { Warning, Error };
enum class ErrorType {
    FileNotFound,
    Syntax,
    Unsupported,
    Internal,
    ModuleNotFound,
    CircularImport,
    UndefinedSymbol,
};

struct CompilerError {
    std::string file;
    std::size_t line = 0;
    Severity severity = Severity::Error;
    ErrorType type = ErrorType::Internal;
    std::string message;
};

inline std::string formatError(const CompilerError& e) {
    return e.file + ":" + std::to_string(e.line) + ": " + e.message;
}

} // namespace openylm::compiler
