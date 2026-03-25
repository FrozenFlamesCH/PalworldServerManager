#include "ui/panels/DashboardPanel.h"
#include "Application.h"
#include "core/Config.h"
#include "server/ServerManager.h"
#include "server/ProcessMonitor.h"
#include "scanner/ScanEngine.h"
#include "core/Database.h"
#include "ui/Widgets.h"

#include <imgui.h>
#include <cstdio>
#include <cmath>

namespace PSM {

DashboardPanel::DashboardPanel(Application& app) : m_app(app) {
    // Subscribe to events for recent activity
    app.eventBus().subscribe(EventBus::EVT_PLAYER_JOINED, [this](const Event& e) {
        ActivityEntry entry;
        entry.message = "Player joined";
        entry.category = "player";
        entry.timestamp = std::chrono::system_clock::now();
        m_recentActivity.insert(m_recentActivity.begin(), entry);
        if (m_recentActivity.size() > MAX_RECENT) m_recentActivity.pop_back();
    });

    app.eventBus().subscribe(EventBus::EVT_PLAYER_LEFT, [this](const Event& e) {
        ActivityEntry entry;
        entry.message = "Player left";
        entry.category = "player";
        entry.timestamp = std::chrono::system_clock::now();
        m_recentActivity.insert(m_recentActivity.begin(), entry);
        if (m_recentActivity.size() > MAX_RECENT) m_recentActivity.pop_back();
    });

    app.eventBus().subscribe(EventBus::EVT_SERVER_CRASHED, [this](const Event& e) {
        m_hasActiveAlerts = true;
        m_alertMessage = "⚠ SERVER CRASHED - Check process status";
        ActivityEntry entry;
        entry.message = "Server crashed!";
        entry.category = "server";
        entry.isAlert = true;
        entry.timestamp = std::chrono::system_clock::now();
        m_recentActivity.insert(m_recentActivity.begin(), entry);
    });

    app.eventBus().subscribe(EventBus::EVT_SCAN_FLAG, [this](const Event& e) {
        ActivityEntry entry;
        entry.message = "Scan flag raised";
        entry.category = "scan";
        entry.isAlert = true;
        entry.timestamp = std::chrono::system_clock::now();
        m_recentActivity.insert(m_recentActivity.begin(), entry);
        if (m_recentActivity.size() > MAX_RECENT) m_recentActivity.pop_back();
    });
}

DashboardPanel::~DashboardPanel() {}

void DashboardPanel::update(float deltaTime) {
    m_refreshTimer += deltaTime;
    if (m_refreshTimer >= m_refreshInterval) {
        m_refreshTimer = 0;

        auto& info = m_app.serverManager().cachedInfo();

        // Update FPS history
        m_fpsHistory[m_fpsHistoryOffset] = info.fps;
        m_frameTimeHistory[m_fpsHistoryOffset] = info.frameTime;
        m_fpsHistoryOffset = (m_fpsHistoryOffset + 1) % FPS_HISTORY_SIZE;
        if (m_fpsHistoryCount < FPS_HISTORY_SIZE) m_fpsHistoryCount++;
    }
}

void DashboardPanel::render() {
    update(ImGui::GetIO().DeltaTime);

    // Alert banner
    if (m_hasActiveAlerts) {
        renderAlertBanner();
    }

    ImGui::Text("📊 Dashboard");
    ImGui::Separator();
    ImGui::Spacing();

    renderServerStatus();
    ImGui::Spacing();

    renderQuickStats();
    ImGui::Spacing();
    ImGui::Spacing();

    renderFPSGraph();
    ImGui::Spacing();
    ImGui::Spacing();

    renderQuickActions();
    ImGui::Spacing();
    ImGui::Spacing();

    renderRecentActivity();
}

void DashboardPanel::renderAlertBanner() {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    float height = 30.0f;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Pulsing red background
    float pulse = (std::sin(static_cast<float>(ImGui::GetTime()) * 4.0f) + 1.0f) * 0.5f;
    uint8_t alpha = static_cast<uint8_t>(120 + pulse * 80);

    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height),
        IM_COL32(180, 30, 30, alpha), 4.0f);

    // Text centered
    ImVec2 textSize = ImGui::CalcTextSize(m_alertMessage.c_str());
    dl->AddText(
        ImVec2(pos.x + (width - textSize.x) * 0.5f, pos.y + (height - textSize.y) * 0.5f),
        IM_COL32(255, 255, 255, 255), m_alertMessage.c_str());

    ImGui::Dummy(ImVec2(width, height));

    // Dismiss button
    ImGui::SameLine(width - 80);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - height);
    if (ImGui::SmallButton("Dismiss")) {
        m_hasActiveAlerts = false;
    }

    ImGui::Spacing();
}

void DashboardPanel::renderServerStatus() {
    auto& info = m_app.serverManager().cachedInfo();
    auto processState = m_app.processMonitor().getState();

    // Server name and version
    ImGui::Text("Server: %s", info.name.empty() ? "Unknown" : info.name.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "v%s", info.version.c_str());

    if (!info.palDefenderVersion.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.65f, 0.95f, 1.0f),
            "| PalDefender v%s", info.palDefenderVersion.c_str());
    }

    // Process status line
    ImGui::Text("Process: ");
    ImGui::SameLine();
    ImVec4 stateColor;
    std::string stateLabel;
    switch (processState) {
        case ServerProcessState::Running:
            stateColor = ImVec4(0.3f, 0.8f, 0.4f, 1.0f);
            stateLabel = "● Running";
            break;
        case ServerProcessState::Starting:
            stateColor = ImVec4(0.95f, 0.75f, 0.2f, 1.0f);
            stateLabel = "◐ Starting";
            break;
        case ServerProcessState::Stopping:
            stateColor = ImVec4(0.95f, 0.75f, 0.2f, 1.0f);
            stateLabel = "◑ Stopping";
            break;
        case ServerProcessState::Crashed:
            stateColor = ImVec4(0.9f, 0.3f, 0.25f, 1.0f);
            stateLabel = "✕ Crashed";
            break;
        case ServerProcessState::Stopped:
        default:
            stateColor = ImVec4(0.5f, 0.5f, 0.55f, 1.0f);
            stateLabel = "○ Stopped";
            break;
    }
    ImGui::TextColored(stateColor, "%s", stateLabel.c_str());
}

void DashboardPanel::renderQuickStats() {
    auto& info = m_app.serverManager().cachedInfo();

    float totalWidth = ImGui::GetContentRegionAvail().x;
    float cardWidth = (totalWidth - 40) / 5.0f;
    float cardHeight = 75.0f;
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    struct StatInfo {
        const char* label;
        std::string value;
        ImVec4 color;
    };

    // Format uptime
    int upH = info.uptime / 3600;
    int upM = (info.uptime % 3600) / 60;
    char uptimeBuf[32];
    snprintf(uptimeBuf, sizeof(uptimeBuf), "%dh %dm", upH, upM);

    // Format player count
    char playerBuf[32];
    snprintf(playerBuf, sizeof(playerBuf), "%d / %d", info.currentPlayers, info.maxPlayers);

    // FPS color
    ImVec4 fpsColor = info.fps > 20 ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) :
                      (info.fps > 10 ? ImVec4(0.95f, 0.75f, 0.2f, 1.0f) :
                                      ImVec4(0.9f, 0.3f, 0.25f, 1.0f));

    char fpsBuf[16];
    snprintf(fpsBuf, sizeof(fpsBuf), "%.1f", info.fps);
    char dayBuf[16];
    snprintf(dayBuf, sizeof(dayBuf), "Day %d", info.inGameDay);

    StatInfo stats[] = {
        {"Status", info.online ? "ONLINE" : "OFFLINE",
         info.online ? ImVec4(0.3f,0.8f,0.4f,1) : ImVec4(0.9f,0.3f,0.25f,1)},
        {"Players", playerBuf, ImVec4(0.4f, 0.65f, 0.95f, 1.0f)},
        {"FPS", fpsBuf, fpsColor},
        {"Uptime", uptimeBuf, ImVec4(0.65f, 0.5f, 0.85f, 1.0f)},
        {"In-Game", dayBuf, ImVec4(0.85f, 0.65f, 0.2f, 1.0f)},
    };

    for (int i = 0; i < 5; ++i) {
        float x = startPos.x + i * (cardWidth + 10);
        float y = startPos.y;

        // Card background
        ImVec4 bgColor = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
        dl->AddRectFilled(
            ImVec2(x, y), ImVec2(x + cardWidth, y + cardHeight),
            ImGui::ColorConvertFloat4ToU32(bgColor), 6.0f);

        // Left accent bar
        dl->AddRectFilled(
            ImVec2(x, y), ImVec2(x + 4, y + cardHeight),
            ImGui::ColorConvertFloat4ToU32(stats[i].color), 6.0f);

        // Label
        dl->AddText(ImVec2(x + 14, y + 10),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.6f, 0.65f, 1.0f)),
            stats[i].label);

        // Value (larger font would be ideal)
        dl->AddText(ImVec2(x + 14, y + 32),
            ImGui::ColorConvertFloat4ToU32(stats[i].color),
            stats[i].value.c_str());
    }

    ImGui::Dummy(ImVec2(totalWidth, cardHeight + 4));
}

void DashboardPanel::renderFPSGraph() {
    ImGui::Text("Performance");
    ImGui::Separator();

    if (m_fpsHistoryCount > 0) {
        // Build display array (unwrap ring buffer)
        float displayFPS[FPS_HISTORY_SIZE];
        for (int i = 0; i < m_fpsHistoryCount; ++i) {
            int idx = (m_fpsHistoryOffset - m_fpsHistoryCount + i + FPS_HISTORY_SIZE) % FPS_HISTORY_SIZE;
            displayFPS[i] = m_fpsHistory[idx];
        }

        char overlay[32];
        snprintf(overlay, sizeof(overlay), "%.1f FPS", displayFPS[m_fpsHistoryCount - 1]);

        ImVec2 graphSize(ImGui::GetContentRegionAvail().x, 100);
        ImGui::PlotLines("##FPSGraph", displayFPS, m_fpsHistoryCount,
            0, overlay, 0.0f, 60.0f, graphSize);

        // Frame time graph
        float displayFT[FPS_HISTORY_SIZE];
        for (int i = 0; i < m_fpsHistoryCount; ++i) {
            int idx = (m_fpsHistoryOffset - m_fpsHistoryCount + i + FPS_HISTORY_SIZE) % FPS_HISTORY_SIZE;
            displayFT[i] = m_frameTimeHistory[idx];
        }

        char ftOverlay[32];
        snprintf(ftOverlay, sizeof(ftOverlay), "%.1fms", displayFT[m_fpsHistoryCount - 1]);

        ImGui::PlotLines("##FrameTimeGraph", displayFT, m_fpsHistoryCount,
            0, ftOverlay, 0.0f, 100.0f, ImVec2(graphSize.x, 60));
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "No FPS data yet...");
    }
}

void DashboardPanel::renderQuickActions() {
    ImGui::Text("Quick Actions");
    ImGui::Separator();
    ImGui::Spacing();

    auto processState = m_app.processMonitor().getState();
    bool serverRunning = (processState == ServerProcessState::Running);
    bool serverStopped = (processState == ServerProcessState::Stopped || processState == ServerProcessState::Crashed);

    ImVec2 btnSize(160, 36);

    // Start
    if (!serverStopped) ImGui::BeginDisabled();
    if (ImGui::Button("▶ Start Server", btnSize)) {
        m_app.serverManager().startServer();
    }
    if (!serverStopped) ImGui::EndDisabled();

    ImGui::SameLine();

    // Shutdown
    if (!serverRunning) ImGui::BeginDisabled();
    if (ImGui::Button("⏹ Shutdown", btnSize)) {
        ImGui::OpenPopup("##ShutdownConfirm");
    }
    if (!serverRunning) ImGui::EndDisabled();

    ImGui::SameLine();

    // Save
    if (ImGui::Button("💾 Save World", btnSize)) {
        m_app.serverManager().saveWorld();
    }

    ImGui::SameLine();

    // Broadcast
    if (ImGui::Button("📢 Broadcast", btnSize)) {
        ImGui::OpenPopup("##QuickBroadcast");
    }

    // Shutdown confirmation popup
    if (ImGui::BeginPopupModal("##ShutdownConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to shut down the server?");
        ImGui::Text("A countdown message will be broadcast to all players.");
        ImGui::Separator();
        if (ImGui::Button("Confirm Shutdown", ImVec2(150, 0))) {
            m_app.serverManager().gracefulShutdownWithCountdown();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Broadcast popup
    if (ImGui::BeginPopup("##QuickBroadcast")) {
        static char msg[256] = "";
        ImGui::Text("Broadcast Message");
        ImGui::InputText("##bcmsg", msg, sizeof(msg));
        if (ImGui::Button("Send") && strlen(msg) > 0) {
            m_app.serverManager().broadcastMessage(msg);
            msg[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void DashboardPanel::renderRecentActivity() {
    ImGui::Text("Recent Activity");
    ImGui::Separator();

    if (m_recentActivity.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "No recent activity");
        return;
    }

    ImGui::BeginChild("##RecentActivity", ImVec2(0, 200), true);
    for (auto& entry : m_recentActivity) {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - entry.timestamp).count();

        char timeBuf[32];
        if (elapsed < 60) {
            snprintf(timeBuf, sizeof(timeBuf), "%llds ago", (long long)elapsed);
        } else if (elapsed < 3600) {
            snprintf(timeBuf, sizeof(timeBuf), "%lldm ago", (long long)(elapsed / 60));
        } else {
            snprintf(timeBuf, sizeof(timeBuf), "%lldh ago", (long long)(elapsed / 3600));
        }

        ImVec4 color;
        if (entry.isAlert) {
            color = ImVec4(0.9f, 0.3f, 0.25f, 1.0f);
        } else if (entry.category == "player") {
            color = ImVec4(0.4f, 0.65f, 0.95f, 1.0f);
        } else if (entry.category == "server") {
            color = ImVec4(0.85f, 0.65f, 0.2f, 1.0f);
        } else {
            color = ImVec4(0.7f, 0.7f, 0.72f, 1.0f);
        }

        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "[%s]", timeBuf);
        ImGui::SameLine();
        ImGui::TextColored(color, "%s", entry.message.c_str());
    }
    ImGui::EndChild();
}

} // namespace PSM

Additional Scan Rules: Shiny Anomaly & Playtime Cross-Reference
cpp

Copy code
// src/scanner/rules/AdvancedScanRules.h
#pragma once

#include "scanner/ScanRule.h"
#include <chrono>

namespace PSM {

// Rule: Shiny pal accumulation rate anomaly
class ShinyAccumulationRule : public ScanRule {
public:
    ShinyAccumulationRule()
        : ScanRule("PAL_SHINY_RATE", "Shiny Pal Accumulation Anomaly",
                   ScanSeverity::High, "Player has abnormal shiny pal acquisition rate") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;

        int shinyCount = 0;
        for (auto& pal : ctx.pals) {
            if (pal.isShiny) shinyCount++;
        }

        // Flag if more than 10 shiny pals with less than 100 hours playtime
        float hoursPlayed = static_cast<float>(ctx.totalPlaytime) / 3600.0f;
        if (hoursPlayed < 1.0f) hoursPlayed = 1.0f; // Prevent division by zero

        float shinyPerHour = static_cast<float>(shinyCount) / hoursPlayed;

        // Normal rate is roughly 1 shiny per 10-20 hours
        if (shinyCount >= 10 && shinyPerHour > 0.5f) {
            flags.push_back(createFlag(ctx.player.uid,
                "Player has " + std::to_string(shinyCount) + " shiny pals in " +
                std::to_string(static_cast<int>(hoursPlayed)) + " hours (" +
                std::to_string(shinyPerHour).substr(0, 4) + " per hour)"));
        }

        return flags;
    }
};

// Rule: Cross-reference inventory vs playtime
class PlaytimeInventoryRule : public ScanRule {
public:
    PlaytimeInventoryRule()
        : ScanRule("INV_PLAYTIME_MISMATCH", "Inventory-Playtime Mismatch",
                   ScanSeverity::Medium,
                   "Player inventory value doesn't match their playtime") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;

        float hoursPlayed = static_cast<float>(ctx.totalPlaytime) / 3600.0f;
        if (hoursPlayed < 0.5f) return flags; // Skip very new players

        // Count rare/valuable items
        int rareItemCount = 0;
        int totalItemValue = 0;

        for (auto& item : ctx.inventory) {
            totalItemValue += item.count;

            // Check for items that should take significant time to acquire
            static const std::vector<std::string> rareItems = {
                "AncientCivilizationParts", "TechnologyPoint_Ancient",
                "DiamondArmor", "RocketLauncher", "MissileGuidedRifle",
                "PalSphere_Legend", "PalSphere_Ultra"
            };

            for (auto& rare : rareItems) {
                if (item.staticId.find(rare) != std::string::npos) {
                    rareItemCount += item.count;
                }
            }
        }

        // Flag if too many rare items for the playtime
        // Rough heuristic: shouldn't have more than ~2 rare items per hour of play
        float expectedMax = hoursPlayed * 2.0f + 5.0f;
        if (rareItemCount > static_cast<int>(expectedMax) && rareItemCount > 10) {
            flags.push_back(createFlag(ctx.player.uid,
                "Player has " + std::to_string(rareItemCount) +
                " rare items with only " + std::to_string(static_cast<int>(hoursPlayed)) +
                " hours played (expected max ~" + std::to_string(static_cast<int>(expectedMax)) + ")"));
        }

        return flags;
    }
};

// Rule: Tower boss pal detection
class TowerBossPalRule : public ScanRule {
public:
    TowerBossPalRule()
        : ScanRule("PAL_TOWER_BOSS", "Tower Boss Pal Captured",
                   ScanSeverity::Critical, "Player has captured a tower boss pal") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;

        static const std::vector<std::string> towerBosses = {
            "GYM_ThunderDragonMan", "GYM_LilyQueen", "GYM_FlameGoat",
            "GYM_IceKingPal", "GYM_ElecPanda"
        };

        for (auto& pal : ctx.pals) {
            for (auto& boss : towerBosses) {
                if (pal.characterId.find(boss) != std::string::npos || pal.isBoss) {
                    flags.push_back(createFlag(ctx.player.uid,
                        "Player has tower boss pal: " + pal.characterId));
                }
            }
        }

        return flags;
    }
};

} // namespace PSM

Notification System (Desktop + Sound)
cpp

Copy code
// src/ui/NotificationManager.h
#pragma once

#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <imgui.h>

namespace PSM {

struct Notification {
    std::string title;
    std::string message;
    ImVec4 color;
    float duration;
    float elapsed = 0.0f;
    float alpha = 1.0f;
    bool dismissed = false;
};

class NotificationManager {
public:
    NotificationManager();
    ~NotificationManager();

    void push(const std::string& title, const std::string& message,
              const ImVec4& color = ImVec4(0.4f, 0.65f, 0.95f, 1.0f),
              float duration = 5.0f);

    void pushSuccess(const std::string& title, const std::string& message);
    void pushWarning(const std::string& title, const std::string& message);
    void pushError(const std::string& title, const std::string& message);
    void pushInfo(const std::string& title, const std::string& message);

    void render();
    void update(float deltaTime);

    // Desktop notifications (platform-specific)
    void showDesktopNotification(const std::string& title, const std::string& message);

    // Sound alerts
    void playAlertSound();
    void setSoundEnabled(bool enabled);

private:
    std::deque<Notification> m_notifications;
    int m_maxVisible = 5;
    bool m_soundEnabled = true;

    // Animation constants
    float m_fadeInTime = 0.3f;
    float m_fadeOutTime = 0.5f;
    float m_slideDistance = 30.0f;
};

} // namespace PSM
