#pragma once

#include <vector>
#include <string>
#include <chrono>

class Application;

namespace PSM {

class DashboardPanel {
public:
    explicit DashboardPanel(Application& app);
    ~DashboardPanel();

    void render();
    void update(float deltaTime);

private:
    void renderServerStatus();
    void renderQuickStats();
    void renderFPSGraph();
    void renderQuickActions();
    void renderRecentActivity();
    void renderAlertBanner();

    Application& m_app;

    // FPS history ring buffer
    static constexpr int FPS_HISTORY_SIZE = 600;
    float m_fpsHistory[FPS_HISTORY_SIZE] = {};
    float m_frameTimeHistory[FPS_HISTORY_SIZE] = {};
    int m_fpsHistoryOffset = 0;
    int m_fpsHistoryCount = 0;

    // Auto-refresh
    float m_refreshTimer = 0.0f;
    float m_refreshInterval = 1.0f; // 1 second

    // Recent events
    struct ActivityEntry {
        std::string message;
        std::string category;
        std::chrono::system_clock::time_point timestamp;
        bool isAlert = false;
    };
    std::vector<ActivityEntry> m_recentActivity;
    static constexpr int MAX_RECENT = 20;

    // Alert state
    bool m_hasActiveAlerts = false;
    std::string m_alertMessage;
};

} // namespace PSM
