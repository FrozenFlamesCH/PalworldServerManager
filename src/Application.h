#pragma once

#include <memory>
#include <atomic>
#include <string>

struct GLFWwindow;

namespace PSM {
    class Config;
    class Database;
    class Logger;
    class ThreadPool;
    class EventBus;
    class HttpClient;
    class RconClient;
    class VanillaAPI;
    class PalDefenderAPI;
    class PalDefenderRCON;
    class LevelSavParser;
    class ServerManager;
    class BackupManager;
    class ProcessMonitor;
    class ScanEngine;
    class UIManager;
    class RestServer;
}

class Application {
public:
    Application();
    ~Application();

    bool initialize(int argc, char* argv[]);
    void run();
    void shutdown();
    void requestShutdown();

    // Accessors for subsystems
    PSM::Config& config();
    PSM::Database& database();
    PSM::Logger& logger();
    PSM::ThreadPool& threadPool();
    PSM::EventBus& eventBus();
    PSM::VanillaAPI& vanillaAPI();
    PSM::PalDefenderAPI& palDefenderAPI();
    PSM::PalDefenderRCON& palDefenderRCON();
    PSM::LevelSavParser& levelSavParser();
    PSM::ServerManager& serverManager();
    PSM::BackupManager& backupManager();
    PSM::ProcessMonitor& processMonitor();
    PSM::ScanEngine& scanEngine();
    PSM::UIManager& uiManager();

    static Application& instance();

private:
    bool initWindow();
    bool initSubsystems();

    static Application* s_instance;

    GLFWwindow* m_window = nullptr;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_shutdownRequested{false};

    std::unique_ptr<PSM::Config> m_config;
    std::unique_ptr<PSM::Database> m_database;
    std::unique_ptr<PSM::Logger> m_logger;
    std::unique_ptr<PSM::ThreadPool> m_threadPool;
    std::unique_ptr<PSM::EventBus> m_eventBus;
    std::unique_ptr<PSM::HttpClient> m_httpClient;
    std::unique_ptr<PSM::RconClient> m_rconClient;
    std::unique_ptr<PSM::VanillaAPI> m_vanillaAPI;
    std::unique_ptr<PSM::PalDefenderAPI> m_palDefenderAPI;
    std::unique_ptr<PSM::PalDefenderRCON> m_palDefenderRCON;
    std::unique_ptr<PSM::LevelSavParser> m_levelSavParser;
    std::unique_ptr<PSM::ServerManager> m_serverManager;
    std::unique_ptr<PSM::BackupManager> m_backupManager;
    std::unique_ptr<PSM::ProcessMonitor> m_processMonitor;
    std::unique_ptr<PSM::ScanEngine> m_scanEngine;
    std::unique_ptr<PSM::UIManager> m_uiManager;
    std::unique_ptr<PSM::RestServer> m_restServer;
};