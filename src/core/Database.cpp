#include "core/Database.h"
#include <iostream>
#include <chrono>

namespace PSM {

Database::Database() {}

Database::~Database() {
    close();
}

bool Database::initialize(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    int rc = sqlite3_open(dbPath.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    // Enable WAL mode for better concurrent access
    sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);

    return createTables();
}

void Database::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool Database::createTables() {
    const char* schema = R"SQL(
        CREATE TABLE IF NOT EXISTS player_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            uid TEXT NOT NULL,
            name TEXT,
            ip TEXT,
            join_time INTEGER NOT NULL,
            leave_time INTEGER DEFAULT 0,
            duration_seconds INTEGER DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS scan_flags (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            player_uid TEXT NOT NULL,
            rule_id TEXT NOT NULL,
            severity TEXT NOT NULL,
            description TEXT,
            score_impact INTEGER DEFAULT 0,
            timestamp INTEGER NOT NULL,
            dismissed INTEGER DEFAULT 0,
            action_taken TEXT DEFAULT ''
        );

        CREATE TABLE IF NOT EXISTS backups (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT NOT NULL,
            size_bytes INTEGER DEFAULT 0,
            timestamp INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            category TEXT NOT NULL,
            message TEXT NOT NULL,
            player_uid TEXT DEFAULT '',
            timestamp INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS player_positions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            uid TEXT NOT NULL,
            x REAL NOT NULL,
            y REAL NOT NULL,
            z REAL NOT NULL,
            timestamp INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS scan_whitelist (
            uid TEXT PRIMARY KEY
        );

        CREATE INDEX IF NOT EXISTS idx_sessions_uid ON player_sessions(uid);
        CREATE INDEX IF NOT EXISTS idx_scan_flags_uid ON scan_flags(player_uid);
        CREATE INDEX IF NOT EXISTS idx_logs_category ON logs(category);
        CREATE INDEX IF NOT EXISTS idx_positions_uid ON player_positions(uid);
        CREATE INDEX IF NOT EXISTS idx_positions_time ON player_positions(timestamp);
    )SQL";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, schema, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating tables: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::execute(const std::string& sql) {
    return execute(sql, {});
}

bool Database::execute(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db) return false;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::vector<Database::Row> Database::query(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Row> results;
    if (!m_db) return results;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return results;

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    int cols = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Row row;
        for (int c = 0; c < cols; ++c) {
            const char* name = sqlite3_column_name(stmt, c);
            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, c));
            row.emplace_back(name ? name : "", val ? val : "");
        }
        results.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
    return results;
}

void Database::recordPlayerJoin(const std::string& uid, const std::string& name,
                                const std::string& ip, int64_t timestamp) {
    execute("INSERT INTO player_sessions (uid, name, ip, join_time) VALUES (?, ?, ?, ?)",
        {uid, name, ip, std::to_string(timestamp)});
}

void Database::recordPlayerLeave(const std::string& uid, int64_t timestamp) {
    execute(
        "UPDATE player_sessions SET leave_time = ?, duration_seconds = ? - join_time "
        "WHERE uid = ? AND leave_time = 0 ORDER BY id DESC LIMIT 1",
        {std::to_string(timestamp), std::to_string(timestamp), uid});
}

nlohmann::json Database::getPlayerSessions(const std::string& uid, int limit) {
    auto rows = query(
        "SELECT * FROM player_sessions WHERE uid = ? ORDER BY join_time DESC LIMIT ?",
        {uid, std::to_string(limit)});

    nlohmann::json result = nlohmann::json::array();
    for (auto& row : rows) {
        nlohmann::json obj;
        for (auto& [k, v] : row) obj[k] = v;
        result.push_back(obj);
    }
    return result;
}

nlohmann::json Database::getAllPlayerStats() {
    auto rows = query(
        "SELECT uid, name, COUNT(*) as sessions, SUM(duration_seconds) as total_time, "
        "MAX(join_time) as last_seen FROM player_sessions GROUP BY uid ORDER BY last_seen DESC", {});

    nlohmann::json result = nlohmann::json::array();
    for (auto& row : rows) {
        nlohmann::json obj;
        for (auto& [k, v] : row) obj[k] = v;
        result.push_back(obj);
    }
    return result;
}

void Database::recordScanResult(const std::string& playerUid, const std::string& ruleId,
                                const std::string& severity, const std::string& description,
                                int scoreImpact, int64_t timestamp) {
    execute(
        "INSERT INTO scan_flags (player_uid, rule_id, severity, description, score_impact, timestamp) "
        "VALUES (?, ?, ?, ?, ?, ?)",
        {playerUid, ruleId, severity, description, std::to_string(scoreImpact), std::to_string(timestamp)});
}

nlohmann::json Database::getScanHistory(const std::string& playerUid, int limit) {
    std::string sql = "SELECT * FROM scan_flags";
    std::vector<std::string> params;

    if (!playerUid.empty()) {
        sql += " WHERE player_uid = ?";
        params.push_back(playerUid);
    }
    sql += " ORDER BY timestamp DESC LIMIT ?";
    params.push_back(std::to_string(limit));

    auto rows = query(sql, params);
    nlohmann::json result = nlohmann::json::array();
    for (auto& row : rows) {
        nlohmann::json obj;
        for (auto& [k, v] : row) obj[k] = v;
        result.push_back(obj);
    }
    return result;
}

int Database::getPlayerSuspicionScore(const std::string& playerUid) {
    auto rows = query(
        "SELECT SUM(score_impact) as total FROM scan_flags WHERE player_uid = ? AND dismissed = 0",
        {playerUid});
    if (!rows.empty() && !rows[0].empty()) {
        try { return std::stoi(rows[0][0].second); }
        catch (...) { return 0; }
    }
    return 0;
}

void Database::dismissScanFlag(int flagId) {
    execute("UPDATE scan_flags SET dismissed = 1 WHERE id = ?", {std::to_string(flagId)});
}

void Database::setFlagActioned(int flagId, const std::string& action) {
    execute("UPDATE scan_flags SET action_taken = ? WHERE id = ?",
        {action, std::to_string(flagId)});
}

void Database::recordBackup(const std::string& path, int64_t sizeBytes, int64_t timestamp) {
    execute("INSERT INTO backups (path, size_bytes, timestamp) VALUES (?, ?, ?)",
        {path, std::to_string(sizeBytes), std::to_string(timestamp)});
}

nlohmann::json Database::getBackupHistory(int limit) {
    auto rows = query("SELECT * FROM backups ORDER BY timestamp DESC LIMIT ?",
        {std::to_string(limit)});
    nlohmann::json result = nlohmann::json::array();
    for (auto& row : rows) {
        nlohmann::json obj;
        for (auto& [k, v] : row) obj[k] = v;
        result.push_back(obj);
    }
    return result;
}

void Database::deleteBackupRecord(int id) {
    execute("DELETE FROM backups WHERE id = ?", {std::to_string(id)});
}

void Database::recordLog(const std::string& category, const std::string& message,
                         const std::string& playerUid, int64_t timestamp) {
    if (timestamp == 0) {
        timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    execute("INSERT INTO logs (category, message, player_uid, timestamp) VALUES (?, ?, ?, ?)",
        {category, message, playerUid, std::to_string(timestamp)});
}

nlohmann::json Database::getLogs(const std::string& category, int limit) {
    std::string sql = "SELECT * FROM logs";
    std::vector<std::string> params;
    if (!category.empty()) {
        sql += " WHERE category = ?";
        params.push_back(category);
    }
    sql += " ORDER BY timestamp DESC LIMIT ?";
    params.push_back(std::to_string(limit));

    auto rows = query(sql, params);
    nlohmann::json result = nlohmann::json::array();
    for (auto& row : rows) {
        nlohmann::json obj;
        for (auto& [k, v] : row) obj[k] = v;
        result.push_back(obj);
    }
    return result;
}

void Database::recordPlayerPosition(const std::string& uid, float x, float y, float z, int64_t timestamp) {
    execute("INSERT INTO player_positions (uid, x, y, z, timestamp) VALUES (?, ?, ?, ?, ?)",
        {uid, std::to_string(x), std::to_string(y), std::to_string(z), std::to_string(timestamp)});
}

nlohmann::json Database::getPositionHeatmapData(int64_t startTime, int64_t endTime) {
    std::string sql = "SELECT x, y, COUNT(*) as weight FROM player_positions";
    std::vector<std::string> params;
    if (startTime > 0 && endTime > 0) {
        sql += " WHERE timestamp BETWEEN ? AND ?";
        params.push_back(std::to_string(startTime));
        params.push_back(std::to_string(endTime));
    }
    sql += " GROUP BY CAST(x/100 AS INT), CAST(y/100 AS INT)";

    auto rows = query(sql, params);
    nlohmann::json result = nlohmann::json::array();
    for (auto& row : rows) {
        nlohmann::json obj;
        for (auto& [k, v] : row) obj[k] = v;
        result.push_back(obj);
    }
    return result;
}

void Database::addScanWhitelist(const std::string& uid) {
    execute("INSERT OR IGNORE INTO scan_whitelist (uid) VALUES (?)", {uid});
}

void Database::removeScanWhitelist(const std::string& uid) {
    execute("DELETE FROM scan_whitelist WHERE uid = ?", {uid});
}

bool Database::isScanWhitelisted(const std::string& uid) {
    auto rows = query("SELECT uid FROM scan_whitelist WHERE uid = ?", {uid});
    return !rows.empty();
}

std::vector<std::string> Database::getScanWhitelist() {
    auto rows = query("SELECT uid FROM scan_whitelist", {});
    std::vector<std::string> result;
    for (auto& row : rows) {
        if (!row.empty()) result.push_back(row[0].second);
    }
    return result;
}

} // namespace PSM