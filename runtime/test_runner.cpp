#include "test_runner.h"

#include "../bytecode/bytecode.h"
#include "../compiler/error.h"
#include "../compiler/ir_to_bytecode.h"
#include "../compiler/module_resolver.h"
#include "../host/host_registry.h"
#include "../vm/vm.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <vector>

namespace openylm::runtime {
namespace {

bool runSingle(const std::filesystem::path& path, bool debug, std::ostringstream& out, bool expectError) {
    std::vector<compiler::CompilerError> compileErrors;
    compiler::ResolvedProject resolved;
    const bool parsed = compiler::resolveProject(path.string(), debug, resolved, compileErrors);

    if (!parsed) {
        if (expectError) {
            out << "[TEST] " << path.filename().string() << " -> PASS (expected compile error)\n";
            return true;
        }
        out << "[TEST] " << path.filename().string() << " -> FAIL\n";
        for (const auto& e : compileErrors) {
            out << "        " << compiler::formatError(e) << "\n";
        }
        return false;
    }

    ir::Program irProgram;
    for (const auto& file : resolved.loadOrder) {
        const auto it = resolved.units.find(file);
        if (it != resolved.units.end()) {
            irProgram.insert(irProgram.end(), it->second.program.begin(), it->second.program.end());
        }
    }

    bytecode::Program bytecodeProgram;
    if (!compiler::irToBytecode(irProgram, bytecodeProgram, debug, compileErrors)) {
        out << "[TEST] " << path.filename().string() << " -> FAIL\n";
        for (const auto& e : compileErrors) {
            out << "        " << compiler::formatError(e) << "\n";
        }
        return false;
    }

    host::HostRegistry registry;
    host::registerDefaultRuntimeHosts(registry);
    vm::VM vm(registry);
    std::string vmError;
    if (!vm.execute(bytecodeProgram, vmError)) {
        out << "[TEST] " << path.filename().string() << " -> FAIL (vm: " << vmError << ")\n";
        return false;
    }

    out << "[TEST] " << path.filename().string() << " -> PASS\n";
    return true;
}

} // namespace

bool runAllTests(const std::string& testsDir, bool debug, std::string& report, int& failedCount) {
    std::ostringstream out;
    failedCount = 0;

    namespace fs = std::filesystem;
    if (!fs::exists(testsDir)) {
        report = "tests directory not found: " + testsDir;
        failedCount = 1;
        return false;
    }

    std::vector<fs::path> testFiles;
    for (const auto& entry : fs::directory_iterator(testsDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".yl") {
            testFiles.push_back(entry.path());
        }
    }
    std::sort(testFiles.begin(), testFiles.end());

    for (const auto& path : testFiles) {
        const bool expectError = path.filename() == "invalid_syntax.yl";
        if (!runSingle(path, debug, out, expectError)) {
            ++failedCount;
        }
    }

    report = out.str();
    return failedCount == 0;
}

} // namespace openylm::runtime
