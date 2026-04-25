#include "ir_to_bytecode.h"

#include <iostream>

namespace openylm::compiler {

bool irToBytecode(const ir::Program& irProgram, bytecode::Program& out, bool debug, std::vector<CompilerError>& errors) {
    out = bytecode::Program{};

    for (const auto& node : irProgram) {
        switch (node.type) {
        case ir::NodeType::CallHost: {
            if (node.host.empty()) {
                errors.push_back({"<ir>", node.sourceLine, Severity::Error, ErrorType::Syntax, "CALL_HOST missing host name"});
                continue;
            }

            bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::CallHost));
            bytecode::emitU32(out, bytecode::internString(out, node.host));
            bytecode::emitU32(out, static_cast<std::uint32_t>(node.args.size()));
            for (const auto& arg : node.args) {
                bytecode::emitU32(out, bytecode::internString(out, arg));
            }

            if (debug) {
                std::cout << "[debug] emit CALL_HOST " << node.host << " argc=" << node.args.size() << '\n';
            }
            break;
        }
        case ir::NodeType::ConstString:
            bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::ConstString));
            bytecode::emitU32(out, bytecode::internString(out, node.value));
            break;
        case ir::NodeType::ConstNumber:
            try {
                bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::ConstNumber));
                bytecode::emitU32(out, static_cast<std::uint32_t>(std::stoi(node.value)));
            } catch (...) {
                errors.push_back({"<ir>", node.sourceLine, Severity::Error, ErrorType::Syntax, "invalid CONST_NUMBER value"});
            }
            break;
        case ir::NodeType::Add:
            bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::Add));
            break;
        case ir::NodeType::Sub:
            bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::Sub));
            break;
        case ir::NodeType::Mul:
            bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::Mul));
            break;
        case ir::NodeType::Div:
            bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::Div));
            break;
        case ir::NodeType::Variable:
            bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::LoadVar));
            bytecode::emitU32(out, bytecode::internString(out, node.value));
            break;
        case ir::NodeType::If:
        case ir::NodeType::Loop:
        case ir::NodeType::SetVar:
            errors.push_back({"<ir>", node.sourceLine, Severity::Error, ErrorType::Unsupported, "control flow node not yet lowered to bytecode"});
            break;
        }
    }

    bytecode::emitU8(out, static_cast<std::uint8_t>(bytecode::OpCode::End));
    return errors.empty();
}

} // namespace openylm::compiler
