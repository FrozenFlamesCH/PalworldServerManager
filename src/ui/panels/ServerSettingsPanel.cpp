// ServerSettingsPanel.cpp
#include "ui/panels/ServerSettingsPanel.h"
#include "Application.h"
#include "server/ServerManager.h"
#include <imgui.h>

namespace PSM {
void ServerSettingsPanel::render() {
    ImGui::Text("⚙️ Server Settings Viewer");
    ImGui::Separator();
    ImGui::Spacing();
    auto s = m_app.serverManager().getServerSettings();
    if (ImGui::BeginTable("##SS", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Setting", ImGuiTableColumnFlags_WidthFixed, 300);
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();
        auto row = [](const char* k, const std::string& v) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%s",k);
            ImGui::TableNextColumn(); ImGui::Text("%s",v.c_str());
        };
        row("EXP Rate", std::to_string(s.expRate));
        row("Capture Rate", std::to_string(s.captureRate));
        row("Pal Spawn Rate", std::to_string(s.palSpawnRate));
        row("Day Speed", std::to_string(s.dayTimeSpeed));
        row("Night Speed", std::to_string(s.nightTimeSpeed));
        row("Death Penalty", s.deathPenalty);
        row("PvP", s.isPvP ? "Yes" : "No");
        row("Friendly Fire", s.friendlyFire ? "Yes" : "No");
        row("Max Bases", std::to_string(s.baseCampMaxNum));
        row("Max Workers", std::to_string(s.baseCampWorkerMaxNum));
        row("Hatch Speed", std::to_string(s.hatchingSpeed));
        if (s.raw.is_object()) {
            for (auto& [k,v] : s.raw.items()) {
                row(k.c_str(), v.is_string() ? v.get<std::string>() : v.dump());
            }
        }
        ImGui::EndTable();
    }
}
} // namespace PSM
