#include "ui/UIManager.h"
#include "Application.h"
#include "core/Config.h"
#include "core/Logger.h"
#include "core/EventBus.h"
#include "core/Database.h"
#include "core/ThreadPool.h"
#include "net/VanillaAPI.h"
#include "net/PalDefenderAPI.h"
#include "net/PalDefenderRCON.h"
#include "pst/LevelSavParser.h"
#include "server/ServerManager.h"
#include "server/BackupManager.h"
#include "server/ProcessMonitor.h"
#include "scanner/ScanEngine.h"
#include "ui/Widgets.h"

#include <imgui.h>
#include <cstring>
#include <algorithm>

namespace PSM {

UIManager::UIManager(Application& app) : m_app(app) {
    m_themeManager.initialize();

    // Load saved theme
    std::string savedTheme = app.config().getString("ui.theme", "dark");
    m_themeManager.apply(savedTheme);

    // Initialize settings buffers from config
    strncpy(m_vanillaHost, app.config().getString("vanilla.host", "127.0.0.1").c_str(), sizeof(m_vanillaHost) - 1);
    m_vanillaPort = app.config().getInt("vanilla.port", 8212);
    strncpy(m_vanillaPassword, app.config().getString("vanilla.password", "").c_str(), sizeof(m_vanillaPassword) - 1);

    strncpy(m_pdHost, app.config().getString("paldefender.host", "127.0.0.1").c_str(), sizeof(m_pdHost) - 1);
    m_pdPort = app.config().getInt("paldefender.port", 8214);
    strncpy(m_pdApiKey, app.config().getString("paldefender.apiKey", "").c_str(), sizeof(m_pdApiKey) - 1);

    strncpy(m_rconHost, app.config().getString("rcon.host", "127.0.0.1").c_str(), sizeof(m_rconHost) - 1);
    m_rconPort = app.config().getInt("rcon.port", 25575);
    strncpy(m_rconPassword, app.config().getString("rcon.password", "").c_str(), sizeof(m_rconPassword) - 1);

    strncpy(m_pstPath, app.config().getString("pst.levelSavPath", "").c_str(), sizeof(m_pstPath) - 1);
    m_pstSyncInterval = app.config().getInt("pst.syncInterval", 60);

    strncpy(m_serverExePath, app.config().getString("server.exePath", "").c_str(), sizeof(m_serverExePath) - 1);
    strncpy(m_serverWorkDir, app.config().getString("server.workingDir", "").c_str(), sizeof(m_serverWorkDir) - 1);
}

UIManager::~UIManager() {}

void UIManager::render() {
    // Auto-refresh timer
    m_refreshTimer += ImGui::GetIO().DeltaTime;
    if (m_refreshTimer >= m_refreshInterval) {
        m_refreshTimer = 0;
        // Trigger background data refresh
        m_app.serverManager().refreshData();
    }

    // Create dockspace over entire window
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("##MainWindow", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    renderMenuBar();

    // Layout: Sidebar + Main content
    float statusBarHeight = 24.0f;
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    contentSize.y -= statusBarHeight;

    // Sidebar
    ImGui::BeginChild("##Sidebar", ImVec2(m_sidebarWidth, contentSize.y), true);
    renderSidebar();
    ImGui::EndChild();

    ImGui::SameLine();

    // Main content area
    ImGui::BeginChild("##MainContent", ImVec2(0, contentSize.y), true);
    renderMainContent();
    ImGui::EndChild();

    // Status bar
    renderStatusBar();

    ImGui::End();

    // Notifications
    renderNotifications();

    // Demo window (debug)
    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
}

void UIManager::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Config")) {
                m_app.config().save();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                m_app.requestShutdown();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Server")) {
            if (ImGui::MenuItem("Start Server")) {
                m_app.serverManager().startServer();
            }
            if (ImGui::MenuItem("Stop Server")) {
                m_showShutdownConfirm = true;
            }
            if (ImGui::MenuItem("Save World")) {
                m_app.serverManager().saveWorld();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Create Backup")) {
                m_app.backupManager().createBackup();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Demo Window", nullptr, &m_showDemoWindow);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("About");
            ImGui::EndMenu();
        }

        // Right-aligned server status
        auto& info = m_app.serverManager().cachedInfo();
        float statusWidth = 200;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - statusWidth);

        if (info.online) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.4f, 1.0f), "● ONLINE");
        } else {
            ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.25f, 1.0f), "● OFFLINE");
        }
        ImGui::SameLine();
        ImGui::Text("%d/%d", info.currentPlayers, info.maxPlayers);

        ImGui::EndMenuBar();
    }
}

void UIManager::renderSidebar() {
    ImGui::Spacing();

    struct PanelEntry {
        Panel panel;
        const char* label;
        const char* icon;
    };

    PanelEntry panels[] = {
        {Panel::Dashboard,      "Dashboard",        "📊"},
        {Panel::Players,        "Players",          "👥"},
        {Panel::Map,            "Map",              "🗺️"},
        {Panel::Inventory,      "Inventory",        "🎒"},
        {Panel::Guilds,         "Guilds",           "🏰"},
        {Panel::Communication,  "Communication",    "💬"},
        {Panel::Research,       "Research",         "🔬"},
        {Panel::PalSpawning,    "Pal Spawning",     "🐾"},
        {Panel::WorldControl,   "World Control",    "⏰"},
        {Panel::Backups,        "Backups",          "💾"},
        {Panel::Analytics,      "Analytics",        "📈"},
        {Panel::Security,       "Security",         "🔐"},
        {Panel::Scanner,        "Scanner",          "🆕"},
        {Panel::ServerSettings, "Server Config",    "⚙️"},
        {Panel::AppSettings,    "Settings",         "🔧"},
        {Panel::Themes,         "Themes",           "🎨"},
    };

    ImVec2 btnSize(m_sidebarWidth - 16, 32);
    for (auto& entry : panels) {
        if (Widgets::SidebarButton(entry.label, entry.icon,
                                    m_currentPanel == entry.panel, btnSize)) {
            m_currentPanel = entry.panel;
        }
    }
}

void UIManager::renderMainContent() {
    switch (m_currentPanel) {
        case Panel::Dashboard:      renderDashboard(); break;
        case Panel::Players:        renderPlayers(); break;
        case Panel::Map:            renderMap(); break;
        case Panel::Inventory:      renderInventory(); break;
        case Panel::Guilds:         renderGuilds(); break;
        case Panel::Communication:  renderCommunication(); break;
        case Panel::Research:       renderResearch(); break;
        case Panel::PalSpawning:    renderPalSpawning(); break;
        case Panel::WorldControl:   renderWorldControl(); break;
        case Panel::Backups:        renderBackups(); break;
        case Panel::Analytics:      renderAnalytics(); break;
        case Panel::Security:       renderSecurity(); break;
        case Panel::Scanner:        renderScanner(); break;
        case Panel::ServerSettings: renderServerSettings(); break;
        case Panel::AppSettings:    renderAppSettings(); break;
        case Panel::Themes:         renderThemes(); break;
        default: break;
    }
}

void UIManager::renderStatusBar() {
    ImGui::Separator();
    ImGui::BeginChild("##StatusBar", ImVec2(0, 24));

    auto state = m_app.processMonitor().getState();
    std::string stateStr = m_app.processMonitor().getStateString();

    ImVec4 stateColor;
    switch (state) {
        case ServerProcessState::Running: stateColor = ImVec4(0.3f, 0.8f, 0.4f, 1.0f); break;
        case ServerProcessState::Starting: stateColor = ImVec4(0.95f, 0.75f, 0.2f, 1.0f); break;
        case ServerProcessState::Crashed: stateColor = ImVec4(0.9f, 0.3f, 0.25f, 1.0f); break;
        default: stateColor = ImVec4(0.5f, 0.5f, 0.55f, 1.0f); break;
    }

    ImGui::TextColored(stateColor, "Server: %s", stateStr.c_str());
    ImGui::SameLine(200);

    auto& info = m_app.serverManager().cachedInfo();
    ImGui::Text("FPS: %.1f | Players: %d/%d | Day: %d",
        info.fps, info.currentPlayers, info.maxPlayers, info.inGameDay);

    ImGui::SameLine(ImGui::GetWindowWidth() - 180);
    ImGui::Text("PST Sync: %llds ago", (long long)(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() -
        m_app.levelSavParser().lastSyncTime()));

    ImGui::EndChild();
}

void UIManager::renderDashboard() {
    ImGui::Text("📊 Dashboard");
    ImGui::Separator();
    ImGui::Spacing();

    auto& info = m_app.serverManager().cachedInfo();
    auto state = m_app.processMonitor().getState();

    // Status row
    float cardWidth = (ImGui::GetContentRegionAvail().x - 40) / 5.0f;

    ImGui::Columns(5, "##DashStats", false);
    ImGui::SetColumnWidth(0, cardWidth);
    ImGui::SetColumnWidth(1, cardWidth);
    ImGui::SetColumnWidth(2, cardWidth);
    ImGui::SetColumnWidth(3, cardWidth);
    ImGui::SetColumnWidth(4, cardWidth);

    // Status
    {
        ImVec4 color = info.online ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.25f, 1.0f);
        Widgets::StatCard("Status", info.online ? "ONLINE" : "OFFLINE", nullptr, color);
    }
    ImGui::NextColumn();

    // Players
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d / %d", info.currentPlayers, info.maxPlayers);
        Widgets::StatCard("Players", buf, nullptr, ImVec4(0.4f, 0.65f, 0.95f, 1.0f));
    }
    ImGui::NextColumn();

    // FPS
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", info.fps);
        ImVec4 fpsColor = info.fps > 20 ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) :
                          (info.fps > 10 ? ImVec4(0.95f, 0.75f, 0.2f, 1.0f) :
                                          ImVec4(0.9f, 0.3f, 0.25f, 1.0f));
        Widgets::StatCard("FPS", buf, nullptr, fpsColor);
    }
    ImGui::NextColumn();

    // Uptime
    {
        int uptime = info.uptime;
        int hours = uptime / 3600;
        int mins = (uptime % 3600) / 60;
        char buf[32];
        snprintf(buf, sizeof(buf), "%dh %dm", hours, mins);
        Widgets::StatCard("Uptime", buf, nullptr, ImVec4(0.65f, 0.5f, 0.85f, 1.0f));
    }
    ImGui::NextColumn();

    // In-game day
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Day %d", info.inGameDay);
        Widgets::StatCard("Game Day", buf, nullptr, ImVec4(0.85f, 0.65f, 0.2f, 1.0f));
    }

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Spacing();

    // Server info
    ImGui::Text("Server: %s", info.name.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "v%s", info.version.c_str());

    if (!info.palDefenderVersion.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.65f, 0.95f, 1.0f), "| PalDefender v%s",
            info.palDefenderVersion.c_str());
    }

    ImGui::Text("Process: %s", m_app.processMonitor().getStateString().c_str());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // FPS Graph
    ImGui::Text("FPS History");
    auto fpsHistory = m_app.serverManager().getFPSHistory();
    if (!fpsHistory.empty()) {
        std::vector<float> fpsVals;
        for (auto& p : fpsHistory) fpsVals.push_back(p.fps);
        Widgets::FPSGraph(fpsVals.data(), static_cast<int>(fpsVals.size()), 0, 60,
            ImVec2(ImGui::GetContentRegionAvail().x, 120));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Quick actions
    ImGui::Text("Quick Actions");
    ImGui::Spacing();

    if (state == ServerProcessState::Stopped || state == ServerProcessState::Crashed) {
        if (ImGui::Button("▶ Start Server", ImVec2(150, 35))) {
            m_app.serverManager().startServer();
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("▶ Start Server", ImVec2(150, 35));
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if (state == ServerProcessState::Running) {
        if (ImGui::Button("⏹ Shutdown", ImVec2(150, 35))) {
            m_showShutdownConfirm = true;
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("⏹ Shutdown", ImVec2(150, 35));
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if (ImGui::Button("💾 Save World", ImVec2(150, 35))) {
        m_app.serverManager().saveWorld();
    }

    ImGui::SameLine();

    if (ImGui::Button("📢 Broadcast", ImVec2(150, 35))) {
        ImGui::OpenPopup("##BroadcastPopup");
    }

    // Broadcast popup
    if (ImGui::BeginPopup("##BroadcastPopup")) {
        ImGui::Text("Broadcast Message");
        ImGui::InputText("##broadcastMsg", m_broadcastBuf, sizeof(m_broadcastBuf));
        if (ImGui::Button("Send")) {
            m_app.serverManager().broadcastMessage(m_broadcastBuf);
            m_broadcastBuf[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Shutdown confirmation
    if (Widgets::ConfirmDialog("Shutdown Server",
        "Are you sure you want to shut down the server? All players will be disconnected.",
        &m_showShutdownConfirm)) {
        m_app.serverManager().stopServer(true);
    }
}

void UIManager::renderPlayers() {
    ImGui::Text("👥 Player Management");
    ImGui::Separator();
    ImGui::Spacing();

    // Search
    Widgets::SearchInput("##playerSearch", m_searchBuf, sizeof(m_searchBuf), "Search players...");
    ImGui::Spacing();

    // Player list
    auto players = m_app.serverManager().getOnlinePlayers();
    auto allPlayers = m_app.levelSavParser().getAllPlayers();

    // Tabs: Online | All | Banned
    if (ImGui::BeginTabBar("##PlayerTabs")) {
        if (ImGui::BeginTabItem("Online")) {
            if (ImGui::BeginTable("##OnlinePlayers", 6,
                ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
                ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {

                ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 50);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("UID", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 60);
                ImGui::TableSetupColumn("Suspicion", ImGuiTableColumnFlags_WidthFixed, 80);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 200);
                ImGui::TableHeadersRow();

                std::string filter(m_searchBuf);
                std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

                for (auto& player : players) {
                    std::string lowerName = player.name;
                    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                    if (!filter.empty() && lowerName.find(filter) == std::string::npos) continue;

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    Widgets::StatusBadge("ON", ImVec4(0.3f, 0.8f, 0.4f, 1.0f));

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", player.name.c_str());

                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "%s", player.uid.c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", player.stats.level);

                    ImGui::TableNextColumn();
                    int score = m_app.database().getPlayerSuspicionScore(player.uid);
                    if (score > 0) {
                        ImVec4 scoreColor = score > 50 ?
                            ImVec4(0.9f, 0.3f, 0.25f, 1.0f) : ImVec4(0.95f, 0.75f, 0.2f, 1.0f);
                        ImGui::TextColored(scoreColor, "%d", score);
                    } else {
                        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.4f, 1.0f), "Clean");
                    }

                    ImGui::TableNextColumn();
                    ImGui::PushID(player.uid.c_str());
                    if (ImGui::SmallButton("Kick")) {
                        m_confirmTargetUid = player.uid;
                        m_confirmTargetName = player.name;
                        m_showKickConfirm = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Ban")) {
                        m_confirmTargetUid = player.uid;
                        m_confirmTargetName = player.name;
                        m_showBanConfirm = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Msg")) {
                        strncpy(m_messageTargetUid, player.uid.c_str(), sizeof(m_messageTargetUid) - 1);
                        ImGui::OpenPopup("##SendMsg");
                    }
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("All Players")) {
            ImGui::Text("Total tracked players: %zu", allPlayers.size());
            // Similar table for all players
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Kick confirm
    if (m_showKickConfirm) {
        std::string msg = "Kick player " + m_confirmTargetName + "?";
        if (Widgets::ConfirmDialog("Kick Player", msg.c_str(), &m_showKickConfirm)) {
            m_app.vanillaAPI().kickPlayer(m_confirmTargetUid, "Kicked by admin");
        }
    }

    // Ban confirm
    if (m_showBanConfirm) {
        std::string msg = "Ban player " + m_confirmTargetName + "? This cannot be easily undone.";
        if (Widgets::ConfirmDialog("Ban Player", msg.c_str(), &m_showBanConfirm)) {
            m_app.palDefenderRCON().banPlayer(m_confirmTargetUid, "Banned by admin");
        }
    }
}

void UIManager::renderMap() {
    ImGui::Text("🗺️ Map");
    ImGui::Separator();
    ImGui::Spacing();

    // Layer toggles
    static bool showPlayers = true;
    static bool showBases = true;
    static bool showHeatmap = false;

    ImGui::Checkbox("Players", &showPlayers);
    ImGui::SameLine();
    ImGui::Checkbox("Bases", &showBases);
    ImGui::SameLine();
    ImGui::Checkbox("Activity Heatmap", &showHeatmap);
    ImGui::Spacing();

    // Map canvas
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    canvasSize.y -= 40; // Leave room for controls

    ImGui::BeginChild("##MapCanvas", canvasSize, true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Draw map background (dark)
    dl->AddRectFilled(canvasPos,
        ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
        IM_COL32(20, 25, 30, 255));

    // Draw grid
    float gridSpacing = 50.0f;
    for (float x = 0; x < canvasSize.x; x += gridSpacing) {
        dl->AddLine(ImVec2(canvasPos.x + x, canvasPos.y),
                    ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y),
                    IM_COL32(40, 45, 50, 255));
    }
    for (float y = 0; y < canvasSize.y; y += gridSpacing) {
        dl->AddLine(ImVec2(canvasPos.x, canvasPos.y + y),
                    ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y),
                    IM_COL32(40, 45, 50, 255));
    }

    // Draw players
    if (showPlayers) {
        auto players = m_app.serverManager().cachedPlayers();
        for (auto& p : players) {
            // Map world coords to screen coords (simplified)
            float sx = canvasPos.x + (p.position.x / 100.0f) + canvasSize.x * 0.5f;
            float sy = canvasPos.y + (p.position.y / 100.0f) + canvasSize.y * 0.5f;

            // Clamp to canvas
            sx = std::clamp(sx, canvasPos.x, canvasPos.x + canvasSize.x);
            sy = std::clamp(sy, canvasPos.y, canvasPos.y + canvasSize.y);

            dl->AddCircleFilled(ImVec2(sx, sy), 6.0f, IM_COL32(80, 180, 100, 255));
            dl->AddText(ImVec2(sx + 8, sy - 8), IM_COL32(200, 200, 200, 255), p.name.c_str());
        }
    }

    // Handle clicks for spawning / teleporting
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        float worldX = (mousePos.x - canvasPos.x - canvasSize.x * 0.5f) * 100.0f;
        float worldY = (mousePos.y - canvasPos.y - canvasSize.y * 0.5f) * 100.0f;
        // Could open context menu with spawn options here
    }

    ImGui::EndChild();
}

void UIManager::renderInventory() {
    ImGui::Text("🎒 Player Inventory & Pals");
    ImGui::Separator();
    ImGui::Spacing();

    static std::string selectedPlayer;

    // Player selector
    auto allPlayers = m_app.levelSavParser().getAllPlayers();
    if (ImGui::BeginCombo("Select Player", selectedPlayer.empty() ? "Choose..." : selectedPlayer.c_str())) {
        for (auto& p : allPlayers) {
            std::string label = p.name + " (" + p.uid + ")";
            if (ImGui::Selectable(label.c_str(), selectedPlayer == p.uid)) {
                selectedPlayer = p.uid;
            }
        }
        ImGui::EndCombo();
    }

    if (selectedPlayer.empty()) {
        ImGui::Text("Select a player to view their inventory and pals.");
        return;
    }

    ImGui::Spacing();

    if (ImGui::BeginTabBar("##InventoryTabs")) {
        if (ImGui::BeginTabItem("Inventory")) {
            auto items = m_app.levelSavParser().getPlayerInventory(selectedPlayer);

            if (ImGui::BeginTable("##ItemTable", 4,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Item");
                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Count");
                ImGui::TableSetupColumn("Container");
                ImGui::TableHeadersRow();

                for (auto& item : items) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("%s", item.name.c_str());
                    ImGui::TableNextColumn(); ImGui::Text("%s", item.staticId.c_str());
                    ImGui::TableNextColumn(); ImGui::Text("%d", item.count);
                    ImGui::TableNextColumn(); ImGui::Text("%s", item.container.c_str());
                }

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Pals")) {
            auto pals = m_app.levelSavParser().getPlayerPals(selectedPlayer);

            if (ImGui::BeginTable("##PalTable", 7,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Pal");
                ImGui::TableSetupColumn("Level");
                ImGui::TableSetupColumn("Rank");
                ImGui::TableSetupColumn("Gender");
                ImGui::TableSetupColumn("Shiny");
                ImGui::TableSetupColumn("Passives");
                ImGui::TableSetupColumn("Actions");
                ImGui::TableHeadersRow();

                for (auto& pal : pals) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (pal.isShiny) ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "✨ ");
                    else ImGui::Text("   ");
                    ImGui::SameLine();
                    ImGui::Text("%s", pal.characterId.c_str());

                    ImGui::TableNextColumn(); ImGui::Text("%d", pal.level);
                    ImGui::TableNextColumn(); ImGui::Text("%d", pal.rank);
                    ImGui::TableNextColumn(); ImGui::Text("%s", pal.gender == 0 ? "♂" : "♀");
                    ImGui::TableNextColumn();
                    if (pal.isShiny) Widgets::TextSuccess("Yes");
                    else ImGui::Text("No");

                    ImGui::TableNextColumn();
                    for (size_t i = 0; i < pal.passives.size(); ++i) {
                        if (i > 0) ImGui::SameLine();
                        ImGui::Text("%s", pal.passives[i].name.c_str());
                    }

                    ImGui::TableNextColumn();
                    ImGui::PushID(pal.instanceId.c_str());
                    if (ImGui::SmallButton("Details")) {
                        // Show pal details popup
                    }
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void UIManager::renderGuilds() {
    ImGui::Text("🏰 Guild Management");
    ImGui::Separator();
    ImGui::Spacing();

    auto guilds = m_app.palDefenderAPI().fetchAllGuilds();

    if (ImGui::Button("Export All Guilds (JSON)")) {
        m_app.palDefenderRCON().exportGuildsToJson();
    }

    ImGui::Spacing();

    for (auto& guild : guilds) {
        if (ImGui::TreeNode(guild.id.c_str(), "%s (%d members)", guild.name.c_str(), guild.memberCount)) {
            ImGui::Text("Leader: %s", guild.leaderName.c_str());
            ImGui::Text("Bases: %zu", guild.bases.size());

            if (ImGui::TreeNode("Members")) {
                for (auto& member : guild.members) {
                    ImVec4 color = member.online ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) : ImVec4(0.5f, 0.5f, 0.55f, 1.0f);
                    ImGui::TextColored(color, "%s %s %s",
                        member.online ? "●" : "○",
                        member.name.c_str(),
                        member.isLeader ? "(Leader)" : "");
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Bases")) {
                for (auto& base : guild.bases) {
                    ImGui::Text("Base: %s  Level: %d  Pals: %d  State: %s",
                        base.name.c_str(), base.level, base.palCount, base.state.c_str());
                    if (base.hasShinyPals) {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1, 0.85f, 0, 1), "✨ Has Shinies");
                    }
                }
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }
}

void UIManager::renderCommunication() {
    ImGui::Text("💬 Communication");
    ImGui::Separator();
    ImGui::Spacing();

    // Broadcast
    Widgets::SeparatorText("Broadcast to All");
    ImGui::InputText("##broadcastInput", m_broadcastBuf, sizeof(m_broadcastBuf));
    ImGui::SameLine();
    if (ImGui::Button("Send Broadcast")) {
        if (strlen(m_broadcastBuf) > 0) {
            m_app.serverManager().broadcastMessage(m_broadcastBuf);
            m_broadcastBuf[0] = '\0';
        }
    }

    ImGui::Spacing();

    // Alert
    static char alertBuf[512] = "";
    Widgets::SeparatorText("Alert Message");
    ImGui::InputText("##alertInput", alertBuf, sizeof(alertBuf));
    ImGui::SameLine();
    if (ImGui::Button("Send Alert")) {
        if (strlen(alertBuf) > 0) {
            m_app.palDefenderRCON().alert(alertBuf);
            alertBuf[0] = '\0';
        }
    }

    ImGui::Spacing();

    // Direct message
    Widgets::SeparatorText("Send to Player");
    ImGui::InputText("Player UID##msgUid", m_messageTargetUid, sizeof(m_messageTargetUid));
    ImGui::InputText("Message##directMsg", m_messageBuf, sizeof(m_messageBuf));
    if (ImGui::Button("Send Message")) {
        if (strlen(m_messageTargetUid) > 0 && strlen(m_messageBuf) > 0) {
            m_app.palDefenderRCON().sendMessage(m_messageTargetUid, m_messageBuf);
            m_messageBuf[0] = '\0';
        }
    }

    ImGui::Spacing();

    // Toggles
    Widgets::SeparatorText("Announcements");
    static bool loginAnnounce = true;
    static bool deathAnnounce = true;
    static bool punishAnnounce = true;

    Widgets::ToggleSwitch("Player Login/Logout Announcements", &loginAnnounce);
    Widgets::ToggleSwitch("Death Announcements", &deathAnnounce);
    Widgets::ToggleSwitch("Punishment Announcements", &punishAnnounce);
}

void UIManager::renderResearch() {
    ImGui::Text("🔬 Research / Tech Tree");
    ImGui::Separator();
    ImGui::Spacing();

    static char uid[128] = "";
    static char techId[128] = "";
    static int techPoints = 0;
    static int ancientPoints = 0;

    ImGui::InputText("Player UID", uid, sizeof(uid));
    ImGui::Spacing();

    Widgets::SeparatorText("Give Points");
    ImGui::InputInt("Tech Points", &techPoints);
    ImGui::SameLine();
    if (ImGui::Button("Give Tech Points")) {
        m_app.palDefenderRCON().giveTechPoints(uid, techPoints);
    }

    ImGui::InputInt("Ancient Tech Points", &ancientPoints);
    ImGui::SameLine();
    if (ImGui::Button("Give Ancient Points")) {
        m_app.palDefenderRCON().giveAncientTechPoints(uid, ancientPoints);
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Technology Management");

    ImGui::InputText("Tech ID", techId, sizeof(techId));
    if (ImGui::Button("Unlock Tech")) {
        m_app.palDefenderRCON().unlockTech(uid, techId);
    }
    ImGui::SameLine();
    if (ImGui::Button("Unlock ALL")) {
        m_app.palDefenderRCON().unlockAllTech(uid);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Tech")) {
        m_app.palDefenderRCON().removeTech(uid, techId);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove ALL")) {
        m_app.palDefenderRCON().removeAllTech(uid);
    }

    ImGui::Spacing();
    if (ImGui::Button("View Available Tech IDs")) {
        auto result = m_app.palDefenderRCON().getAvailableTechIds();
        // Display in popup
    }
    ImGui::SameLine();
    if (ImGui::Button("View Available Skin IDs")) {
        auto result = m_app.palDefenderRCON().getAvailableSkinIds();
    }
}

void UIManager::renderPalSpawning() {
    ImGui::Text("🐾 Pal Spawning & Summoning");
    ImGui::Separator();
    ImGui::Spacing();

    static char palId[128] = "";
    static char targetUid[128] = "";
    static char templatePath[512] = "";
    static int palLevel = 1;
    static float spawnX = 0, spawnY = 0, spawnZ = 0;

    ImGui::InputText("Pal ID", palId, sizeof(palId));
    ImGui::InputText("Target Player UID", targetUid, sizeof(targetUid));
    ImGui::SliderInt("Level", &palLevel, 1, 55);

    Widgets::SeparatorText("Give Pal to Player");
    if (ImGui::Button("Give Pal")) {
        m_app.palDefenderRCON().givePal(targetUid, palId, palLevel);
    }
    ImGui::SameLine();
    if (ImGui::Button("Give Pal Egg")) {
        m_app.palDefenderRCON().givePalEgg(targetUid, palId);
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Spawn at Coordinates");
    ImGui::InputFloat("X", &spawnX);
    ImGui::InputFloat("Y", &spawnY);
    ImGui::InputFloat("Z", &spawnZ);

    if (ImGui::Button("Spawn Pal at Coordinates")) {
        m_app.palDefenderRCON().spawnPal(palId, spawnX, spawnY, spawnZ);
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Template Spawn");
    ImGui::InputText("Template Path", templatePath, sizeof(templatePath));
    if (ImGui::Button("Spawn from Template")) {
        m_app.palDefenderRCON().spawnPalFromTemplate(templatePath, spawnX, spawnY, spawnZ);
    }
}

void UIManager::renderWorldControl() {
    ImGui::Text("⏰ World & Server Control");
    ImGui::Separator();
    ImGui::Spacing();

    // Time control
    Widgets::SeparatorText("Time Control");
    static int hour = 12;
    ImGui::SliderInt("Set Hour", &hour, 0, 23);
    if (ImGui::Button("Set Time")) {
        m_app.palDefenderRCON().setTime(hour);
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Day")) {
        m_app.palDefenderRCON().setTimeDay();
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Night")) {
        m_app.palDefenderRCON().setTimeNight();
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Server Control");

    if (ImGui::Button("💾 Save World", ImVec2(200, 35))) {
        m_app.serverManager().saveWorld();
    }

    ImGui::SameLine();
    if (ImGui::Button("⏹ Graceful Shutdown", ImVec2(200, 35))) {
        m_showShutdownConfirm = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("🔄 Reload PalDefender", ImVec2(200, 35))) {
        m_app.palDefenderRCON().reloadConfig();
    }

    if (Widgets::ConfirmDialog("Shutdown Server",
        "This will broadcast a countdown and then shut down the server.",
        &m_showShutdownConfirm)) {
        m_app.serverManager().gracefulShutdownWithCountdown();
    }
}

void UIManager::renderBackups() {
    ImGui::Text("💾 Backup & Storage");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Create Backup Now", ImVec2(200, 35))) {
        m_app.backupManager().createBackup();
    }

    ImGui::Spacing();
    auto backups = m_app.backupManager().getBackupList();

    if (ImGui::BeginTable("##BackupTable", 4,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Date");
        ImGui::TableSetupColumn("Path");
        ImGui::TableSetupColumn("sizeBytes");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        for (auto& backup : backups) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            auto time = static_cast<time_t>(backup.timestamp);
            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &time);
#else
            localtime_r(&time, &tm);
#endif
            char timeBuf[64];
            std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", &tm);
            ImGui::Text("%s", timeBuf);

            ImGui::TableNextColumn();
            ImGui::Text("%s", backup.path.c_str());

            ImGui::TableNextColumn();
            float sizeMB = backup.sizeBytes / (1024.0f * 1024.0f);
            ImGui::Text("%.1f MB", sizeMB);

            ImGui::TableNextColumn();
            ImGui::PushID(backup.id);
            if (ImGui::SmallButton("Restore")) {
                // Confirm and restore
                m_app.backupManager().restoreBackup(backup.path);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Delete")) {
                m_app.backupManager().deleteBackup(backup.id);
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void UIManager::renderAnalytics() {
    ImGui::Text("📈 Analytics & Logs");
    ImGui::Separator();
    ImGui::Spacing();

    static const char* categories[] = {
        "All", "death", "cheat", "rcon", "chat", "building",
        "capture", "craft", "tech", "oil_rig", "helicopter", "session"
    };
    static int selectedCategory = 0;

    ImGui::Combo("Log Category", &selectedCategory, categories, IM_ARRAYSIZE(categories));
    ImGui::Spacing();

    std::string catFilter = selectedCategory == 0 ? "" : categories[selectedCategory];
    auto logs = m_app.database().getLogs(catFilter, 200);

    if (ImGui::BeginTable("##LogTable", 4,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Category");
        ImGui::TableSetupColumn("Player");
        ImGui::TableSetupColumn("Message");
        ImGui::TableHeadersRow();

        for (auto& log : logs) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%s", log.value("timestamp", "").c_str());

            ImGui::TableNextColumn();
            std::string cat = log.value("category", "");
            ImVec4 catColor = ImVec4(0.5f, 0.5f, 0.55f, 1.0f);
            if (cat == "cheat") catColor = ImVec4(0.9f, 0.3f, 0.25f, 1.0f);
            else if (cat == "death") catColor = ImVec4(0.85f, 0.5f, 0.2f, 1.0f);
            ImGui::TextColored(catColor, "%s", cat.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", log.value("player_uid", "").c_str());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", log.value("message", "").c_str());
        }

        ImGui::EndTable();
    }
}

void UIManager::renderSecurity() {
    ImGui::Text("🔐 Security & Anti-Cheat");
    ImGui::Separator();
    ImGui::Spacing();

    // Anti-cheat toggles
    Widgets::SeparatorText("Anti-Cheat Toggles");

    static bool autoKickCheaters = false;
    static bool whitelistEnforcement = false;
    static bool illegalItemProtection = false;
    static bool illegalPalDetection = false;
    static bool towerBossBlock = false;
    static bool steamIdProtection = false;

    Widgets::ToggleSwitch("Auto-kick/ban cheaters", &autoKickCheaters);
    Widgets::ToggleSwitch("Whitelist enforcement", &whitelistEnforcement);
    Widgets::ToggleSwitch("Illegal item protection", &illegalItemProtection);
    Widgets::ToggleSwitch("Illegal pal stat detection", &illegalPalDetection);
    Widgets::ToggleSwitch("Tower boss capture block", &towerBossBlock);
    Widgets::ToggleSwitch("SteamID duplicate login protection", &steamIdProtection);

    ImGui::Spacing();
    Widgets::SeparatorText("IP Ban Management");

    static char ipBuf[64] = "";
    ImGui::InputText("IP Address", ipBuf, sizeof(ipBuf));
    if (ImGui::Button("Ban IP")) {
        m_app.palDefenderRCON().banIP(ipBuf);
        ipBuf[0] = '\0';
    }
    ImGui::SameLine();
    if (ImGui::Button("Unban IP")) {
        m_app.palDefenderRCON().unbanIP(ipBuf);
        ipBuf[0] = '\0';
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Cheat Detection Alerts");
    auto cheatLogs = m_app.database().getLogs("cheat", 50);
    for (auto& log : cheatLogs) {
        Widgets::TextError(log.value("message", "").c_str());
    }
}

void UIManager::renderScanner() {
    ImGui::Text("🆕 Custom Scan Rules");
    ImGui::Separator();
    ImGui::Spacing();

    // Controls
    if (ImGui::Button("Run Full Scan Now", ImVec2(200, 35))) {
        m_app.threadPool().enqueue([this] {
            m_app.scanEngine().scanAllPlayers();
        });
    }

    ImGui::Spacing();

    // Rules list
    Widgets::SeparatorText("Active Rules");
    auto& rules = m_app.scanEngine().getRules();
    for (auto& rule : rules) {
        bool enabled = rule->isEnabled();
        if (Widgets::ToggleSwitch(rule->name().c_str(), &enabled)) {
            rule->setEnabled(enabled);
        }
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Flagged Players");

    auto reports = m_app.scanEngine().getReports();
    std::sort(reports.begin(), reports.end(),
        [](const PlayerScanReport& a, const PlayerScanReport& b) {
            return a.totalScore > b.totalScore;
        });

    if (ImGui::BeginTable("##FlaggedPlayers", 5,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Player");
        ImGui::TableSetupColumn("Score");
        ImGui::TableSetupColumn("Flags");
        ImGui::TableSetupColumn("Last Flag");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        for (auto& report : reports) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%s", report.playerName.c_str());

            ImGui::TableNextColumn();
            ImVec4 scoreColor = report.totalScore > 50 ?
                ImVec4(0.9f, 0.3f, 0.25f, 1.0f) :
                ImVec4(0.95f, 0.75f, 0.2f, 1.0f);
            ImGui::TextColored(scoreColor, "%d", report.totalScore);

            ImGui::TableNextColumn();
            ImGui::Text("%zu", report.flags.size());

            ImGui::TableNextColumn();
            if (!report.flags.empty()) {
                ImGui::Text("%s", report.flags[0].description.c_str());
            }

            ImGui::TableNextColumn();
            ImGui::PushID(report.playerUid.c_str());
            if (ImGui::SmallButton("Review")) {
                // Open detail view
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Dismiss All")) {
                for (auto& flag : report.flags) {
                    m_app.scanEngine().dismissFlag(flag.id);
                }
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Kick")) {
                m_app.palDefenderRCON().kickPlayer(report.playerUid, "Suspicious activity");
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Ban")) {
                m_app.palDefenderRCON().banPlayer(report.playerUid, "Cheating detected");
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void UIManager::renderServerSettings() {
    ImGui::Text("⚙️ Server Settings Viewer");
    ImGui::Separator();
    ImGui::Spacing();

    auto settings = m_app.serverManager().getServerSettings();

    if (ImGui::BeginTable("##ServerSettings", 2,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Setting", ImGuiTableColumnFlags_WidthFixed, 300);
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        auto addRow = [](const char* name, const std::string& value) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%s", name);
            ImGui::TableNextColumn(); ImGui::Text("%s", value.c_str());
        };

        addRow("EXP Rate", std::to_string(settings.expRate));
        addRow("Capture Rate", std::to_string(settings.captureRate));
        addRow("Pal Spawn Rate", std::to_string(settings.palSpawnRate));
        addRow("Day Time Speed", std::to_string(settings.dayTimeSpeed));
        addRow("Night Time Speed", std::to_string(settings.nightTimeSpeed));
        addRow("Death Penalty", settings.deathPenalty);
        addRow("PvP Enabled", settings.isPvP ? "Yes" : "No");
        addRow("Hardcore Mode", settings.isHardcore ? "Yes" : "No");
        addRow("Friendly Fire", settings.friendlyFire ? "Yes" : "No");
        addRow("Base Camp Max", std::to_string(settings.baseCampMaxNum));
        addRow("Base Workers Max", std::to_string(settings.baseCampWorkerMaxNum));
        addRow("Hatching Speed", std::to_string(settings.hatchingSpeed));

        // Raw settings
        if (settings.raw.is_object()) {
            for (auto& [key, val] : settings.raw.items()) {
                std::string valStr;
                if (val.is_string()) valStr = val.get<std::string>();
                else valStr = val.dump();
                addRow(key.c_str(), valStr);
            }
        }

        ImGui::EndTable();
    }
}

void UIManager::renderAppSettings() {
    ImGui::Text("🔧 App Settings & Configuration");
    ImGui::Separator();

    if (ImGui::BeginTabBar("##SettingsTabs")) {

        // Connection Settings
        if (ImGui::BeginTabItem("Connection")) {
            Widgets::SeparatorText("Vanilla REST API");
            ImGui::InputText("Host##vanilla", m_vanillaHost, sizeof(m_vanillaHost));
            ImGui::InputInt("Port##vanilla", &m_vanillaPort);
            Widgets::PasswordInput("Password##vanilla", m_vanillaPassword, sizeof(m_vanillaPassword), &m_showVanillaPassword);
            if (ImGui::Button("Test Connection##vanilla")) {
                m_app.vanillaAPI().updateConnection(m_vanillaHost, m_vanillaPort, m_vanillaPassword);
                bool ok = m_app.vanillaAPI().testConnection();
                Widgets::ShowNotification("Test", ok ? "Connected!" : "Failed!",
                    ok ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.25f, 1.0f));
            }

            Widgets::SeparatorText("PalDefender REST API");
            ImGui::InputText("Host##pd", m_pdHost, sizeof(m_pdHost));
            ImGui::InputInt("Port##pd", &m_pdPort);
            Widgets::PasswordInput("API Key##pd", m_pdApiKey, sizeof(m_pdApiKey), &m_showPdApiKey);
            if (ImGui::Button("Test Connection##pd")) {
                m_app.palDefenderAPI().updateConnection(m_pdHost, m_pdPort, m_pdApiKey);
                bool ok = m_app.palDefenderAPI().testConnection();
                Widgets::ShowNotification("Test", ok ? "Connected!" : "Failed!",
                    ok ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.25f, 1.0f));
            }

            Widgets::SeparatorText("RCON");
            ImGui::InputText("Host##rcon", m_rconHost, sizeof(m_rconHost));
            ImGui::InputInt("Port##rcon", &m_rconPort);
            Widgets::PasswordInput("Password##rcon", m_rconPassword, sizeof(m_rconPassword), &m_showRconPassword);
            if (ImGui::Button("Test Connection##rcon")) {
                m_app.palDefenderRCON().updateConnection(m_rconHost, m_rconPort, m_rconPassword);
                bool ok = m_app.palDefenderRCON().testConnection();
                Widgets::ShowNotification("Test", ok ? "Connected!" : "Failed!",
                    ok ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.25f, 1.0f));
            }

            Widgets::SeparatorText("PST (Level.sav)");
            ImGui::InputText("Level.sav Path", m_pstPath, sizeof(m_pstPath));
            ImGui::InputInt("Sync Interval (sec)", &m_pstSyncInterval);
            if (ImGui::Button("Test Parse##pst")) {
                m_app.levelSavParser().setPath(m_pstPath);
                bool ok = m_app.levelSavParser().testParse();
                Widgets::ShowNotification("Test", ok ? "Parseable!" : "Failed!",
                    ok ? ImVec4(0.3f, 0.8f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.25f, 1.0f));
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Save All Connection Settings", ImVec2(250, 35))) {
                m_app.config().setString("vanilla.host", m_vanillaHost);
                m_app.config().setInt("vanilla.port", m_vanillaPort);
                m_app.config().setString("vanilla.password", m_vanillaPassword);
                m_app.config().setString("paldefender.host", m_pdHost);
                m_app.config().setInt("paldefender.port", m_pdPort);
                m_app.config().setString("paldefender.apiKey", m_pdApiKey);
                m_app.config().setString("rcon.host", m_rconHost);
                m_app.config().setInt("rcon.port", m_rconPort);
                m_app.config().setString("rcon.password", m_rconPassword);
                m_app.config().setString("pst.levelSavPath", m_pstPath);
                m_app.config().setInt("pst.syncInterval", m_pstSyncInterval);
                m_app.config().save();

                // Update connections
                m_app.vanillaAPI().updateConnection(m_vanillaHost, m_vanillaPort, m_vanillaPassword);
                m_app.palDefenderAPI().updateConnection(m_pdHost, m_pdPort, m_pdApiKey);
                m_app.palDefenderRCON().updateConnection(m_rconHost, m_rconPort, m_rconPassword);
                m_app.levelSavParser().setPath(m_pstPath);
                m_app.levelSavParser().setSyncInterval(m_pstSyncInterval);
            }

            ImGui::SameLine();
            if (ImGui::Button("Reset to Defaults")) {
                m_app.config().loadDefaults();
                // Reload buffers
            }

            ImGui::EndTabItem();
        }

        // Server Launch Settings
        if (ImGui::BeginTabItem("Server Launch")) {
            ImGui::InputText("Server EXE Path", m_serverExePath, sizeof(m_serverExePath));
            ImGui::InputText("Working Directory", m_serverWorkDir, sizeof(m_serverWorkDir));

            Widgets::SeparatorText("Launch Arguments");
            // Would iterate and allow add/remove of individual args

            Widgets::SeparatorText("Auto-management");
            static bool autoLaunch = m_app.config().getBool("server.autoLaunchOnStart", false);
            static bool autoRestart = m_app.config().getBool("server.autoRestartOnCrash", false);
            static int restartDelay = m_app.config().getInt("server.restartDelay", 10);
            static int maxRestarts = m_app.config().getInt("server.maxRestartAttempts", 5);
            static int shutdownCountdown = m_app.config().getInt("server.shutdownCountdown", 30);

            Widgets::ToggleSwitch("Auto-launch on app start", &autoLaunch);
            Widgets::ToggleSwitch("Auto-restart on crash", &autoRestart);
            ImGui::InputInt("Restart delay (seconds)", &restartDelay);
            ImGui::InputInt("Max restart attempts", &maxRestarts);
            ImGui::InputInt("Shutdown countdown (seconds)", &shutdownCountdown);

            if (ImGui::Button("Save Launch Settings", ImVec2(200, 35))) {
                m_app.config().setString("server.exePath", m_serverExePath);
                m_app.config().setString("server.workingDir", m_serverWorkDir);
                m_app.config().setBool("server.autoLaunchOnStart", autoLaunch);
                m_app.config().setBool("server.autoRestartOnCrash", autoRestart);
                m_app.config().setInt("server.restartDelay", restartDelay);
                m_app.config().setInt("server.maxRestartAttempts", maxRestarts);
                m_app.config().setInt("server.shutdownCountdown", shutdownCountdown);
                m_app.config().save();
                m_app.processMonitor().setExePath(m_serverExePath);
            }

            ImGui::EndTabItem();
        }

        // Backup Settings
        if (ImGui::BeginTabItem("Backups")) {
            static char backupPath[512];
            static int backupInterval = 60;
            static int retentionDays = 7;

            strncpy(backupPath, m_app.config().getString("backup.outputPath", "backups").c_str(),
                    sizeof(backupPath) - 1);
            backupInterval = m_app.config().getInt("backup.intervalMinutes", 60);
            retentionDays = m_app.config().getInt("backup.retentionDays", 7);

            ImGui::InputText("Backup Output Folder", backupPath, sizeof(backupPath));
            ImGui::InputInt("Backup Interval (minutes)", &backupInterval);
            ImGui::InputInt("Retention (days)", &retentionDays);

            if (ImGui::Button("Save Backup Settings")) {
                m_app.config().setString("backup.outputPath", backupPath);
                m_app.config().setInt("backup.intervalMinutes", backupInterval);
                m_app.config().setInt("backup.retentionDays", retentionDays);
                m_app.config().save();
            }

            ImGui::EndTabItem();
        }

        // Notifications
        if (ImGui::BeginTabItem("Notifications")) {
            static bool crashNotify = true;
            static bool banNotify = true;
            static bool cheatNotify = true;
            static bool soundOnCrash = true;

            Widgets::ToggleSwitch("Desktop notification on crash", &crashNotify);
            Widgets::ToggleSwitch("Desktop notification on ban", &banNotify);
            Widgets::ToggleSwitch("Desktop notification on cheat", &cheatNotify);
            Widgets::ToggleSwitch("Sound alert on crash", &soundOnCrash);

            ImGui::EndTabItem();
        }

        // Scan Settings
        if (ImGui::BeginTabItem("Scan")) {
            static int scanInterval = m_app.config().getInt("scan.intervalMinutes", 30);
            static bool autoAction = m_app.config().getBool("scan.autoAction", false);
            static int threshold = m_app.config().getInt("scan.suspicionThreshold", 50);

            ImGui::InputInt("Scan Interval (minutes)", &scanInterval);
            Widgets::ToggleSwitch("Auto-action on scan result", &autoAction);
            ImGui::InputInt("Suspicion Score Threshold", &threshold);

            if (ImGui::Button("Save Scan Settings")) {
                m_app.config().setInt("scan.intervalMinutes", scanInterval);
                m_app.config().setBool("scan.autoAction", autoAction);
                m_app.config().setInt("scan.suspicionThreshold", threshold);
                m_app.config().save();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void UIManager::renderThemes() {
    ImGui::Text("🎨 Themes");
    ImGui::Separator();
    ImGui::Spacing();

    auto& themes = m_themeManager.getThemes();
    std::string currentId = m_themeManager.getCurrentThemeId();

    float itemWidth = 250.0f;
    float availWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(availWidth / itemWidth));

    ImGui::Columns(columns, "##ThemeGrid", false);

    for (auto& theme : themes) {
        bool isSelected = (theme.id == currentId);

        ImGui::PushID(theme.id.c_str());

        // Theme preview card
        ImVec2 cardPos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float cardWidth = ImGui::GetColumnWidth() - 10;
        float cardHeight = 100;

        // Background
        ImVec4 bg = isSelected ?
            ImVec4(theme.colors.accent.x, theme.colors.accent.y, theme.colors.accent.z, 0.15f) :
            ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
        dl->AddRectFilled(cardPos, ImVec2(cardPos.x + cardWidth, cardPos.y + cardHeight),
            ImGui::ColorConvertFloat4ToU32(bg), 6.0f);

        // Border if selected
        if (isSelected) {
            dl->AddRect(cardPos, ImVec2(cardPos.x + cardWidth, cardPos.y + cardHeight),
                ImGui::ColorConvertFloat4ToU32(theme.colors.accent), 6.0f, 0, 2.0f);
        }

        // Color swatches
        float swatchY = cardPos.y + 10;
        float swatchX = cardPos.x + 10;
        float swatchSize = 18;
        float swatchGap = 4;

        ImVec4 swatches[] = {
            theme.colors.windowBg, theme.colors.accent, theme.colors.button,
            theme.colors.text, theme.colors.success, theme.colors.error
        };

        for (auto& swatch : swatches) {
            dl->AddRectFilled(
                ImVec2(swatchX, swatchY),
                ImVec2(swatchX + swatchSize, swatchY + swatchSize),
                ImGui::ColorConvertFloat4ToU32(swatch), 3.0f);
            swatchX += swatchSize + swatchGap;
        }

        // Theme name
        std::string displayName = theme.emoji + " " + theme.name;
        dl->AddText(ImVec2(cardPos.x + 10, cardPos.y + 38),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.9f, 0.9f, 0.92f, 1.0f)),
            displayName.c_str());

        // Description
        dl->AddText(ImVec2(cardPos.x + 10, cardPos.y + 58),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.6f, 0.62f, 1.0f)),
            theme.description.c_str());

        // Apply button area
        if (ImGui::InvisibleButton("##apply", ImVec2(cardWidth, cardHeight))) {
            m_themeManager.apply(theme.id);
            m_app.config().setString("ui.theme", theme.id);
            m_app.config().save();
        }

        if (isSelected) {
            dl->AddText(ImVec2(cardPos.x + cardWidth - 60, cardPos.y + 78),
                ImGui::ColorConvertFloat4ToU32(theme.colors.accent), "Active");
        }

        ImGui::PopID();
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
}

void UIManager::renderNotifications() {
    // Render floating notifications (would track active notifications)
    // This is a simplified placeholder
}

} // namespace PSM
