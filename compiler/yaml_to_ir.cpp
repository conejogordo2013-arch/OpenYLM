#include "yaml_to_ir.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

namespace openylm::compiler {
namespace {

std::string trim(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

std::string unquote(std::string s) {
    s = trim(s);
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

void addError(std::vector<CompilerError>& errors, const std::string& file, std::size_t line, ErrorType type, const std::string& message) {
    errors.push_back(CompilerError{file, line, Severity::Error, type, message});
}

} // namespace

bool parseSourceUnit(const std::string& yamlPath, SourceUnit& out, bool debug, std::vector<CompilerError>& errors) {
    std::ifstream in(yamlPath);
    if (!in) {
        addError(errors, yamlPath, 0, ErrorType::FileNotFound, "input .yl file not found");
        return false;
    }

    out = SourceUnit{};
    out.filePath = yamlPath;

    std::string line;
    std::size_t lineNo = 0;
    bool sawProgram = false;
    bool inCall = false;
    bool inExport = false;
    bool inProgramBlock = false;
    ir::Node current;

    while (std::getline(in, line)) {
        ++lineNo;
        const std::string t = trim(line);
        if (t.empty()) {
            continue;
        }

        if (debug) {
            std::cout << "[debug] parse line " << lineNo << " (" << yamlPath << "): " << t << '\n';
        }

        if (t.rfind("import:", 0) == 0) {
            inProgramBlock = false;
            out.imports.push_back(unquote(t.substr(7)));
            continue;
        }
        if (t.rfind("module:", 0) == 0) {
            inProgramBlock = false;
            out.moduleName = unquote(t.substr(7));
            continue;
        }
        if (t == "export:") {
            inExport = true;
            continue;
        }
        if (inExport && t.rfind("-", 0) == 0) {
            out.exports.push_back(trim(t.substr(1)));
            continue;
        }
        inExport = false;

        if (t == "program:") {
            sawProgram = true;
            inProgramBlock = true;
            continue;
        }

        if (t == "- call:" || t == "call:") {
            if (inCall) {
                if (current.host.empty()) {
                    addError(errors, yamlPath, current.sourceLine, ErrorType::Syntax, "call node missing host");
                }
                out.program.push_back(current);
            }
            current = ir::Node{};
            current.type = ir::NodeType::CallHost;
            current.sourceLine = lineNo;
            inCall = true;
            continue;
        }
        if (t == "- if:" || t == "if:") {
            if (inCall) {
                if (current.type == ir::NodeType::CallHost && current.host.empty()) {
                    addError(errors, yamlPath, current.sourceLine, ErrorType::Syntax, "call node missing host");
                }
                out.program.push_back(current);
            }
            current = ir::Node{};
            current.type = ir::NodeType::If;
            current.sourceLine = lineNo;
            inCall = true;
            continue;
        }

        if (t == "- loop:" || t == "loop:") {
            if (inCall) {
                if (current.type == ir::NodeType::CallHost && current.host.empty()) {
                    addError(errors, yamlPath, current.sourceLine, ErrorType::Syntax, "call node missing host");
                }
                out.program.push_back(current);
            }
            current = ir::Node{};
            current.type = ir::NodeType::Loop;
            current.sourceLine = lineNo;
            inCall = true;
            continue;
        }

        if (!inCall) {
            if (inProgramBlock && t.rfind("-", 0) == 0) {
                addError(errors, yamlPath, lineNo, ErrorType::Syntax, "unsupported program entry; expected call/if/loop");
            }
            continue;
        }

        if (t.rfind("host:", 0) == 0) {
            current.host = unquote(t.substr(5));
            if (current.host.empty()) {
                addError(errors, yamlPath, lineNo, ErrorType::Syntax, "host cannot be empty");
            }
            continue;
        }

        if (t.rfind("args:", 0) == 0) {
            continue;
        }

        const auto colon = t.find(':');
        if (colon != std::string::npos) {
            const auto value = unquote(t.substr(colon + 1));
            if (trim(value).empty()) {
                addError(errors, yamlPath, lineNo, ErrorType::Syntax, "argument value cannot be empty");
            }
            current.args.push_back(value);
        } else {
            addError(errors, yamlPath, lineNo, ErrorType::Syntax, "invalid key/value syntax");
        }
    }

    if (inCall) {
        if (current.type == ir::NodeType::CallHost && current.host.empty()) {
            addError(errors, yamlPath, current.sourceLine, ErrorType::Syntax, "call node missing host");
        }
        out.program.push_back(current);
    }

    if (!sawProgram) {
        addError(errors, yamlPath, 1, ErrorType::Syntax, "missing program root");
    }

    return errors.empty();
}

bool yamlToIr(const std::string& yamlPath, ir::Program& out, bool debug, std::vector<CompilerError>& errors) {
    SourceUnit unit;
    const bool ok = parseSourceUnit(yamlPath, unit, debug, errors);
    out = std::move(unit.program);
    return ok;
}

} // namespace openylm::compiler
