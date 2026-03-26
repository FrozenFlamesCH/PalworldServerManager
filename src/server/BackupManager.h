#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

namespace PSM {

class Config;
class Logger;
class Database;

struct BackupEntry {
    int id = 0;
    std::string path;
    int64_t sizeBytes = 0;
    int64_t timestamp = 0;
};

class BackupManager {
public:
    BackupManager(Config& config, Logger& logger, Database& database);
    ~BackupManager();

    bool createBackup();
    bool restoreBackup(const std::string& backupPath);
    void cleanOldBackups();

    void startScheduler();
    void stopScheduler();

    std::vector<BackupEntry> getBackupList();
    bool deleteBackup(int id);

private:
    void schedulerThread();
    std::string generateBackupPath();
    bool copyDirectory(const std::string& src, const std::string& dst);

    Config& m_config;
    Logger& m_logger;
    Database& m_database;

    std::atomic<bool> m_running{false};
    std::thread m_schedulerThread;
    std::mutex m_mutex;
};

} // namespace PSM
