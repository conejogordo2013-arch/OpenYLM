#pragma once

#include "app_manager.h"

#include <optional>
#include <string>
#include <vector>

namespace openylm::runtime {

class ControlApi {
public:
    explicit ControlApi(AppManager& manager) : manager_(manager) {}

    bool pause_app(const std::string& appId) { return manager_.pauseApp(appId); }
    bool resume_app(const std::string& appId) { return manager_.resumeApp(appId); }
    bool stop_app(const std::string& appId) { return manager_.stopApp(appId); }
    std::vector<std::string> list_apps() const { return manager_.listApps(); }
    std::optional<vm::AppStats> inspect_app(const std::string& appId) const { return manager_.inspectApp(appId); }

private:
    AppManager& manager_;
};

} // namespace openylm::runtime
