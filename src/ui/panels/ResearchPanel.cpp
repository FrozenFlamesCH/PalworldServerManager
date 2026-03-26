// ResearchPanel.cpp
#include "ui/panels/ResearchPanel.h"
#include "Application.h"
#include "net/PalDefenderRCON.h"
#include "net/PalDefenderAPI.h"
#include "ui/Widgets.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstring>

namespace PSM {
void ResearchPanel::render() {
    ImGui::Text("🔬 Research / Tech Tree");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::InputText("Player UID", m_uid, sizeof(m_uid));
    ImGui::Spacing();
    Widgets::SeparatorText("Give Points");
    ImGui::InputInt("Tech Points", &m_techPts);
    ImGui::SameLine();
    if (ImGui::Button("Give##tp") && strlen(m_uid) > 0 && m_techPts > 0)
        m_app.palDefenderRCON().giveTechPoints(m_uid, m_techPts);
    ImGui::InputInt("Ancient Tech", &m_ancientPts);
    ImGui::SameLine();
    if (ImGui::Button("Give##atp") && strlen(m_uid) > 0 && m_ancientPts > 0)
        m_app.palDefenderRCON().giveAncientTechPoints(m_uid, m_ancientPts);
    ImGui::Spacing();
    Widgets::SeparatorText("Technology Management");
    ImGui::InputText("Tech ID", m_techId, sizeof(m_techId));
    if (ImGui::Button("Unlock")) m_app.palDefenderRCON().unlockTech(m_uid, m_techId);
    ImGui::SameLine();
    if (ImGui::Button("Unlock ALL")) m_app.palDefenderRCON().unlockAllTech(m_uid);
    ImGui::SameLine();
    if (ImGui::Button("Remove")) m_app.palDefenderRCON().removeTech(m_uid, m_techId);
    ImGui::SameLine();
    if (ImGui::Button("Remove ALL")) m_app.palDefenderRCON().removeAllTech(m_uid);
    ImGui::Spacing();
    if (ImGui::Button("View Tech IDs")) { m_app.palDefenderRCON().getAvailableTechIds(); }
    ImGui::SameLine();
    if (ImGui::Button("View Skin IDs")) { m_app.palDefenderRCON().getAvailableSkinIds(); }
}
} // namespace PSM
