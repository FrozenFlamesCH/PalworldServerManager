#include "ui/panels/InventoryPanel.h"
#include "Application.h"
#include "pst/LevelSavParser.h"
#include "net/PalDefenderRCON.h"
#include "ui/Widgets.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>

namespace PSM {

InventoryPanel::InventoryPanel(Application& app) : m_app(app) {}
InventoryPanel::~InventoryPanel() {}

void InventoryPanel::render() {
    ImGui::Text("🎒 Player Inventory & Pals");
    ImGui::Separator();
    ImGui::Spacing();

    renderPlayerSelector();

    if (m_selectedUid.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "Select a player to view their data.");
        return;
    }

    ImGui::Spacing();

    // Action buttons
    if (ImGui::Button("Give Item")) m_showGiveItem = true;
    ImGui::SameLine();
    if (ImGui::Button("Give Pal")) m_showGivePal = true;
    ImGui::SameLine();
    if (ImGui::Button("Export Pals")) {
        m_app.palDefenderRCON().exportPals(m_selectedUid);
    }

    ImGui::Spacing();

    if (ImGui::BeginTabBar("##InvTabs")) {
        if (ImGui::BeginTabItem("Inventory")) {
            renderInventoryTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Pals")) {
            renderPalTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    renderGiveItemPopup();
    renderGivePalPopup();
}

void InventoryPanel::renderPlayerSelector() {
    auto allPlayers = m_app.levelSavParser().getAllPlayers();

    if (ImGui::BeginCombo("Player##invsel",
        m_selectedUid.empty() ? "Select player..." : m_selectedName.c_str())) {
        for (auto& p : allPlayers) {
            std::string label = p.name + " (" + p.uid.substr(0, 8) + "...)";
            bool selected = (m_selectedUid == p.uid);
            if (ImGui::Selectable(label.c_str(), selected)) {
                m_selectedUid = p.uid;
                m_selectedName = p.name;
            }
        }
        ImGui::EndCombo();
    }
}

void InventoryPanel::renderInventoryTab() {
    auto items = m_app.levelSavParser().getPlayerInventory(m_selectedUid);

    Widgets::SearchInput("##invSearch", m_searchBuf, sizeof(m_searchBuf), "Filter items...");
    ImGui::Spacing();

    std::string filter(m_searchBuf);
    std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    if (ImGui::BeginTable("##Items", 5,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable,
        ImVec2(0, ImGui::GetContentRegionAvail().y))) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Container", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableHeadersRow();

        for (auto& item : items) {
            std::string ln = item.name;
            std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
            if (!filter.empty() && ln.find(filter) == std::string::npos) continue;

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%s", item.name.c_str());

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "%s", item.staticId.c_str());

            ImGui::TableNextColumn();
            if (item.maxStack > 0 && item.count > item.maxStack) {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.25f, 1), "%d", item.count);
            } else {
                ImGui::Text("%d", item.count);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", item.container.c_str());

            ImGui::TableNextColumn();
            ImGui::PushID(item.id.c_str());
            if (ImGui::SmallButton("Del")) {
                m_app.palDefenderRCON().deleteItem(m_selectedUid, item.staticId, item.count);
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void InventoryPanel::renderPalTab() {
    auto pals = m_app.levelSavParser().getPlayerPals(m_selectedUid);

    ImGui::Text("Total Pals: %zu", pals.size());
    ImGui::Spacing();

    if (ImGui::BeginTable("##Pals", 8,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Resizable,
        ImVec2(0, ImGui::GetContentRegionAvail().y))) {

        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 25);
        ImGui::TableSetupColumn("Pal");
        ImGui::TableSetupColumn("Lv", ImGuiTableColumnFlags_WidthFixed, 35);
        ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed, 40);
        ImGui::TableSetupColumn("Gender", ImGuiTableColumnFlags_WidthFixed, 45);
        ImGui::TableSetupColumn("Talents");
        ImGui::TableSetupColumn("Passives");
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableHeadersRow();

        for (auto& pal : pals) {
            ImGui::TableNextRow();

            // Shiny indicator
            ImGui::TableNextColumn();
            if (pal.isShiny) {
                ImGui::TextColored(ImVec4(1, 0.85f, 0, 1), "✨");
            } else if (pal.isLegendary) {
                ImGui::TextColored(ImVec4(0.8f, 0.3f, 1, 1), "⭐");
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", pal.characterId.c_str());
            if (!pal.nickname.empty()) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "(%s)", pal.nickname.c_str());
            }

            ImGui::TableNextColumn();
            ImGui::Text("%d", pal.level);

            ImGui::TableNextColumn();
            if (pal.rank > 4) {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.25f, 1), "%d!", pal.rank);
            } else {
                ImGui::Text("%d", pal.rank);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", pal.gender == 0 ? "♂" : "♀");

            ImGui::TableNextColumn();
            ImGui::Text("H:%.0f M:%.0f S:%.0f D:%.0f",
                pal.talent_hp, pal.talent_melee, pal.talent_shot, pal.talent_defense);

            ImGui::TableNextColumn();
            for (size_t i = 0; i < pal.passives.size() && i < 4; ++i) {
                if (i > 0) { ImGui::SameLine(); ImGui::Text(","); ImGui::SameLine(); }
                ImGui::Text("%s", pal.passives[i].name.c_str());
            }

            ImGui::TableNextColumn();
            ImGui::PushID(pal.instanceId.c_str());
            if (ImGui::SmallButton("Del")) {
                m_app.palDefenderRCON().deletePals(m_selectedUid, pal.instanceId);
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void InventoryPanel::renderGiveItemPopup() {
    if (m_showGiveItem) ImGui::OpenPopup("Give Item##invpanel");

    if (ImGui::BeginPopupModal("Give Item##invpanel", &m_showGiveItem, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Give to: %s", m_selectedName.c_str());
        ImGui::InputText("Item ID##giveitem", m_itemId, sizeof(m_itemId));
        ImGui::InputInt("Count##givecount", &m_itemCount);
        if (m_itemCount < 1) m_itemCount = 1;

        if (ImGui::Button("Give", ImVec2(120, 0)) && strlen(m_itemId) > 0) {
            m_app.palDefenderRCON().giveItem(m_selectedUid, m_itemId, m_itemCount);
            ImGui::CloseCurrentPopup();
            m_showGiveItem = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##gi", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            m_showGiveItem = false;
        }
        ImGui::EndPopup();
    }
}

void InventoryPanel::renderGivePalPopup() {
    if (m_showGivePal) ImGui::OpenPopup("Give Pal##invpanel");

    if (ImGui::BeginPopupModal("Give Pal##invpanel", &m_showGivePal, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Give to: %s", m_selectedName.c_str());
        ImGui::InputText("Pal ID##givepal", m_palId, sizeof(m_palId));
        ImGui::SliderInt("Level##givepallv", &m_palLevel, 1, 55);

        if (ImGui::Button("Give Pal", ImVec2(120, 0)) && strlen(m_palId) > 0) {
            m_app.palDefenderRCON().givePal(m_selectedUid, m_palId, m_palLevel);
            ImGui::CloseCurrentPopup();
            m_showGivePal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Give Egg", ImVec2(120, 0)) && strlen(m_palId) > 0) {
            m_app.palDefenderRCON().givePalEgg(m_selectedUid, m_palId);
            ImGui::CloseCurrentPopup();
            m_showGivePal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##gp", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            m_showGivePal = false;
        }
        ImGui::EndPopup();
    }
}

} // namespace PSM
