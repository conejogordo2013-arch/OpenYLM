#pragma once

#include "yaml_to_ir.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace openylm::compiler {

struct ResolvedProject {
    std::vector<std::string> loadOrder;
    std::unordered_map<std::string, SourceUnit> units;
    std::unordered_map<std::string, std::vector<std::string>> exportsByModule;
};

bool resolveProject(const std::string& entryFile, bool debug, ResolvedProject& out, std::vector<CompilerError>& errors);

} // namespace openylm::compiler
