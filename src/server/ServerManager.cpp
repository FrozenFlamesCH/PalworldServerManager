#include "server/ServerManager.h"
#include "net/VanillaAPI.h"
#include "net/PalDefenderRCON.h"
#include "server/ProcessMonitor.h"
#include "core/Config.h"
#include "core/EventBus.h"
#include <chrono>
#include <thread>
#include <algorithm>

namespace PSM {

ServerManager::ServerManager(VanillaAPI& vanilla, PalDefenderRCON& rcon,
                             ProcessMonitor& process, Config& config, EventBus& eventBus)
    : m_vanilla(vanilla), m_rcon(rcon), m_process(process), m_config(config), m_eventBus(eventBus) {

    m_autoRestart = config.getBool("server.autoRestartOnCrash", false);

    // Register for process state changes
    process.onStateChange([this](ServerProcessState oldState, ServerProcessState newState) {
        if (newState == ServerProcessState::Running) {
            m_eventBus.publishDeferred(EventBus::EVT_SERVER_STARTED);
            m_restartAttempts = 0;
        }
        else if (newState == ServerProcessState::Stopped) {
            m_eventBus.publishDeferred(EventBus::EVT_SERVER_STOPPED);
        }
        else if (newState == ServerProcessState::Crashed) {
            m_eventBus.publishDeferred(EventBus::EVT_SERVER_CRASHED);
            if (m_autoRestart) {
                autoRestartCheck();
            }
        }
    });
}

ServerManager::~ServerManager() {
    if (m_countdownThread.joinable()) m_countdownThread.join();
}

bool ServerManager::startServer() {
    std::string exePath = m_config.getString("server.exePath", "");
    std::string workDir = m_config.getString("server.workingDir", "");

    std::vector<std::string> args;
    auto argsJson = m_config.data().value("server", nlohmann::json::object()).value("launchArgs", nlohmann::json::array());
    for (auto& a : argsJson) {
        if (a.is_string()) args.push_back(a.get<std::string>());
    }

    return m_process.startProcess(workDir, args);
}

bool ServerManager::stopServer(bool graceful) {
    if (graceful) {
        int countdown = m_config.getInt("server.shutdownCountdown", 30);
        std::string message = m_config.getString("server.shutdownMessage", "Server shutting down!");

        if (countdown > 0) {
            gracefulShutdownWithCountdown();
            return true;
        }

        // Try API shutdown first
        m_vanilla.shutdown(0, message);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    if (m_process.isProcessRunning()) {
        return m_process.killProcess();
    }
    return true;
}

bool ServerManager::restartServer(bool graceful) {
    if (!stopServer(graceful)) return false;
    // Wait for process to fully stop
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return startServer();
}

void ServerManager::forceKill() {
    m_process.killProcess();
}

ServerInfo ServerManager::getServerInfo() {
    try {
        m_cachedInfo = m_vanilla.fetchServerInfo();
    }
    catch (...) {
        m_cachedInfo.online = false;
    }
    return m_cachedInfo;
}

ServerSettings ServerManager::getServerSettings() {
    try {
        m_cachedSettings = m_vanilla.fetchServerSettings();
    }
    catch (...) {}
    return m_cachedSettings;
}

std::vector<Player> ServerManager::getOnlinePlayers() {
    try {
        m_cachedPlayers = m_vanilla.fetchOnlinePlayers();
    }
    catch (...) {
        m_cachedPlayers.clear();
    }
    return m_cachedPlayers;
}

bool ServerManager::saveWorld() {
    return m_vanilla.saveWorld();
}

bool ServerManager::broadcastMessage(const std::string& message) {
    // Try both vanilla and PalDefender
    bool result = m_vanilla.broadcastMessage(message);
    if (!result) {
        auto resp = m_rcon.broadcast(message);
        result = !resp.empty();
    }
    return result;
}

void ServerManager::gracefulShutdownWithCountdown() {
    if (m_countdownThread.joinable()) {
        m_countdownThread.join();
    }

    int countdown = m_config.getInt("server.shutdownCountdown", 30);
    std::string message = m_config.getString("server.shutdownMessage", "Server shutting down in {seconds} seconds!");

    m_countdownThread = std::thread(&ServerManager::countdownThread, this, countdown, message);
}

void ServerManager::countdownThread(int seconds, const std::string& messageTemplate) {
    // Broadcast at key intervals
    std::vector<int> announceAt = {300, 120, 60, 30, 15, 10, 5, 4, 3, 2, 1};

    for (int remaining = seconds; remaining > 0; --remaining) {
        if (std::find(announceAt.begin(), announceAt.end(), remaining) != announceAt.end()) {
            std::string msg = messageTemplate;
            size_t pos = msg.find("{seconds}");
            if (pos != std::string::npos) {
                msg.replace(pos, 9, std::to_string(remaining));
            }
            broadcastMessage(msg);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Save and shutdown
    saveWorld();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    m_vanilla.shutdown(0, "Server shutting down now!");
    std::this_thread::sleep_for(std::chrono::seconds(5));

    if (m_process.isProcessRunning()) {
        m_process.killProcess();
    }
}

void ServerManager::enableAutoRestart(bool enable) {
    m_autoRestart = enable;
    m_config.setBool("server.autoRestartOnCrash", enable);
}

bool ServerManager::isAutoRestartEnabled() const {
    return m_autoRestart;
}

void ServerManager::autoRestartCheck() {
    int maxAttempts = m_config.getInt("server.maxRestartAttempts", 5);
    int delay = m_config.getInt("server.restartDelay", 10);

    if (m_restartAttempts >= maxAttempts) return;

    m_restartAttempts++;
    std::thread([this, delay]() {
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        startServer();
    }).detach();
}

std::vector<ServerManager::FPSDataPoint> ServerManager::getFPSHistory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_fpsHistory;
}

const ServerInfo& ServerManager::cachedInfo() const { return m_cachedInfo; }
const std::vector<Player>& ServerManager::cachedPlayers() const { return m_cachedPlayers; }

void ServerManager::refreshData() {
    auto info = getServerInfo();

    std::lock_guard<std::mutex> lock(m_mutex);
    FPSDataPoint point;
    point.fps = info.fps;
    point.frameTime = info.frameTime;
    point.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    m_fpsHistory.push_back(point);

    // Keep last 300 data points (5 minutes at 1/sec)
    if (m_fpsHistory.size() > 300) {
        m_fpsHistory.erase(m_fpsHistory.begin());
    }
}

} // namespace PSM
