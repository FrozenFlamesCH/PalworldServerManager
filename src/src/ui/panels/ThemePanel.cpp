// ThemePanel.cpp
#include "ui/panels/ThemePanel.h"
#include "Application.h"
#include "ui/UIManager.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace PSM {
void ThemePanel::render() {
    m_app.uiManager().renderThemes();  // delegates to UIManager
}
} // namespace PSM