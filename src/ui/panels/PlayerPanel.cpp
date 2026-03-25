#include "ui/panels/PlayerPanel.h"
#include "Application.h"
#include "core/Database.h"
#include "net/VanillaAPI.h"
#include "net/PalDefenderAPI.h"
#include "net/PalDefenderRCON.h"
#include "pst/LevelSavParser.h"
#include "ui/Widgets.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>

namespace PSM {

PlayerPanel::PlayerPanel(Application& app) : m_app(app) {}
PlayerPanel::~PlayerPanel() {}

void PlayerPanel::render() {
    // Update cache
    m_cacheTimer += ImGui::GetIO().DeltaTime;
    if (m_cacheTimer >= 2.0f) {
        m_cacheTimer = 0;
        m_cachedOnline = m_app.serverManager().getOnlinePlayers();
        m_cachedAll = m_app.levelSavParser().getAllPlayers();
    }

    ImGui::Text("👥 Player Management");
    ImGui::Separator();
    ImGui::Spacing();

    renderToolbar();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##PlayerTabs")) {
        if (ImGui::BeginTabItem("Online")) {
            renderOnlineTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("All Players")) {
            renderAllPlayersTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Whitelist")) {
            renderWhitelistDialog();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // Detail side panel
    if (m_showPlayerDetail && !m_selectedPlayerUid.empty()) {
        ImGui::SameLine();
        ImGui::BeginChild("##PlayerDetail", ImVec2(400, 0), true);
        Player p;
        for (auto& pl : m_cachedAll) {
            if (pl.uid == m_selectedPlayerUid) { p = pl; break; }
        }
        if (p.uid.empty()) {
            for (auto& pl : m_cachedOnline) {
                if (pl.uid == m_selectedPlayerUid) { p = pl; break; }
            }
        }
        if (!p.uid.empty()) {
            renderPlayerDetail(p);
        }
        ImGui::EndChild();
    }

    // Dialogs
    renderKickDialog();
    renderBanDialog();
    renderRenameDialog();
    renderGiveExpDialog();
    renderGiveItemDialog();
}

void PlayerPanel::renderToolbar() {
    Widgets::SearchInput("##playerSearch", m_searchBuf, sizeof(m_searchBuf), "Search by name or UID...");

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200);
    ImGui::Text("Online: %zu | Total: %zu", m_cachedOnline.size(), m_cachedAll.size());
}

void PlayerPanel::renderOnlineTab() {
    if (ImGui::BeginTable("##OnlinePlayers", 7,
        ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Sortable)) {

        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("UID", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("Guild", ImGuiTableColumnFlags_WidthFixed, 120);
        ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 250);
        ImGui::TableHeadersRow();

        std::string filter(m_searchBuf);
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

        for (auto& player : m_cachedOnline) {
            std::string lowerName = player.name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::string lowerUid = player.uid;
            std::transform(lowerUid.begin(), lowerUid.end(), lowerUid.begin(), ::tolower);

            if (!filter.empty() && lowerName.find(filter) == std::string::npos &&
                lowerUid.find(filter) == std::string::npos) continue;

            ImGui::TableNextRow();

            // Status dot
            ImGui::TableNextColumn();
            ImVec2 dotPos = ImGui::GetCursorScreenPos();
            dotPos.x += 10; dotPos.y += 8;
            ImGui::GetWindowDrawList()->AddCircleFilled(dotPos, 5, IM_COL32(80, 200, 120, 255));
            ImGui::Dummy(ImVec2(20, 0));

            // Name (clickable)
            ImGui::TableNextColumn();
            ImGui::PushID(player.uid.c_str());
            if (ImGui::Selectable(player.name.c_str(), m_selectedPlayerUid == player.uid,
                ImGuiSelectableFlags_SpanAllColumns)) {
                m_selectedPlayerUid = player.uid;
                m_showPlayerDetail = true;
            }
            ImGui::PopID();

            // UID
            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "%s", player.uid.c_str());
            if (ImGui::IsItemClicked()) {
                ImGui::SetClipboardText(player.uid.c_str());
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Click to copy UID");
            }

            // Level
            ImGui::TableNextColumn();
            ImGui::Text("%d", player.stats.level);

            // Guild
            ImGui::TableNextColumn();
            ImGui::Text("%s", player.guildName.c_str());

            // Suspicion score
            ImGui::TableNextColumn();
            int score = m_app.database().getPlayerSuspicionScore(player.uid);
            if (score > 50) {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.25f, 1), "%d", score);
            } else if (score > 20) {
                ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.2f, 1), "%d", score);
            } else if (score > 0) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.72f, 1), "%d", score);
            } else {
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.4f, 1), "✓");
            }

            // Actions
            ImGui::TableNextColumn();
            ImGui::PushID(("act_" + player.uid).c_str());
            if (ImGui::SmallButton("Kick")) {
                m_targetUid = player.uid;
                m_targetName = player.name;
                m_showKickDialog = true;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Ban")) {
                m_targetUid = player.uid;
                m_targetName = player.name;
                m_showBanDialog = true;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Msg")) {
                ImGui::OpenPopup("##QuickMsg");
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Give")) {
                m_targetUid = player.uid;
                m_targetName = player.name;
                m_showGiveItemDialog = true;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("...")) {
                ImGui::OpenPopup("##PlayerMore");
            }

            // Quick message popup
            if (ImGui::BeginPopup("##QuickMsg")) {
                static char msgBuf[256] = "";
                ImGui::InputText("##msg", msgBuf, sizeof(msgBuf));
                ImGui::SameLine();
                if (ImGui::Button("Send") && strlen(msgBuf) > 0) {
                    m_app.palDefenderRCON().sendMessage(player.uid, msgBuf);
                    msgBuf[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            // More actions popup
            if (ImGui::BeginPopup("##PlayerMore")) {
                if (ImGui::MenuItem("Rename")) {
                    m_targetUid = player.uid;
                    m_targetName = player.name;
                    m_showRenameDialog = true;
                }
                if (ImGui::MenuItem("Give EXP")) {
                    m_targetUid = player.uid;
                    m_targetName = player.name;
                    m_showGiveExpDialog = true;
                }
                if (ImGui::MenuItem("IP Ban")) {
                    m_app.palDefenderRCON().ipBanPlayer(player.uid);
                }
                if (ImGui::MenuItem("Get IP")) {
                    auto ip = m_app.palDefenderRCON().getPlayerIP(player.uid);
                    ImGui::SetClipboardText(ip.c_str());
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Grant Admin")) {
                    m_app.palDefenderRCON().grantAdmin(player.uid);
                }
                if (ImGui::MenuItem("Revoke Admin")) {
                    m_app.palDefenderRCON().revokeAdmin(player.uid);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Add to Whitelist")) {
                    m_app.palDefenderRCON().whitelistAdd(player.uid);
                }
                if (ImGui::MenuItem("Scan Player")) {
                    m_app.scanEngine().scanPlayer(player.uid);
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void PlayerPanel::renderAllPlayersTab() {
    if (ImGui::BeginTable("##AllPlayers", 6,
        ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("UID");
        ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("Last Seen");
        ImGui::TableSetupColumn("Play Time");
        ImGui::TableHeadersRow();

        std::string filter(m_searchBuf);
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

        for (auto& player : m_cachedAll) {
            std::string ln = player.name;
            std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
            if (!filter.empty() && ln.find(filter) == std::string::npos) continue;

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (player.online) {
                Widgets::StatusBadge("ON", ImVec4(0.3f, 0.8f, 0.4f, 1));
            } else {
                Widgets::StatusBadge("OFF", ImVec4(0.4f, 0.4f, 0.45f, 1));
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable(player.name.c_str(), m_selectedPlayerUid == player.uid)) {
                m_selectedPlayerUid = player.uid;
                m_showPlayerDetail = true;
            }

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "%s", player.uid.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%d", player.stats.level);

            ImGui::TableNextColumn();
            if (player.lastSeen > 0) {
                auto t = static_cast<time_t>(player.lastSeen);
                std::tm tm{};
#ifdef _WIN32
                localtime_s(&tm, &t);
#else
                localtime_r(&t, &tm);
#endif
                char buf[32];
                std::strftime(buf, sizeof(buf), "%m/%d %H:%M", &tm);
                ImGui::Text("%s", buf);
            }

            ImGui::TableNextColumn();
            int hours = static_cast<int>(player.totalPlaytime / 3600);
            int mins = static_cast<int>((player.totalPlaytime % 3600) / 60);
            ImGui::Text("%dh %dm", hours, mins);
        }

        ImGui::EndTable();
    }
}

void PlayerPanel::renderPlayerDetail(const Player& player) {
    if (ImGui::Button("✕ Close")) {
        m_showPlayerDetail = false;
        return;
    }

    ImGui::Spacing();
    ImGui::Text("%s", player.name.c_str());
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "UID: %s", player.uid.c_str());
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "Steam: %s", player.steamId.c_str());

    ImGui::Spacing();
    Widgets::SeparatorText("Stats");
    ImGui::Text("Level: %d", player.stats.level);
    ImGui::Text("HP: %d / %d", player.stats.hp, player.stats.maxHp);
    ImGui::Text("Stamina: %d", player.stats.stamina);
    ImGui::Text("Attack: %d", player.stats.attack);
    ImGui::Text("Defense: %d", player.stats.defense);
    ImGui::Text("Weight: %d", player.stats.weight);
    ImGui::Text("Craft Speed: %d", player.stats.craftSpeed);

    ImGui::Spacing();
    Widgets::SeparatorText("Tech");
    ImGui::Text("Tech Points: %d", player.stats.techPoints);
    ImGui::Text("Ancient Tech: %d", player.stats.ancientTechPoints);
    ImGui::Text("Effigies: %d", player.stats.effigies);
    ImGui::Text("Unlocked Techs: %zu", player.unlockedTechs.size());

    ImGui::Spacing();
    Widgets::SeparatorText("Position");
    ImGui::Text("X: %.0f  Y: %.0f  Z: %.0f", player.position.x, player.position.y, player.position.z);

    if (!player.guildName.empty()) {
        ImGui::Spacing();
        Widgets::SeparatorText("Guild");
        ImGui::Text("%s", player.guildName.c_str());
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Suspicion");
    int score = m_app.database().getPlayerSuspicionScore(player.uid);
    ImVec4 scoreColor = score > 50 ? ImVec4(0.9f, 0.3f, 0.25f, 1) :
                        (score > 20 ? ImVec4(0.95f, 0.75f, 0.2f, 1) :
                                     ImVec4(0.3f, 0.8f, 0.4f, 1));
    ImGui::TextColored(scoreColor, "Score: %d", score);

    ImGui::Spacing();
    renderPlayerActions(player);
}

void PlayerPanel::renderPlayerActions(const Player& player) {
    Widgets::SeparatorText("Quick Actions");

    if (ImGui::Button("Give EXP", ImVec2(-1, 0))) {
        m_targetUid = player.uid;
        m_targetName = player.name;
        m_showGiveExpDialog = true;
    }
    if (ImGui::Button("Give Item", ImVec2(-1, 0))) {
        m_targetUid = player.uid;
        m_targetName = player.name;
        m_showGiveItemDialog = true;
    }
    if (ImGui::Button("Teleport To...", ImVec2(-1, 0))) {
        // Would open player picker
    }

    ImGui::Spacing();
    if (ImGui::Button("Kick", ImVec2(-1, 0))) {
        m_targetUid = player.uid;
        m_targetName = player.name;
        m_showKickDialog = true;
    }

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.15f, 0.15f, 1));
    if (ImGui::Button("Ban", ImVec2(-1, 0))) {
        m_targetUid = player.uid;
        m_targetName = player.name;
        m_showBanDialog = true;
    }
    ImGui::PopStyleColor();
}

void PlayerPanel::renderKickDialog() {
    if (!m_showKickDialog) return;

    ImGui::OpenPopup("Kick Player");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Kick Player", &m_showKickDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Kick %s?", m_targetName.c_str());
        ImGui::InputText("Reason", m_kickReason, sizeof(m_kickReason));

        ImGui::Spacing();
        if (ImGui::Button("Kick", ImVec2(120, 0))) {
            m_app.palDefenderRCON().kickPlayer(m_targetUid, m_kickReason);
            m_kickReason[0] = '\0';
            m_showKickDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_showKickDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void PlayerPanel::renderBanDialog() {
    if (!m_showBanDialog) return;

    ImGui::OpenPopup("Ban Player");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Ban Player", &m_showBanDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.25f, 1), "⚠ Ban %s?", m_targetName.c_str());
        ImGui::Text("This will permanently ban the player.");
        ImGui::InputText("Reason", m_banReason, sizeof(m_banReason));

        static bool alsoIpBan = false;
        ImGui::Checkbox("Also IP ban", &alsoIpBan);

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.15f, 0.15f, 1));
        if (ImGui::Button("Ban", ImVec2(120, 0))) {
            m_app.palDefenderRCON().banPlayer(m_targetUid, m_banReason);
            if (alsoIpBan) {
                m_app.palDefenderRCON().ipBanPlayer(m_targetUid);
            }
            m_banReason[0] = '\0';
            m_showBanDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_showBanDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void PlayerPanel::renderRenameDialog() {
    if (!m_showRenameDialog) return;

    ImGui::OpenPopup("Rename Player");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Rename Player", &m_showRenameDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Rename %s", m_targetName.c_str());
        ImGui::InputText("New Name", m_newName, sizeof(m_newName));

        if (ImGui::Button("Rename", ImVec2(120, 0)) && strlen(m_newName) > 0) {
            m_app.palDefenderRCON().renamePlayer(m_targetUid, m_newName);
            m_newName[0] = '\0';
            m_showRenameDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_showRenameDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void PlayerPanel::renderGiveExpDialog() {
    if (!m_showGiveExpDialog) return;

    ImGui::OpenPopup("Give Rewards");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Give Rewards", &m_showGiveExpDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Give to: %s", m_targetName.c_str());
        ImGui::Separator();

        ImGui::InputInt("EXP", &m_giveExpAmount);
        ImGui::SameLine();
        if (ImGui::Button("Give##exp") && m_giveExpAmount > 0) {
            m_app.palDefenderRCON().giveExp(m_targetUid, m_giveExpAmount);
        }

        ImGui::InputInt("Status Points", &m_giveStatusPoints);
        ImGui::SameLine();
        if (ImGui::Button("Give##sp") && m_giveStatusPoints > 0) {
            m_app.palDefenderRCON().giveStatusPoints(m_targetUid, m_giveStatusPoints);
        }

        ImGui::InputInt("Tech Points", &m_giveTechPoints);
        ImGui::SameLine();
        if (ImGui::Button("Give##tp") && m_giveTechPoints > 0) {
            m_app.palDefenderRCON().giveTechPoints(m_targetUid, m_giveTechPoints);
        }

        ImGui::InputInt("Ancient Tech Points", &m_giveAncientPoints);
        ImGui::SameLine();
        if (ImGui::Button("Give##atp") && m_giveAncientPoints > 0) {
            m_app.palDefenderRCON().giveAncientTechPoints(m_targetUid, m_giveAncientPoints);
        }

        ImGui::InputInt("Effigies", &m_giveEffigies);
        ImGui::SameLine();
        if (ImGui::Button("Give##eff") && m_giveEffigies > 0) {
            m_app.palDefenderRCON().giveEffigies(m_targetUid, m_giveEffigies);
        }

        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(-1, 0))) {
            m_showGiveExpDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void PlayerPanel::renderGiveItemDialog() {
    if (!m_showGiveItemDialog) return;

    ImGui::OpenPopup("Give Item");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Give Item", &m_showGiveItemDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Give to: %s", m_targetName.c_str());
        ImGui::Separator();

        ImGui::InputText("Item ID", m_giveItemId, sizeof(m_giveItemId));
        ImGui::InputInt("Count", &m_giveItemCount);
        if (m_giveItemCount < 1) m_giveItemCount = 1;

        if (ImGui::Button("Give Item", ImVec2(120, 0)) && strlen(m_giveItemId) > 0) {
            m_app.palDefenderRCON().giveItem(m_targetUid, m_giveItemId, m_giveItemCount);
        }
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            m_showGiveItemDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void PlayerPanel::renderWhitelistDialog() {
    static char wlUid[128] = "";

    ImGui::InputText("Player UID", wlUid, sizeof(wlUid));
    ImGui::SameLine();
    if (ImGui::Button("Add") && strlen(wlUid) > 0) {
        m_app.palDefenderRCON().whitelistAdd(wlUid);
        wlUid[0] = '\0';
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove") && strlen(wlUid) > 0) {
        m_app.palDefenderRCON().whitelistRemove(wlUid);
        wlUid[0] = '\0';
    }

    ImGui::Spacing();
    if (ImGui::Button("Refresh Whitelist")) {
        auto result = m_app.palDefenderRCON().whitelistView();
        // Parse and display
        ImGui::TextWrapped("%s", result.c_str());
    }
}

} // namespace PSM
