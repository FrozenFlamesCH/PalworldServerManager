#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include "models/ServerInfo.h"
#include "models/Player.h"

namespace PSM {

class VanillaAPI;
class PalDefenderRCON;
class ProcessMonitor;
class Config;
class EventBus;

class ServerManager {
public:
    ServerManager(VanillaAPI& vanilla, PalDefenderRCON& rcon,
                  ProcessMonitor& process, Config& config, EventBus& eventBus);
    ~ServerManager();

    // Server control
    bool startServer();
    bool stopServer(bool graceful = true);
    bool restartServer(bool graceful = true);
    void forceKill();

    // Status
    ServerInfo getServerInfo();
    ServerSettings getServerSettings();
    std::vector<Player> getOnlinePlayers();

    // Quick actions
    bool saveWorld();
    bool broadcastMessage(const std::string& message);
    void gracefulShutdownWithCountdown();

    // Auto restart
    void enableAutoRestart(bool enable);
    bool isAutoRestartEnabled() const;

    // FPS tracking
    struct FPSDataPoint {
        float fps;
        float frameTime;
        int64_t timestamp;
    };
    std::vector<FPSDataPoint> getFPSHistory() const;

    // Cached data
    const ServerInfo& cachedInfo() const;
    const std::vector<Player>& cachedPlayers() const;

    void refreshData();

private:
    void autoRestartCheck();
    void countdownThread(int seconds, const std::string& messageTemplate);

    VanillaAPI& m_vanilla;
    PalDefenderRCON& m_rcon;
    ProcessMonitor& m_process;
    Config& m_config;
    EventBus& m_eventBus;

    mutable std::mutex m_mutex;
    ServerInfo m_cachedInfo;
    ServerSettings m_cachedSettings;
    std::vector<Player> m_cachedPlayers;
    std::vector<FPSDataPoint> m_fpsHistory;

    std::atomic<bool> m_autoRestart{false};
    int m_restartAttempts = 0;
    std::thread m_countdownThread;
};

} // namespace PSM
