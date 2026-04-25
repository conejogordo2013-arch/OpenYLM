#include "module_resolver.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <unordered_set>

namespace openylm::compiler {
namespace {

std::string moduleNameFromPath(const std::filesystem::path& path) {
    return path.stem().string();
}

bool dfs(const std::filesystem::path& file,
         bool debug,
         ResolvedProject& out,
         std::vector<CompilerError>& errors,
         std::unordered_set<std::string>& visiting,
         std::unordered_set<std::string>& visited) {
    const std::string norm = std::filesystem::weakly_canonical(file).string();
    if (visited.count(norm)) {
        return true;
    }
    if (visiting.count(norm)) {
        errors.push_back({norm, 0, Severity::Error, ErrorType::CircularImport, "circular import detected"});
        return false;
    }
    visiting.insert(norm);

    SourceUnit unit;
    if (!parseSourceUnit(norm, unit, debug, errors)) {
        return false;
    }

    if (unit.moduleName.empty()) {
        unit.moduleName = moduleNameFromPath(file);
    }
    out.exportsByModule[unit.moduleName] = unit.exports;

    const auto parent = file.parent_path();
    for (const auto& imp : unit.imports) {
        auto candidate = parent / imp;
        if (!std::filesystem::exists(candidate)) {
            candidate = std::filesystem::path("stdlib") / imp;
        }
        if (!std::filesystem::exists(candidate)) {
            errors.push_back({norm, 0, Severity::Error, ErrorType::ModuleNotFound, "import not found: " + imp});
            return false;
        }

        if (debug) {
            std::cout << "[debug] resolve import " << imp << " -> " << candidate.string() << '\n';
        }

        if (!dfs(candidate, debug, out, errors, visiting, visited)) {
            return false;
        }
    }

    // Symbol checks for namespaced calls like module.symbol
    for (const auto& node : unit.program) {
        const auto dot = node.host.find('.');
        if (dot == std::string::npos) {
            continue;
        }
        const auto mod = node.host.substr(0, dot);
        const auto symbol = node.host.substr(dot + 1);
        const auto it = out.exportsByModule.find(mod);
        if (it == out.exportsByModule.end()) {
            errors.push_back({norm, node.sourceLine, Severity::Error, ErrorType::UndefinedSymbol, "unknown module in host call: " + mod});
            return false;
        }
        const auto& exports = it->second;
        if (std::find(exports.begin(), exports.end(), symbol) == exports.end()) {
            errors.push_back({norm, node.sourceLine, Severity::Error, ErrorType::UndefinedSymbol, "undefined symbol: " + node.host});
            return false;
        }
    }

    out.units[norm] = std::move(unit);
    out.loadOrder.push_back(norm);
    visiting.erase(norm);
    visited.insert(norm);
    return true;
}

} // namespace

bool resolveProject(const std::string& entryFile, bool debug, ResolvedProject& out, std::vector<CompilerError>& errors) {
    out = ResolvedProject{};
    std::unordered_set<std::string> visiting;
    std::unordered_set<std::string> visited;
    return dfs(std::filesystem::path(entryFile), debug, out, errors, visiting, visited);
}

} // namespace openylm::compiler
