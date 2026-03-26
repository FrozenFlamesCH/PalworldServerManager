// AppSettingsPanel.cpp — delegates to UIManager's inline implementation
#include "ui/panels/AppSettingsPanel.h"
#include "Application.h"
#include "ui/UIManager.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace PSM {
void AppSettingsPanel::render() {
    // Already fully implemented in UIManager::renderAppSettings()
    // This class exists for future refactoring
    m_app.uiManager().renderAppSettings();
}
} // namespace PSM
