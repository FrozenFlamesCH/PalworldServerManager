// AnalyticsPanel.cpp
#include "ui/panels/AnalyticsPanel.h"
#include "Application.h"
#include "core/Database.h"
#include "ui/Widgets.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace PSM {
void AnalyticsPanel::render() {
    ImGui::Text("📈 Analytics & Logs");
    ImGui::Separator();
    ImGui::Spacing();
    static const char* cats[] = {"All","death","cheat","rcon","chat","building","capture","craft","tech","oil_rig","helicopter","session"};
    ImGui::Combo("Category", &m_selectedCategory, cats, IM_ARRAYSIZE(cats));
    ImGui::Spacing();
    std::string catFilter = m_selectedCategory == 0 ? "" : cats[m_selectedCategory];
    auto logs = m_app.database().getLogs(catFilter, 200);
    if (ImGui::BeginTable("##Logs", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Time"); ImGui::TableSetupColumn("Category");
        ImGui::TableSetupColumn("Player"); ImGui::TableSetupColumn("Message");
        ImGui::TableHeadersRow();
        for (auto& l : logs) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%s", l.value("timestamp","").c_str());
            ImGui::TableNextColumn();
            std::string c = l.value("category","");
            ImVec4 cc = (c=="cheat") ? ImVec4(0.9f,0.3f,0.25f,1) : ImVec4(0.5f,0.5f,0.55f,1);
            ImGui::TextColored(cc, "%s", c.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%s", l.value("player_uid","").c_str());
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", l.value("message","").c_str());
        }
        ImGui::EndTable();
    }
}
} // namespace PSM
