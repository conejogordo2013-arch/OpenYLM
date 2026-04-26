#pragma once

#include "../bytecode/bytecode.h"

namespace openylm::jit {

inline bool isOptimizableOpcode(bytecode::OpCode op) {
    return op == bytecode::OpCode::Add || op == bytecode::OpCode::Sub || op == bytecode::OpCode::Mul || op == bytecode::OpCode::Div;
}

} // namespace openylm::jit
