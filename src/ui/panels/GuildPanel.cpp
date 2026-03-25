#include "ui/panels/GuildPanel.h"
#include "Application.h"
#include "net/PalDefenderAPI.h"
#include "net/PalDefenderRCON.h"
#include "ui/Widgets.h"
#include <imgui.h>
#include <algorithm>

namespace PSM {

GuildPanel::GuildPanel(Application& app) : m_app(app) {}
GuildPanel::~GuildPanel() {}

void GuildPanel::render() {
    m_cacheTimer += ImGui::GetIO().DeltaTime;
    if (m_cacheTimer >= 10.0f) {
        m_cacheTimer = 0;
        m_cachedGuilds = m_app.palDefenderAPI().fetchAllGuilds();
    }

    ImGui::Text("🏰 Guild Management");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Export All (JSON)")) {
        m_app.palDefenderRCON().exportGuildsToJson();
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        m_cacheTimer = 99.0f;
    }
    ImGui::SameLine();
    Widgets::SearchInput("##guildSearch", m_searchBuf, sizeof(m_searchBuf), "Search guilds...");

    ImGui::Spacing();

    // Two-column: list + detail
    float listWidth = 300;
    ImGui::BeginChild("##GuildList", ImVec2(listWidth, 0), true);
    renderGuildList();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##GuildDetail", ImVec2(0, 0), true);
    for (auto& g : m_cachedGuilds) {
        if (g.id == m_selectedGuildId) {
            renderGuildDetail(g);
            break;
        }
    }
    if (m_selectedGuildId.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "Select a guild");
    }
    ImGui::EndChild();
}

void GuildPanel::renderGuildList() {
    std::string filter(m_searchBuf);
    std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    for (auto& guild : m_cachedGuilds) {
        std::string ln = guild.name;
        std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
        if (!filter.empty() && ln.find(filter) == std::string::npos) continue;

        bool selected = (guild.id == m_selectedGuildId);
        char label[256];
        snprintf(label, sizeof(label), "%s (%d members)##%s",
            guild.name.c_str(), guild.memberCount, guild.id.c_str());

        if (ImGui::Selectable(label, selected)) {
            m_selectedGuildId = guild.id;
        }
    }
}

void GuildPanel::renderGuildDetail(const Guild& guild) {
    ImGui::Text("%s", guild.name.c_str());
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "ID: %s", guild.id.c_str());
    ImGui::Text("Leader: %s", guild.leaderName.c_str());
    ImGui::Text("Members: %d", guild.memberCount);

    ImGui::Spacing();
    Widgets::SeparatorText("Members");

    if (ImGui::BeginTable("##GuildMembers", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Role", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableHeadersRow();

        for (auto& m : guild.members) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%s", m.name.c_str());
            ImGui::TableNextColumn();
            if (m.online) Widgets::StatusBadge("ON", ImVec4(0.3f, 0.8f, 0.4f, 1));
            else Widgets::StatusBadge("OFF", ImVec4(0.4f, 0.4f, 0.45f, 1));
            ImGui::TableNextColumn();
            ImGui::Text("%s", m.isLeader ? "Leader" : "Member");
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Bases");

    for (auto& base : guild.bases) {
        if (ImGui::TreeNode(base.id.c_str(), "Lv.%d — %d pals — %s",
            base.level, base.palCount, base.state.c_str())) {
            ImGui::Text("Position: (%.0f, %.0f, %.0f)", base.x, base.y, base.z);
            if (base.hasShinyPals) Widgets::TextSuccess("✨ Contains Shiny Pals");
            if (base.hasLegendaryPals) Widgets::TextInfo("⭐ Contains Legendary Pals");
            ImGui::TreePop();
        }
    }

    if (!guild.expeditions.empty()) {
        ImGui::Spacing();
        Widgets::SeparatorText("Expeditions");
        for (auto& exp : guild.expeditions) {
            ImGui::Text("%s — %s (%.0f%%)", exp.id.c_str(), exp.status.c_str(), exp.progress * 100);
        }
    }

    if (!guild.labResearch.empty()) {
        ImGui::Spacing();
        Widgets::SeparatorText("Lab Research");
        for (auto& lab : guild.labResearch) {
            ImGui::Text("%s — %s (%.0f%%)", lab.name.c_str(), lab.status.c_str(), lab.progress * 100);
        }
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Actions");
    static char newLeaderUid[128] = "";
    ImGui::InputText("New Leader UID", newLeaderUid, sizeof(newLeaderUid));
    ImGui::SameLine();
    if (ImGui::Button("Set Leader") && strlen(newLeaderUid) > 0) {
        m_app.palDefenderRCON().setGuildLeader(guild.id, newLeaderUid);
        newLeaderUid[0] = '\0';
    }
}

} // namespace PSM
