#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace openylm::ir {

enum class NodeType {
    CallHost,
    ConstString,
    ConstNumber,
    Add,
    Sub,
    Mul,
    Div,
    Variable,
    If,
    Loop,
    SetVar,
};

struct Node {
    NodeType type = NodeType::CallHost;
    std::string host;
    std::string value;
    std::vector<std::string> args;
    std::size_t sourceLine = 0;
};

using Program = std::vector<Node>;

std::string nodeTypeToString(NodeType type);
bool nodeTypeFromString(const std::string& text, NodeType& out);

bool serializeToFile(const Program& program, const std::string& path, std::string& error);
bool deserializeFromFile(const std::string& path, Program& out, std::string& error);

} // namespace openylm::ir
