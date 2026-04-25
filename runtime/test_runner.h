#pragma once

#include <string>

namespace openylm::runtime {

bool runAllTests(const std::string& testsDir, bool debug, std::string& report, int& failedCount);

} // namespace openylm::runtime
