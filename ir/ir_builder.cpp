#include "ir.h"

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace openylm::ir {

std::string nodeTypeToString(NodeType type) {
    switch (type) {
    case NodeType::CallHost:
        return "CALL_HOST";
    case NodeType::ConstString:
        return "CONST_STRING";
    case NodeType::ConstNumber:
        return "CONST_NUMBER";
    case NodeType::Add:
        return "ADD";
    case NodeType::Sub:
        return "SUB";
    case NodeType::Mul:
        return "MUL";
    case NodeType::Div:
        return "DIV";
    case NodeType::Variable:
        return "VARIABLE";
    case NodeType::If:
        return "IF";
    case NodeType::Loop:
        return "LOOP";
    case NodeType::SetVar:
        return "SET_VAR";
    }
    return "UNKNOWN";
}

bool nodeTypeFromString(const std::string& text, NodeType& out) {
    if (text == "CALL_HOST") {
        out = NodeType::CallHost;
    } else if (text == "CONST_STRING") {
        out = NodeType::ConstString;
    } else if (text == "CONST_NUMBER") {
        out = NodeType::ConstNumber;
    } else if (text == "ADD") {
        out = NodeType::Add;
    } else if (text == "SUB") {
        out = NodeType::Sub;
    } else if (text == "MUL") {
        out = NodeType::Mul;
    } else if (text == "DIV") {
        out = NodeType::Div;
    } else if (text == "VARIABLE") {
        out = NodeType::Variable;
    } else if (text == "IF") {
        out = NodeType::If;
    } else if (text == "LOOP") {
        out = NodeType::Loop;
    } else if (text == "SET_VAR") {
        out = NodeType::SetVar;
    } else {
        return false;
    }
    return true;
}

bool serializeToFile(const Program& program, const std::string& path, std::string& error) {
    std::ofstream out(path);
    if (!out) {
        error = "unable to open IR output file";
        return false;
    }

    out << "IR1\n";
    for (const auto& node : program) {
        out << nodeTypeToString(node.type) << '\n';
        out << std::quoted(node.host) << '\n';
        out << std::quoted(node.value) << '\n';
        out << node.sourceLine << '\n';
        out << node.args.size() << '\n';
        for (const auto& arg : node.args) {
            out << std::quoted(arg) << '\n';
        }
    }

    if (!out) {
        error = "failed to write IR file";
        return false;
    }

    return true;
}

bool deserializeFromFile(const std::string& path, Program& out, std::string& error) {
    std::ifstream in(path);
    if (!in) {
        error = "unable to open IR input file";
        return false;
    }

    std::string header;
    std::getline(in, header);
    if (header != "IR1") {
        error = "invalid IR file header";
        return false;
    }

    out.clear();
    while (in.peek() != EOF) {
        std::string typeLine;
        std::getline(in, typeLine);
        if (typeLine.empty()) {
            continue;
        }

        Node node;
        if (!nodeTypeFromString(typeLine, node.type)) {
            error = "invalid IR node type: " + typeLine;
            return false;
        }

        if (!(in >> std::quoted(node.host))) {
            error = "failed to read IR node host";
            return false;
        }
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (!(in >> std::quoted(node.value))) {
            error = "failed to read IR node value";
            return false;
        }
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (!(in >> node.sourceLine)) {
            error = "failed to read IR source line";
            return false;
        }
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::size_t argCount = 0;
        if (!(in >> argCount)) {
            error = "failed to read IR arg count";
            return false;
        }
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        node.args.reserve(argCount);
        for (std::size_t i = 0; i < argCount; ++i) {
            std::string arg;
            if (!(in >> std::quoted(arg))) {
                error = "failed to read IR arg";
                return false;
            }
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            node.args.push_back(arg);
        }

        out.push_back(std::move(node));
    }

    return true;
}

} // namespace openylm::ir
