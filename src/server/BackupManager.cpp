#include "server/BackupManager.h"
#include "core/Config.h"
#include "core/Logger.h"
#include "core/Database.h"
#include <filesystem>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace PSM {

BackupManager::BackupManager(Config& config, Logger& logger, Database& database)
    : m_config(config), m_logger(logger), m_database(database) {}

BackupManager::~BackupManager() {
    stopScheduler();
}

bool BackupManager::createBackup() {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string savePath = m_config.getString("pst.levelSavPath", "");
    if (savePath.empty()) {
        m_logger.warn("No Level.sav path configured for backup");
        return false;
    }

    std::string backupDir = generateBackupPath();
    std::filesystem::create_directories(backupDir);

    try {
        // Copy the save directory
        std::filesystem::path savDir = std::filesystem::path(savePath).parent_path();

        for (auto& entry : std::filesystem::recursive_directory_iterator(savDir)) {
            auto relativePath = std::filesystem::relative(entry.path(), savDir);
            auto destPath = std::filesystem::path(backupDir) / relativePath;

            if (entry.is_directory()) {
                std::filesystem::create_directories(destPath);
            } else {
                std::filesystem::copy_file(entry.path(), destPath,
                    std::filesystem::copy_options::overwrite_existing);
            }
        }

        // Calculate size
        int64_t totalSize = 0;
        for (auto& entry : std::filesystem::recursive_directory_iterator(backupDir)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
            }
        }

        int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        m_database.recordBackup(backupDir, totalSize, now);
        m_logger.info("Backup created: " + backupDir);
        return true;
    }
    catch (const std::exception& e) {
        m_logger.error("Backup failed: " + std::string(e.what()));
        return false;
    }
}

bool BackupManager::restoreBackup(const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string savePath = m_config.getString("pst.levelSavPath", "");
    if (savePath.empty()) return false;

    std::filesystem::path savDir = std::filesystem::path(savePath).parent_path();

    try {
        // Copy backup over save directory
        for (auto& entry : std::filesystem::recursive_directory_iterator(backupPath)) {
            auto relativePath = std::filesystem::relative(entry.path(), backupPath);
            auto destPath = savDir / relativePath;

            if (entry.is_directory()) {
                std::filesystem::create_directories(destPath);
            } else {
                std::filesystem::copy_file(entry.path(), destPath,
                    std::filesystem::copy_options::overwrite_existing);
            }
        }

        m_logger.info("Backup restored from: " + backupPath);
        return true;
    }
    catch (const std::exception& e) {
        m_logger.error("Restore failed: " + std::string(e.what()));
        return false;
    }
}

void BackupManager::cleanOldBackups() {
    int retentionDays = m_config.getInt("backup.retentionDays", 7);
    int64_t cutoff = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - (retentionDays * 86400);

    auto backups = getBackupList();
    for (auto& backup : backups) {
        if (backup.timestamp < cutoff) {
            try {
                std::filesystem::remove_all(backup.path);
                m_database.deleteBackupRecord(backup.id);
                m_logger.info("Cleaned old backup: " + backup.path);
            }
            catch (...) {}
        }
    }
}

std::string BackupManager::generateBackupPath() {
    std::string outputDir = m_config.getString("backup.outputPath", "backups");
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream ss;
    ss << outputDir << "/backup_" << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return ss.str();
}

void BackupManager::startScheduler() {
    if (m_running) return;
    m_running = true;
    m_schedulerThread = std::thread(&BackupManager::schedulerThread, this);
}

void BackupManager::stopScheduler() {
    m_running = false;
    if (m_schedulerThread.joinable()) {
        m_schedulerThread.join();
    }
}

void BackupManager::schedulerThread() {
    while (m_running) {
        int intervalMinutes = m_config.getInt("backup.intervalMinutes", 60);

        for (int i = 0; i < intervalMinutes * 60 * 10 && m_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (m_running) {
            createBackup();
            cleanOldBackups();
        }
    }
}

std::vector<BackupEntry> BackupManager::getBackupList() {
    auto rows = m_database.getBackupHistory(100);
    std::vector<BackupEntry> entries;
    for (auto& row : rows) {
        BackupEntry e;
        for (auto& [k, v] : row) {
            if (k == "id") e.id = std::stoi(v);
            else if (k == "path") e.path = v;
            else if (k == "size_bytes") e.sizeBytes = std::stoll(v);
            else if (k == "timestamp") e.timestamp = std::stoll(v);
        }
        entries.push_back(e);
    }
    return entries;
}

bool BackupManager::deleteBackup(int id) {
    auto entries = getBackupList();
    for (auto& e : entries) {
        if (e.id == id) {
            try { std::filesystem::remove_all(e.path); } catch (...) {}
            m_database.deleteBackupRecord(id);
            return true;
        }
    }
    return false;
}

} // namespace PSM
