#include "ui/panels/MapPanel.h"
#include "Application.h"
#include "server/ServerManager.h"
#include "net/PalDefenderAPI.h"
#include "net/PalDefenderRCON.h"
#include "core/Database.h"
#include "ui/Widgets.h"
#include <imgui.h>
#include <tuple>

namespace PSM {

MapPanel::MapPanel(Application& app) : m_app(app) {
    m_mapRenderer.setMapBounds(-500000, -500000, 500000, 500000);
}

MapPanel::~MapPanel() {}

void MapPanel::render() {
    ImGui::Text("🗺️ Map");
    ImGui::Separator();
    ImGui::Spacing();

    renderToolbar();
    ImGui::Spacing();

    // Map canvas
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float sidePanel = (m_mapRenderer.getSelection().type != MapRenderer::MapSelection::None) ? 300.0f : 0.0f;
    ImVec2 mapSize(avail.x - sidePanel - 10, avail.y);

    ImGui::BeginChild("##MapView", mapSize, true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    m_mapRenderer.beginRender(canvasPos, canvasSize);
    m_mapRenderer.renderBackground();
    m_mapRenderer.renderGrid();

    // Render heatmap
    if (m_mapRenderer.isLayerVisible("heatmap")) {
        auto heatData = m_app.database().getPositionHeatmapData();
        std::vector<std::tuple<float, float, float>> points;
        for (auto& row : heatData) {
            float x = std::stof(row.value("x", "0"));
            float y = std::stof(row.value("y", "0"));
            float w = std::stof(row.value("weight", "1"));
            points.emplace_back(x, y, w);
        }
        m_mapRenderer.renderHeatmap(points);
    }

    // Render bases
    if (m_mapRenderer.isLayerVisible("bases")) {
        auto guilds = m_app.palDefenderAPI().fetchAllGuilds();
        std::vector<GuildBase> allBases;
        std::map<std::string, ImVec4> guildColors;
        int colorIdx = 0;
        ImVec4 palette[] = {
            {0.9f,0.3f,0.3f,0.8f}, {0.3f,0.9f,0.3f,0.8f}, {0.3f,0.3f,0.9f,0.8f},
            {0.9f,0.9f,0.3f,0.8f}, {0.9f,0.3f,0.9f,0.8f}, {0.3f,0.9f,0.9f,0.8f},
            {0.9f,0.6f,0.3f,0.8f}, {0.6f,0.3f,0.9f,0.8f}
        };
        for (auto& g : guilds) {
            guildColors[g.id] = palette[colorIdx % 8];
            colorIdx++;
            for (auto& b : g.bases) {
                GuildBase base = b;
                base.guildId = g.id;
                base.guildName = g.name;
                allBases.push_back(base);
            }
        }
        m_mapRenderer.renderBases(allBases, guildColors);
    }

    // Render players
    auto players = m_app.serverManager().cachedPlayers();
    m_mapRenderer.renderPlayers(players);

    m_mapRenderer.endRender();
    m_mapRenderer.handleInput();

    // Right-click context menu
    auto sel = m_mapRenderer.getSelection();
    if (sel.type == MapRenderer::MapSelection::Coordinate) {
        m_contextWorldX = sel.worldX;
        m_contextWorldY = sel.worldY;
        ImGui::OpenPopup("##MapContext");
        m_mapRenderer.clearSelection();
    }

    renderContextMenu();

    ImGui::EndChild();

    // Selection side panel
    if (sidePanel > 0) {
        ImGui::SameLine();
        ImGui::BeginChild("##MapSelection", ImVec2(sidePanel, avail.y), true);
        renderSelectionPanel();
        ImGui::EndChild();
    }
}

void MapPanel::renderToolbar() {
    bool showPlayers = m_mapRenderer.isLayerVisible("players");
    bool showBases = m_mapRenderer.isLayerVisible("bases");
    bool showHeatmap = m_mapRenderer.isLayerVisible("heatmap");
    bool showGrid = m_mapRenderer.isLayerVisible("grid");

    if (ImGui::Checkbox("Players", &showPlayers))
        m_mapRenderer.setLayerVisible("players", showPlayers);
    ImGui::SameLine();
    if (ImGui::Checkbox("Bases", &showBases))
        m_mapRenderer.setLayerVisible("bases", showBases);
    ImGui::SameLine();
    if (ImGui::Checkbox("Heatmap", &showHeatmap))
        m_mapRenderer.setLayerVisible("heatmap", showHeatmap);
    ImGui::SameLine();
    if (ImGui::Checkbox("Grid", &showGrid))
        m_mapRenderer.setLayerVisible("grid", showGrid);

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
    if (ImGui::Button("Reset View")) {
        m_mapRenderer.setViewport(0, 0, 1.0f);
    }
}

void MapPanel::renderContextMenu() {
    if (ImGui::BeginPopup("##MapContext")) {
        ImGui::Text("Position: (%.0f, %.0f)", m_contextWorldX, m_contextWorldY);
        ImGui::Separator();

        if (ImGui::MenuItem("Spawn Pal Here")) {
            m_spawnX = m_contextWorldX;
            m_spawnY = m_contextWorldY;
            m_spawnZ = 0;
            m_showSpawnDialog = true;
        }
        if (ImGui::MenuItem("Find Nearest Base")) {
            auto result = m_app.palDefenderRCON().getNearestBase(m_contextWorldX, m_contextWorldY, 0);
            // Display result
        }
        if (ImGui::MenuItem("Copy Coordinates")) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.0f %.0f 0", m_contextWorldX, m_contextWorldY);
            ImGui::SetClipboardText(buf);
        }

        ImGui::EndPopup();
    }

    // Spawn dialog
    if (m_showSpawnDialog) {
        ImGui::OpenPopup("Spawn Pal");
    }
    if (ImGui::BeginPopupModal("Spawn Pal", &m_showSpawnDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Spawn at (%.0f, %.0f, %.0f)", m_spawnX, m_spawnY, m_spawnZ);
        ImGui::InputText("Pal ID", m_spawnPalId, sizeof(m_spawnPalId));
        ImGui::InputFloat("Z", &m_spawnZ);

        if (ImGui::Button("Spawn", ImVec2(120, 0)) && strlen(m_spawnPalId) > 0) {
            m_app.palDefenderRCON().spawnPal(m_spawnPalId, m_spawnX, m_spawnY, m_spawnZ);
            m_showSpawnDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_showSpawnDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void MapPanel::renderSelectionPanel() {
    auto sel = m_mapRenderer.getSelection();

    if (sel.type == MapRenderer::MapSelection::PlayerSel) {
        ImGui::Text("Selected Player");
        ImGui::Separator();
        ImGui::Text("UID: %s", sel.id.c_str());
        ImGui::Text("Pos: (%.0f, %.0f)", sel.worldX, sel.worldY);

        ImGui::Spacing();
        if (ImGui::Button("Teleport To...", ImVec2(-1, 0))) {
            strncpy(m_teleportUid1, sel.id.c_str(), sizeof(m_teleportUid1) - 1);
            m_showTeleportDialog = true;
        }
        if (ImGui::Button("Clear Selection", ImVec2(-1, 0))) {
            m_mapRenderer.clearSelection();
        }
    }
    else if (sel.type == MapRenderer::MapSelection::BaseSel) {
        ImGui::Text("Selected Base");
        ImGui::Separator();
        ImGui::Text("ID: %s", sel.id.c_str());

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.15f, 0.15f, 1));
        if (ImGui::Button("Destroy Base", ImVec2(-1, 0))) {
            m_destroyBaseId = sel.id;
            m_showDestroyConfirm = true;
        }
        ImGui::PopStyleColor();

        if (m_showDestroyConfirm) {
            if (Widgets::ConfirmDialog("Destroy Base",
                "Are you SURE you want to destroy this base? This cannot be undone!",
                &m_showDestroyConfirm)) {
                m_app.palDefenderRCON().destroyBase(m_destroyBaseId);
                m_mapRenderer.clearSelection();
            }
        }

        if (ImGui::Button("Clear Selection", ImVec2(-1, 0))) {
            m_mapRenderer.clearSelection();
        }
    }
}

} // namespace PSM
