#pragma once

#include "../ir/ir.h"
#include "error.h"

#include <string>
#include <vector>

namespace openylm::compiler {

struct SourceUnit {
    std::string filePath;
    std::string moduleName;
    std::vector<std::string> imports;
    std::vector<std::string> exports;
    ir::Program program;
};

bool parseSourceUnit(const std::string& yamlPath, SourceUnit& out, bool debug, std::vector<CompilerError>& errors);
bool yamlToIr(const std::string& yamlPath, ir::Program& out, bool debug, std::vector<CompilerError>& errors);

} // namespace openylm::compiler
