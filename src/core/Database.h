#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <optional>
#include <sqlite3.h>
#include <nlohmann/json.hpp>

namespace PSM {

class Database {
public:
    Database();
    ~Database();

    bool initialize(const std::string& dbPath);
    void close();

    // Generic query
    bool execute(const std::string& sql);
    bool execute(const std::string& sql, const std::vector<std::string>& params);

    using Row = std::vector<std::pair<std::string, std::string>>;
    std::vector<Row> query(const std::string& sql, const std::vector<std::string>& params = {});

    // Player session tracking
    void recordPlayerJoin(const std::string& uid, const std::string& name,
                          const std::string& ip, int64_t timestamp);
    void recordPlayerLeave(const std::string& uid, int64_t timestamp);
    nlohmann::json getPlayerSessions(const std::string& uid, int limit = 50);
    nlohmann::json getAllPlayerStats();

    // Scan results
    void recordScanResult(const std::string& playerUid, const std::string& ruleId,
                          const std::string& severity, const std::string& description,
                          int scoreImpact, int64_t timestamp);
    nlohmann::json getScanHistory(const std::string& playerUid = "", int limit = 100);
    int getPlayerSuspicionScore(const std::string& playerUid);
    void dismissScanFlag(int flagId);
    void setFlagActioned(int flagId, const std::string& action);

    // Backup tracking
    void recordBackup(const std::string& path, int64_t sizeBytes, int64_t timestamp);
    nlohmann::json getBackupHistory(int limit = 50);
    void deleteBackupRecord(int id);

    // Log entries
    void recordLog(const std::string& category, const std::string& message,
                   const std::string& playerUid = "", int64_t timestamp = 0);
    nlohmann::json getLogs(const std::string& category = "", int limit = 100);

    // Heatmap data
    void recordPlayerPosition(const std::string& uid, float x, float y, float z, int64_t timestamp);
    nlohmann::json getPositionHeatmapData(int64_t startTime = 0, int64_t endTime = 0);

    // Whitelisted players for scans
    void addScanWhitelist(const std::string& uid);
    void removeScanWhitelist(const std::string& uid);
    bool isScanWhitelisted(const std::string& uid);
    std::vector<std::string> getScanWhitelist();

private:
    bool createTables();

    sqlite3* m_db = nullptr;
    mutable std::mutex m_mutex;
};

} // namespace PSM