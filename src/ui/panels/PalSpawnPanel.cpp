// PalSpawnPanel.cpp
#include "ui/panels/PalSpawnPanel.h"
#include "Application.h"
#include "net/PalDefenderRCON.h"
#include "net/PalDefenderAPI.h"
#include "ui/Widgets.h"
#include <imgui.h>
#include <cstring>

namespace PSM {
void PalSpawnPanel::render() {
    ImGui::Text("🐾 Pal Spawning & Summoning");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::InputText("Pal ID", m_palId, sizeof(m_palId));
    ImGui::InputText("Target UID", m_targetUid, sizeof(m_targetUid));
    ImGui::SliderInt("Level", &m_level, 1, 55);
    Widgets::SeparatorText("Give to Player");
    if (ImGui::Button("Give Pal") && strlen(m_palId) > 0 && strlen(m_targetUid) > 0)
        m_app.palDefenderRCON().givePal(m_targetUid, m_palId, m_level);
    ImGui::SameLine();
    if (ImGui::Button("Give Egg") && strlen(m_palId) > 0 && strlen(m_targetUid) > 0)
        m_app.palDefenderRCON().givePalEgg(m_targetUid, m_palId);
    Widgets::SeparatorText("Spawn at Coords");
    ImGui::InputFloat("X", &m_x); ImGui::InputFloat("Y", &m_y); ImGui::InputFloat("Z", &m_z);
    if (ImGui::Button("Spawn") && strlen(m_palId) > 0)
        m_app.palDefenderRCON().spawnPal(m_palId, m_x, m_y, m_z);
    Widgets::SeparatorText("Template / Summon");
    ImGui::InputText("Template Path", m_templatePath, sizeof(m_templatePath));
    if (ImGui::Button("From Template") && strlen(m_templatePath) > 0)
        m_app.palDefenderRCON().spawnPalFromTemplate(m_templatePath, m_x, m_y, m_z);
    ImGui::InputText("Summon File", m_summonFile, sizeof(m_summonFile));
    if (ImGui::Button("From Summon") && strlen(m_summonFile) > 0)
        m_app.palDefenderRCON().spawnPalFromSummon(m_summonFile);
}
} // namespace PSM
