#include "debugger_ui.h"

#include "runtime_view.h"

namespace openylm::inspector {

std::string DebuggerUi::render() const {
    return renderRuntimeView(runtime_);
}

} // namespace openylm::inspector
