#include "ui/MapRenderer.h"
#include <algorithm>
#include <cmath>

namespace PSM {

MapRenderer::MapRenderer() {
    m_layerVisibility["players"] = true;
    m_layerVisibility["bases"] = true;
    m_layerVisibility["heatmap"] = false;
    m_layerVisibility["grid"] = true;
}

MapRenderer::~MapRenderer() {}

void MapRenderer::setMapBounds(float minX, float minY, float maxX, float maxY) {
    m_worldMinX = minX;
    m_worldMinY = minY;
    m_worldMaxX = maxX;
    m_worldMaxY = maxY;
}

void MapRenderer::setViewport(float centerX, float centerY, float zoom) {
    m_viewCenterX = centerX;
    m_viewCenterY = centerY;
    m_zoom = std::clamp(zoom, m_minZoom, m_maxZoom);
}

void MapRenderer::beginRender(ImVec2 canvasPos, ImVec2 canvasSize) {
    m_canvasPos = canvasPos;
    m_canvasSize = canvasSize;
    m_drawList = ImGui::GetWindowDrawList();
    m_tooltip.active = false;
}

void MapRenderer::renderBackground() {
    m_drawList->AddRectFilled(
        m_canvasPos,
        ImVec2(m_canvasPos.x + m_canvasSize.x, m_canvasPos.y + m_canvasSize.y),
        IM_COL32(15, 20, 28, 255));
}

void MapRenderer::renderGrid() {
    if (!isLayerVisible("grid")) return;

    // Calculate grid spacing based on zoom
    float baseSpacing = 10000.0f; // World units
    float screenSpacing = baseSpacing * m_zoom * m_canvasSize.x / (m_worldMaxX - m_worldMinX);

    // Adjust spacing to keep grid readable
    while (screenSpacing < 30.0f) {
        baseSpacing *= 2.0f;
        screenSpacing = baseSpacing * m_zoom * m_canvasSize.x / (m_worldMaxX - m_worldMinX);
    }
    while (screenSpacing > 200.0f) {
        baseSpacing *= 0.5f;
        screenSpacing = baseSpacing * m_zoom * m_canvasSize.x / (m_worldMaxX - m_worldMinX);
    }

    ImU32 gridColor = IM_COL32(35, 40, 50, 120);
    ImU32 gridColorMajor = IM_COL32(45, 50, 65, 160);

    // Vertical lines
    float startX = std::floor((m_viewCenterX - (m_worldMaxX - m_worldMinX) / (2 * m_zoom)) / baseSpacing) * baseSpacing;
    for (float wx = startX; ; wx += baseSpacing) {
        ImVec2 screen = worldToScreen(wx, 0);
        if (screen.x > m_canvasPos.x + m_canvasSize.x) break;
        if (screen.x < m_canvasPos.x) continue;

        bool isMajor = (std::fmod(wx, baseSpacing * 5.0f) < 1.0f);
        m_drawList->AddLine(
            ImVec2(screen.x, m_canvasPos.y),
            ImVec2(screen.x, m_canvasPos.y + m_canvasSize.y),
            isMajor ? gridColorMajor : gridColor);
    }

    // Horizontal lines
    float startY = std::floor((m_viewCenterY - (m_worldMaxY - m_worldMinY) / (2 * m_zoom)) / baseSpacing) * baseSpacing;
    for (float wy = startY; ; wy += baseSpacing) {
        ImVec2 screen = worldToScreen(0, wy);
        if (screen.y > m_canvasPos.y + m_canvasSize.y) break;
        if (screen.y < m_canvasPos.y) continue;

        bool isMajor = (std::fmod(wy, baseSpacing * 5.0f) < 1.0f);
        m_drawList->AddLine(
            ImVec2(m_canvasPos.x, screen.y),
            ImVec2(m_canvasPos.x + m_canvasSize.x, screen.y),
            isMajor ? gridColorMajor : gridColor);
    }

    // Origin crosshair
    ImVec2 origin = worldToScreen(0, 0);
    if (origin.x >= m_canvasPos.x && origin.x <= m_canvasPos.x + m_canvasSize.x &&
        origin.y >= m_canvasPos.y && origin.y <= m_canvasPos.y + m_canvasSize.y) {
        m_drawList->AddLine(
            ImVec2(origin.x - 10, origin.y), ImVec2(origin.x + 10, origin.y),
            IM_COL32(200, 200, 200, 100));
        m_drawList->AddLine(
            ImVec2(origin.x, origin.y - 10), ImVec2(origin.x, origin.y + 10),
            IM_COL32(200, 200, 200, 100));
    }
}

void MapRenderer::renderPlayers(const std::vector<Player>& players) {
    if (!isLayerVisible("players")) return;

    ImVec2 mousePos = ImGui::GetMousePos();

    for (const auto& player : players) {
        ImVec2 screen = worldToScreen(player.position.x, player.position.y);

        // Check if on screen
        if (screen.x < m_canvasPos.x - 20 || screen.x > m_canvasPos.x + m_canvasSize.x + 20 ||
            screen.y < m_canvasPos.y - 20 || screen.y > m_canvasPos.y + m_canvasSize.y + 20) {
            continue;
        }

        float radius = 6.0f;
        ImU32 color = player.online ? IM_COL32(80, 200, 120, 255) : IM_COL32(120, 120, 130, 180);
        ImU32 borderColor = IM_COL32(255, 255, 255, 80);

        // Draw player dot
        m_drawList->AddCircleFilled(screen, radius, color);
        m_drawList->AddCircle(screen, radius + 1, borderColor);

        // Draw name label
        ImVec2 textPos(screen.x + radius + 4, screen.y - 6);
        m_drawList->AddText(textPos, IM_COL32(220, 220, 225, 220), player.name.c_str());

        // Hover detection
        float dist = std::sqrt(std::pow(mousePos.x - screen.x, 2) + std::pow(mousePos.y - screen.y, 2));
        if (dist < radius + 5) {
            m_tooltip.active = true;
            m_tooltip.title = player.name;
            m_tooltip.position = ImVec2(screen.x + 15, screen.y + 15);
            m_tooltip.lines.clear();
            m_tooltip.lines.push_back("UID: " + player.uid);
            m_tooltip.lines.push_back("Level: " + std::to_string(player.stats.level));
            m_tooltip.lines.push_back("Position: (" +
                std::to_string(static_cast<int>(player.position.x)) + ", " +
                std::to_string(static_cast<int>(player.position.y)) + ", " +
                std::to_string(static_cast<int>(player.position.z)) + ")");
            if (!player.guildName.empty()) {
                m_tooltip.lines.push_back("Guild: " + player.guildName);
            }

            // Click to select
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                m_selection.type = MapSelection::PlayerSel;
                m_selection.id = player.uid;
                m_selection.worldX = player.position.x;
                m_selection.worldY = player.position.y;
            }
        }
    }
}

void MapRenderer::renderBases(const std::vector<GuildBase>& bases,
                               const std::map<std::string, ImVec4>& guildColors) {
    if (!isLayerVisible("bases")) return;

    ImVec2 mousePos = ImGui::GetMousePos();

    for (const auto& base : bases) {
        ImVec2 screen = worldToScreen(base.x, base.y);

        if (screen.x < m_canvasPos.x - 30 || screen.x > m_canvasPos.x + m_canvasSize.x + 30 ||
            screen.y < m_canvasPos.y - 30 || screen.y > m_canvasPos.y + m_canvasSize.y + 30) {
            continue;
        }

        // Get guild color
        ImU32 baseColor;
        auto colorIt = guildColors.find(base.guildId);
        if (colorIt != guildColors.end()) {
            baseColor = ImGui::ColorConvertFloat4ToU32(colorIt->second);
        } else {
            baseColor = GUILD_COLORS[std::hash<std::string>{}(base.guildId) % 8];
        }

        float size = 10.0f;

        // Draw base marker (diamond shape)
        m_drawList->AddQuadFilled(
            ImVec2(screen.x, screen.y - size),
            ImVec2(screen.x + size, screen.y),
            ImVec2(screen.x, screen.y + size),
            ImVec2(screen.x - size, screen.y),
            baseColor);

        // Shiny/legendary highlight
        if (base.hasShinyPals) {
            m_drawList->AddCircle(screen, size + 4, IM_COL32(255, 215, 0, 180), 0, 2.0f);
        }
        if (base.hasLegendaryPals) {
            m_drawList->AddCircle(screen, size + 7, IM_COL32(200, 50, 255, 180), 0, 2.0f);
        }

        // Pal count badge
        if (base.palCount > 0) {
            char countBuf[16];
            snprintf(countBuf, sizeof(countBuf), "%d", base.palCount);
            ImVec2 badgePos(screen.x + size + 3, screen.y - size - 2);
            ImVec2 textSize = ImGui::CalcTextSize(countBuf);
            m_drawList->AddRectFilled(
                ImVec2(badgePos.x - 2, badgePos.y - 1),
                ImVec2(badgePos.x + textSize.x + 2, badgePos.y + textSize.y + 1),
                IM_COL32(40, 40, 50, 200), 3.0f);
            m_drawList->AddText(badgePos, IM_COL32(240, 240, 245, 255), countBuf);
        }

        // Guild name
        m_drawList->AddText(
            ImVec2(screen.x - size, screen.y + size + 3),
            IM_COL32(180, 180, 190, 200),
            base.guildName.c_str());

        // Hover tooltip
        float dist = std::max(std::abs(mousePos.x - screen.x), std::abs(mousePos.y - screen.y));
        if (dist < size + 5) {
            m_tooltip.active = true;
            m_tooltip.title = "Base: " + base.name;
            m_tooltip.position = ImVec2(screen.x + 20, screen.y + 20);
            m_tooltip.lines.clear();
            m_tooltip.lines.push_back("Guild: " + base.guildName);
            m_tooltip.lines.push_back("Level: " + std::to_string(base.level));
            m_tooltip.lines.push_back("Pals: " + std::to_string(base.palCount));
            m_tooltip.lines.push_back("State: " + base.state);
            if (base.hasShinyPals) m_tooltip.lines.push_back("✨ Has Shiny Pals!");
            if (base.hasLegendaryPals) m_tooltip.lines.push_back("⭐ Has Legendary Pals!");

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                m_selection.type = MapSelection::BaseSel;
                m_selection.id = base.id;
                m_selection.worldX = base.x;
                m_selection.worldY = base.y;
            }
        }
    }
}

void MapRenderer::renderHeatmap(const std::vector<std::tuple<float, float, float>>& points) {
    if (!isLayerVisible("heatmap") || points.empty()) return;

    // Find max weight for normalization
    float maxWeight = 0;
    for (auto& [x, y, w] : points) {
        maxWeight = std::max(maxWeight, w);
    }
    if (maxWeight <= 0) return;

    for (auto& [x, y, weight] : points) {
        ImVec2 screen = worldToScreen(x, y);

        if (screen.x < m_canvasPos.x - 50 || screen.x > m_canvasPos.x + m_canvasSize.x + 50 ||
            screen.y < m_canvasPos.y - 50 || screen.y > m_canvasPos.y + m_canvasSize.y + 50) {
            continue;
        }

        float normalizedWeight = weight / maxWeight;
        float radius = 15.0f + normalizedWeight * 30.0f;

        // Color gradient: blue -> green -> yellow -> red
        uint8_t r, g, b;
        if (normalizedWeight < 0.33f) {
            float t = normalizedWeight / 0.33f;
            r = 0; g = static_cast<uint8_t>(t * 200); b = static_cast<uint8_t>((1.0f - t) * 200);
        } else if (normalizedWeight < 0.66f) {
            float t = (normalizedWeight - 0.33f) / 0.33f;
            r = static_cast<uint8_t>(t * 255); g = 200; b = 0;
        } else {
            float t = (normalizedWeight - 0.66f) / 0.34f;
            r = 255; g = static_cast<uint8_t>((1.0f - t) * 200); b = 0;
        }

        uint8_t alpha = static_cast<uint8_t>(40 + normalizedWeight * 80);

        m_drawList->AddCircleFilled(screen, radius, IM_COL32(r, g, b, alpha));
    }
}

void MapRenderer::endRender() {
    // Render tooltip if active
    if (m_tooltip.active) {
        ImVec2 tooltipPos = m_tooltip.position;

        // Calculate tooltip size
        float maxWidth = 0;
        float totalHeight = ImGui::GetTextLineHeight() + 4; // title
        for (auto& line : m_tooltip.lines) {
            ImVec2 textSize = ImGui::CalcTextSize(line.c_str());
            maxWidth = std::max(maxWidth, textSize.x);
            totalHeight += ImGui::GetTextLineHeight();
        }
        maxWidth = std::max(maxWidth, ImGui::CalcTextSize(m_tooltip.title.c_str()).x);

        float padding = 8.0f;
        ImVec2 tooltipSize(maxWidth + padding * 2, totalHeight + padding * 2);

        // Ensure tooltip stays on screen
        if (tooltipPos.x + tooltipSize.x > m_canvasPos.x + m_canvasSize.x) {
            tooltipPos.x = m_canvasPos.x + m_canvasSize.x - tooltipSize.x;
        }
        if (tooltipPos.y + tooltipSize.y > m_canvasPos.y + m_canvasSize.y) {
            tooltipPos.y = m_canvasPos.y + m_canvasSize.y - tooltipSize.y;
        }

        // Background
        m_drawList->AddRectFilled(
            tooltipPos,
            ImVec2(tooltipPos.x + tooltipSize.x, tooltipPos.y + tooltipSize.y),
            IM_COL32(25, 28, 35, 230), 4.0f);
        m_drawList->AddRect(
            tooltipPos,
            ImVec2(tooltipPos.x + tooltipSize.x, tooltipPos.y + tooltipSize.y),
            IM_COL32(80, 85, 100, 150), 4.0f);

        // Title
        float yOffset = tooltipPos.y + padding;
        m_drawList->AddText(ImVec2(tooltipPos.x + padding, yOffset),
            IM_COL32(240, 240, 245, 255), m_tooltip.title.c_str());
        yOffset += ImGui::GetTextLineHeight() + 4;

        // Lines
        for (auto& line : m_tooltip.lines) {
            m_drawList->AddText(ImVec2(tooltipPos.x + padding, yOffset),
                IM_COL32(180, 180, 190, 220), line.c_str());
            yOffset += ImGui::GetTextLineHeight();
        }
    }

    // Render coordinate display
    ImVec2 mousePos = ImGui::GetMousePos();
    if (mousePos.x >= m_canvasPos.x && mousePos.x <= m_canvasPos.x + m_canvasSize.x &&
        mousePos.y >= m_canvasPos.y && mousePos.y <= m_canvasPos.y + m_canvasSize.y) {
        ImVec2 world = screenToWorld(mousePos);
        char coordBuf[64];
        snprintf(coordBuf, sizeof(coordBuf), "(%.0f, %.0f)", world.x, world.y);
        m_drawList->AddText(
            ImVec2(m_canvasPos.x + 8, m_canvasPos.y + m_canvasSize.y - 20),
            IM_COL32(150, 150, 160, 200), coordBuf);
    }

    // Zoom indicator
    char zoomBuf[32];
    snprintf(zoomBuf, sizeof(zoomBuf), "Zoom: %.1fx", m_zoom);
    m_drawList->AddText(
        ImVec2(m_canvasPos.x + m_canvasSize.x - 100, m_canvasPos.y + m_canvasSize.y - 20),
        IM_COL32(150, 150, 160, 200), zoomBuf);
}

bool MapRenderer::isHovered() const {
    ImVec2 mousePos = ImGui::GetMousePos();
    return mousePos.x >= m_canvasPos.x && mousePos.x <= m_canvasPos.x + m_canvasSize.x &&
           mousePos.y >= m_canvasPos.y && mousePos.y <= m_canvasPos.y + m_canvasSize.y;
}

ImVec2 MapRenderer::screenToWorld(ImVec2 screenPos) const {
    float worldWidth = (m_worldMaxX - m_worldMinX) / m_zoom;
    float worldHeight = (m_worldMaxY - m_worldMinY) / m_zoom;

    float relX = (screenPos.x - m_canvasPos.x) / m_canvasSize.x;
    float relY = (screenPos.y - m_canvasPos.y) / m_canvasSize.y;

    float worldX = m_viewCenterX + (relX - 0.5f) * worldWidth;
    float worldY = m_viewCenterY + (relY - 0.5f) * worldHeight;

    return ImVec2(worldX, worldY);
}

ImVec2 MapRenderer::worldToScreen(float worldX, float worldY) const {
    float worldWidth = (m_worldMaxX - m_worldMinX) / m_zoom;
    float worldHeight = (m_worldMaxY - m_worldMinY) / m_zoom;

    float relX = (worldX - m_viewCenterX) / worldWidth + 0.5f;
    float relY = (worldY - m_viewCenterY) / worldHeight + 0.5f;

    float screenX = m_canvasPos.x + relX * m_canvasSize.x;
    float screenY = m_canvasPos.y + relY * m_canvasSize.y;

    return ImVec2(screenX, screenY);
}

void MapRenderer::handleInput() {
    if (!isHovered()) {
        m_isPanning = false;
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    // Zoom with mouse wheel
    if (std::abs(io.MouseWheel) > 0.01f) {
        float zoomFactor = 1.0f + io.MouseWheel * 0.15f;
        float newZoom = std::clamp(m_zoom * zoomFactor, m_minZoom, m_maxZoom);

        // Zoom toward mouse cursor
        ImVec2 mouseWorld = screenToWorld(io.MousePos);
        m_zoom = newZoom;
        ImVec2 newMouseWorld = screenToWorld(io.MousePos);

        m_viewCenterX -= (newMouseWorld.x - mouseWorld.x);
        m_viewCenterY -= (newMouseWorld.y - mouseWorld.y);
    }

    // Pan with middle mouse button or left + alt
    bool panButton = ImGui::IsMouseDown(ImGuiMouseButton_Middle) ||
                     (ImGui::IsMouseDown(ImGuiMouseButton_Left) && io.KeyAlt);

    if (panButton && !m_isPanning) {
        m_isPanning = true;
        m_panStart = io.MousePos;
        m_panStartCenterX = m_viewCenterX;
        m_panStartCenterY = m_viewCenterY;
    }

    if (m_isPanning) {
        if (panButton) {
            ImVec2 delta(io.MousePos.x - m_panStart.x, io.MousePos.y - m_panStart.y);

            float worldWidth = (m_worldMaxX - m_worldMinX) / m_zoom;
            float worldHeight = (m_worldMaxY - m_worldMinY) / m_zoom;

            m_viewCenterX = m_panStartCenterX - delta.x * worldWidth / m_canvasSize.x;
            m_viewCenterY = m_panStartCenterY - delta.y * worldHeight / m_canvasSize.y;
        } else {
            m_isPanning = false;
        }
    }

    // Right click for coordinate selection
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImVec2 world = screenToWorld(io.MousePos);
        m_selection.type = MapSelection::Coordinate;
        m_selection.id = "";
        m_selection.worldX = world.x;
        m_selection.worldY = world.y;
    }
}

void MapRenderer::setLayerVisible(const std::string& layer, bool visible) {
    m_layerVisibility[layer] = visible;
}

bool MapRenderer::isLayerVisible(const std::string& layer) const {
    auto it = m_layerVisibility.find(layer);
    if (it != m_layerVisibility.end()) return it->second;
    return true;
}

MapRenderer::MapSelection MapRenderer::getSelection() const {
    return m_selection;
}

void MapRenderer::clearSelection() {
    m_selection.type = MapSelection::None;
}

MapRenderer::MapTooltip MapRenderer::getTooltip() const {
    return m_tooltip;
}

} // namespace PSM
