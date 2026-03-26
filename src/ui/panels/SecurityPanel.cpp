// SecurityPanel.cpp
#include "ui/panels/SecurityPanel.h"
#include "Application.h"
#include "net/PalDefenderRCON.h"
#include "core/Database.h"
#include "ui/Widgets.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstring>

namespace PSM {
void SecurityPanel::render() {
    ImGui::Text("🔐 Security & Anti-Cheat");
    ImGui::Separator();
    ImGui::Spacing();
    Widgets::SeparatorText("Toggles");
    Widgets::ToggleSwitch("Auto-kick cheaters", &m_autoKick);
    Widgets::ToggleSwitch("Whitelist enforcement", &m_whitelist);
    Widgets::ToggleSwitch("Illegal item protection", &m_illegalItem);
    Widgets::ToggleSwitch("Illegal pal detection", &m_illegalPal);
    Widgets::ToggleSwitch("Tower boss block", &m_towerBlock);
    Widgets::ToggleSwitch("SteamID dupe protection", &m_steamDupe);
    Widgets::SeparatorText("IP Ban");
    ImGui::InputText("IP", m_ipBuf, sizeof(m_ipBuf));
    if (ImGui::Button("Ban IP") && strlen(m_ipBuf) > 0) { m_app.palDefenderRCON().banIP(m_ipBuf); m_ipBuf[0]='\0'; }
    ImGui::SameLine();
    if (ImGui::Button("Unban IP") && strlen(m_ipBuf) > 0) { m_app.palDefenderRCON().unbanIP(m_ipBuf); m_ipBuf[0]='\0'; }
    Widgets::SeparatorText("Recent Cheat Alerts");
    auto cheatLogs = m_app.database().getLogs("cheat", 30);
    for (auto& l : cheatLogs) Widgets::TextError(l.value("message","").c_str());
}
} // namespace PSM
