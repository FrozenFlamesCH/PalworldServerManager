#pragma once

#include <vector>
#include <string>
#include <map>
#include <imgui.h>
#include "models/Player.h"
#include "models/Guild.h"
#include "models/Base.h"

namespace PSM {

class MapRenderer {
public:
    MapRenderer();
    ~MapRenderer();

    // Map settings
    void setMapBounds(float minX, float minY, float maxX, float maxY);
    void setViewport(float centerX, float centerY, float zoom);

    // Rendering
    void beginRender(ImVec2 canvasPos, ImVec2 canvasSize);
    void renderBackground();
    void renderGrid();
    void renderPlayers(const std::vector<Player>& players);
    void renderBases(const std::vector<GuildBase>& bases, const std::map<std::string, ImVec4>& guildColors);
    void renderHeatmap(const std::vector<std::tuple<float, float, float>>& points); // x, y, weight
    void endRender();

    // Interaction
    bool isHovered() const;
    ImVec2 screenToWorld(ImVec2 screenPos) const;
    ImVec2 worldToScreen(float worldX, float worldY) const;

    // Dragging / panning
    void handleInput();

    // Layer toggles
    void setLayerVisible(const std::string& layer, bool visible);
    bool isLayerVisible(const std::string& layer) const;

    // Selection
    struct MapSelection {
        enum Type { None, PlayerSel, BaseSel, Coordinate };
        Type type = None;
        std::string id;
        float worldX = 0, worldY = 0;
    };

    MapSelection getSelection() const;
    void clearSelection();

    // Tooltip
    struct MapTooltip {
        bool active = false;
        std::string title;
        std::vector<std::string> lines;
        ImVec2 position;
    };

    MapTooltip getTooltip() const;

private:
    ImVec2 m_canvasPos;
    ImVec2 m_canvasSize;
    ImDrawList* m_drawList = nullptr;

    // Map bounds (world coordinates)
    float m_worldMinX = -500000.0f;
    float m_worldMinY = -500000.0f;
    float m_worldMaxX = 500000.0f;
    float m_worldMaxY = 500000.0f;

    // View
    float m_viewCenterX = 0.0f;
    float m_viewCenterY = 0.0f;
    float m_zoom = 1.0f;
    float m_minZoom = 0.1f;
    float m_maxZoom = 10.0f;

    // Pan state
    bool m_isPanning = false;
    ImVec2 m_panStart;
    float m_panStartCenterX = 0;
    float m_panStartCenterY = 0;

    // Layers
    std::map<std::string, bool> m_layerVisibility;

    // Selection
    MapSelection m_selection;
    MapTooltip m_tooltip;

    // Guild color assignment
    int m_nextColorIndex = 0;
    static constexpr ImU32 GUILD_COLORS[] = {
        IM_COL32(255, 100, 100, 200),
        IM_COL32(100, 255, 100, 200),
        IM_COL32(100, 100, 255, 200),
        IM_COL32(255, 255, 100, 200),
        IM_COL32(255, 100, 255, 200),
        IM_COL32(100, 255, 255, 200),
        IM_COL32(255, 180, 100, 200),
        IM_COL32(180, 100, 255, 200),
    };
};

} // namespace PSM
