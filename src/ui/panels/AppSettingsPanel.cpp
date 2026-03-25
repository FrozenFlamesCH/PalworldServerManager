// AppSettingsPanel.cpp — delegates to UIManager's inline implementation
#include "ui/panels/AppSettingsPanel.h"
#include "Application.h"
#include <imgui.h>

namespace PSM {
void AppSettingsPanel::render() {
    // Already fully implemented in UIManager::renderAppSettings()
    // This class exists for future refactoring
    m_app.uiManager().renderAppSettings();
}
} // namespace PSM
