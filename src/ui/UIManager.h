#pragma once

#include <memory>
#include <string>
#include <vector>
#include "ui/Theme.h"

class Application;

namespace PSM {

class UIManager {
public:
    explicit UIManager(Application& app);
    ~UIManager();

    void render();

    ThemeManager& themeManager() { return m_themeManager; }

private:
    void renderMenuBar();
    void renderSidebar();
    void renderMainContent();
    void renderStatusBar();
    void renderNotifications();

    void renderDashboard();
    void renderPlayers();
    void renderMap();
    void renderInventory();
    void renderGuilds();
    void renderCommunication();
    void renderResearch();
    void renderPalSpawning();
    void renderWorldControl();
    void renderBackups();
    void renderAnalytics();
    void renderSecurity();
    void renderScanner();
    void renderServerSettings();
    void renderAppSettings();
    void renderThemes();

    Application& m_app;
    ThemeManager m_themeManager;

    enum class Panel {
        Dashboard,
        Players,
        Map,
        Inventory,
        Guilds,
        Communication,
        Research,
        PalSpawning,
        WorldControl,
        Backups,
        Analytics,
        Security,
        Scanner,
        ServerSettings,
        AppSettings,
        Themes,
        COUNT
    };

    Panel m_currentPanel = Panel::Dashboard;
    float m_sidebarWidth = 200.0f;
    float m_refreshTimer = 0.0f;
    float m_refreshInterval = 2.0f;
    bool m_showDemoWindow = false;

    // Panel-specific state
    char m_searchBuf[256] = "";
    char m_broadcastBuf[512] = "";
    char m_messageBuf[512] = "";
    char m_messageTargetUid[128] = "";

    // Dashboard data cache
    float m_fpsValues[300] = {};
    int m_fpsValueCount = 0;

    // Settings buffers
    char m_vanillaHost[256] = "";
    int m_vanillaPort = 8212;
    char m_vanillaPassword[256] = "";
    bool m_showVanillaPassword = false;

    char m_pdHost[256] = "";
    int m_pdPort = 8214;
    char m_pdApiKey[256] = "";
    bool m_showPdApiKey = false;

    char m_rconHost[256] = "";
    int m_rconPort = 25575;
    char m_rconPassword[256] = "";
    bool m_showRconPassword = false;

    char m_pstPath[1024] = "";
    int m_pstSyncInterval = 60;

    char m_serverExePath[1024] = "";
    char m_serverWorkDir[1024] = "";

    // Confirmation dialogs
    bool m_showKickConfirm = false;
    bool m_showBanConfirm = false;
    bool m_showShutdownConfirm = false;
    bool m_showDestroyBaseConfirm = false;
    std::string m_confirmTargetUid;
    std::string m_confirmTargetName;
};

} // namespace PSM
