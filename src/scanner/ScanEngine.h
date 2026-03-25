#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include "scanner/ScanRule.h"
#include "models/ScanResult.h"

namespace PSM {

class LevelSavParser;
class PalDefenderAPI;
class PalDefenderRCON;
class Database;
class Config;
class EventBus;

class ScanEngine {
public:
    ScanEngine(LevelSavParser& pst, PalDefenderAPI& pdApi, PalDefenderRCON& pdRcon,
               Database& db, Config& config, EventBus& eventBus);
    ~ScanEngine();

    void registerRule(std::shared_ptr<ScanRule> rule);
    void registerDefaultRules();

    // Run scans
    std::vector<ScanFlag> scanPlayer(const std::string& uid);
    std::vector<ScanFlag> scanAllPlayers();

    // Scheduler
    void startScheduler();
    void stopScheduler();

    // Flag management
    std::vector<ScanFlag> getFlaggedPlayers();
    std::vector<PlayerScanReport> getReports();
    void dismissFlag(int flagId);
    void actionFlag(int flagId, const std::string& action);

    // Auto action
    void processAutoActions(const std::vector<ScanFlag>& flags);

    // Rule management
    const std::vector<std::shared_ptr<ScanRule>>& getRules() const;

private:
    ScanContext buildContext(const std::string& uid);
    void schedulerThread();

    LevelSavParser& m_pst;
    PalDefenderAPI& m_pdApi;
    PalDefenderRCON& m_pdRcon;
    Database& m_db;
    Config& m_config;
    EventBus& m_eventBus;

    std::vector<std::shared_ptr<ScanRule>> m_rules;
    std::atomic<bool> m_running{false};
    std::thread m_schedulerThread;
    mutable std::mutex m_mutex;
};

} // namespace PSM
