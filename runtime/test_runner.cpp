#include "test_runner.h"

#include "../bytecode/bytecode.h"
#include "../compiler/error.h"
#include "../compiler/ir_to_bytecode.h"
#include "../compiler/module_resolver.h"
#include "../host/host_registry.h"
#include "../vm/multi_app_runtime.h"
#include "../vm/vm.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <vector>

namespace openylm::runtime {
namespace {

bool runSingle(const std::filesystem::path& path, bool debug, std::ostringstream& out, bool expectCompileError, bool expectRuntimeError) {
    std::vector<compiler::CompilerError> compileErrors;
    compiler::ResolvedProject resolved;
    const bool parsed = compiler::resolveProject(path.string(), debug, resolved, compileErrors);

    if (!parsed) {
        if (expectCompileError) {
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
    std::string vmError;
    bool vmOk = true;

    if (expectRuntimeError) {
        vm::MultiAppRuntime runtime(registry);
        runtime.loadApp(path.filename().string(), bytecodeProgram);
        vmOk = runtime.runAll(vmError);
    } else {
        vm::VM vm(registry);
        vmOk = vm.execute(bytecodeProgram, vmError);
    }

    if (!vmOk && expectRuntimeError) {
        out << "[TEST] " << path.filename().string() << " -> PASS (expected runtime error)\n";
        return true;
    }
    if (!vmOk) {
        out << "[TEST] " << path.filename().string() << " -> FAIL (vm: " << vmError << ")\n";
        return false;
    }

    if (expectRuntimeError) {
        out << "[TEST] " << path.filename().string() << " -> FAIL (expected runtime error)\n";
        return false;
    }

    out << "[TEST] " << path.filename().string() << " -> PASS\n";
    return true;
}

bool shouldExpectCompileError(const std::string& name) {
    return name == "invalid_syntax.yl" || name == "invalid_bytecode.yl";
}

bool shouldExpectRuntimeError(const std::string& name) {
    return name == "sandbox_violation.yl" || name == "memory_limit_test.yl";
}

} // namespace

bool runSingleTest(const std::string& testFile, bool debug, std::string& report) {
    std::ostringstream out;
    const std::filesystem::path path(testFile);
    const bool ok = runSingle(path, debug, out, shouldExpectCompileError(path.filename().string()), shouldExpectRuntimeError(path.filename().string()));
    report = out.str();
    return ok;
}

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
        const auto name = path.filename().string();
        if (!runSingle(path, debug, out, shouldExpectCompileError(name), shouldExpectRuntimeError(name))) {
            ++failedCount;
        }
    }

    report = out.str();
    return failedCount == 0;
}

} // namespace openylm::runtime
