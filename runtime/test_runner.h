#pragma once

#include <string>

namespace openylm::runtime {

bool runAllTests(const std::string& testsDir, bool debug, std::string& report, int& failedCount);
bool runSingleTest(const std::string& testFile, bool debug, std::string& report);

} // namespace openylm::runtime
