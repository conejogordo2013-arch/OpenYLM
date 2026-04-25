#pragma once

#include "../bytecode/bytecode.h"
#include "../ir/ir.h"
#include "error.h"

#include <vector>

namespace openylm::compiler {

bool irToBytecode(const ir::Program& irProgram, bytecode::Program& out, bool debug, std::vector<CompilerError>& errors);

} // namespace openylm::compiler
