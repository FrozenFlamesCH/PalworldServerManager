// WorldControlPanel.cpp
#include "ui/panels/WorldControlPanel.h"
#include "Application.h"
#include "server/ServerManager.h"
#include "net/PalDefenderRCON.h"
#include "ui/Widgets.h"
#include <imgui.h>

namespace PSM {
void WorldControlPanel::render() {
    ImGui::Text("⏰ World & Server Control");
    ImGui::Separator();
    ImGui::Spacing();
    Widgets::SeparatorText("Time Control");
    ImGui::SliderInt("Hour", &m_hour, 0, 23);
    if (ImGui::Button("Set Time")) m_app.palDefenderRCON().setTime(m_hour);
    ImGui::SameLine();
    if (ImGui::Button("Day")) m_app.palDefenderRCON().setTimeDay();
    ImGui::SameLine();
    if (ImGui::Button("Night")) m_app.palDefenderRCON().setTimeNight();
    Widgets::SeparatorText("Server");
    if (ImGui::Button("💾 Save World", ImVec2(200, 35))) m_app.serverManager().saveWorld();
    ImGui::SameLine();
    if (ImGui::Button("⏹ Graceful Shutdown", ImVec2(200, 35))) m_shutdownConfirm = true;
    ImGui::SameLine();
    if (ImGui::Button("🔄 Reload PD Config", ImVec2(200, 35))) m_app.palDefenderRCON().reloadConfig();
    if (Widgets::ConfirmDialog("Confirm Shutdown",
        "Broadcast countdown then shut down?", &m_shutdownConfirm))
        m_app.serverManager().gracefulShutdownWithCountdown();
}
} // namespace PSM
