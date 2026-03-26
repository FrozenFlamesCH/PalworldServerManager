// DashboardPanel.cpp
#include "ui/panels/DashboardPanel.h"
#include "Application.h"
#include "core/Config.h"
#include "core/EventBus.h"
#include "core/Database.h"
#include "server/ServerManager.h"
#include "server/ProcessMonitor.h"
#include "scanner/ScanEngine.h"
#include "models/ScanResult.h"
#include "ui/Widgets.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <deque>
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
        dl->AddText(ImVec2(x + 10, y + 8),
            IM_COL32(160, 160, 165, 255), stats[i].label);

        // Value
        ImVec2 valSize = ImGui::CalcTextSize(stats[i].value.c_str());
        dl->AddText(
            ImVec2(x + (cardWidth - valSize.x) * 0.5f, y + cardHeight * 0.45f),
            ImGui::ColorConvertFloat4ToU32(stats[i].color),
            stats[i].value.c_str());
    }

    ImGui::Dummy(ImVec2(totalWidth, cardHeight + 5));
}

void DashboardPanel::renderFPSGraph() {
    if (m_fpsHistoryCount == 0) return;

    ImGui::Text("Server FPS");
    ImGui::PlotLines("##fps", m_fpsHistory, FPS_HISTORY_SIZE, m_fpsHistoryOffset,
        nullptr, 0.0f, 60.0f, ImVec2(ImGui::GetContentRegionAvail().x, 60));
}

void DashboardPanel::renderQuickActions() {
    ImGui::Text("Quick Actions");
    ImGui::Separator();

    if (ImGui::Button("💾 Save World", ImVec2(150, 35)))
        m_app.serverManager().saveWorld();
    ImGui::SameLine();
    if (ImGui::Button("📢 Broadcast", ImVec2(150, 35)))
        ImGui::OpenPopup("##QuickBroadcast");
    ImGui::SameLine();
    if (ImGui::Button("🔄 Restart", ImVec2(150, 35)))
        m_shutdownConfirm = true;

    if (Widgets::ConfirmDialog("Confirm Restart",
        "Broadcast countdown then restart server?", &m_shutdownConfirm))
        m_app.serverManager().gracefulShutdownWithCountdown();

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
