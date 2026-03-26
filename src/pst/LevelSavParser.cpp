#include "pst/LevelSavParser.h"
#include <fstream>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cstring>

// For decompression - Palworld uses zlib
// In a real implementation you'd use zlib to decompress the GVAS data
#ifdef _WIN32
#include <windows.h>
#endif

namespace PSM {

LevelSavParser::LevelSavParser(const std::string& levelSavPath, int syncIntervalSeconds)
    : m_path(levelSavPath), m_syncInterval(syncIntervalSeconds) {}

LevelSavParser::~LevelSavParser() {
    stopAutoSync();
}

void LevelSavParser::setPath(const std::string& path) {
    m_path = path;
}

void LevelSavParser::setSyncInterval(int seconds) {
    m_syncInterval = seconds;
}

bool LevelSavParser::parse() {
    if (m_path.empty()) return false;

    try {
        if (!decompressAndParse()) return false;

        m_lastSync = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        detectPlayerChanges();

        for (auto& cb : m_syncCallbacks) {
            cb();
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Level.sav parse error: " << e.what() << std::endl;
        return false;
    }
}

bool LevelSavParser::testParse() {
    if (m_path.empty()) return false;

    std::ifstream file(m_path, std::ios::binary);
    if (!file.is_open()) return false;

    // Read header magic bytes
    char magic[4];
    file.read(magic, 4);

    // GVAS magic: "GVAS" (0x47 0x56 0x41 0x53)
    // Or compressed format
    return file.good();
}

bool LevelSavParser::decompressAndParse() {
    std::ifstream file(m_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    size_t fileSize = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> rawData(fileSize);
    file.read(reinterpret_cast<char*>(rawData.data()), fileSize);
    file.close();

    if (fileSize < 12) return false;

    // Check for compression header
    // Palworld Level.sav files may have a custom header followed by zlib-compressed GVAS data
    // The exact format depends on the game version

    // For this implementation, we'll attempt to parse the raw data
    // In production, you'd use zlib::uncompress or a Palworld-specific decompressor

    // Check GVAS magic
    if (rawData[0] == 'G' && rawData[1] == 'V' && rawData[2] == 'A' && rawData[3] == 'S') {
        return parseGvas(rawData);
    }

    // Try decompression
    // Palworld uses a custom compression format:
    // [4 bytes: magic "PlZ"] [4 bytes: compression type] [8 bytes: uncompressed size] [compressed data]
    // This is a simplified representation

    uint32_t magic;
    memcpy(&magic, rawData.data(), 4);

    // Attempt to decompress based on known formats
    // For now, store empty data - the real implementation would decompress here
    std::vector<uint8_t> decompressed;

    // Placeholder: In production, use zlib inflate
    // z_stream stream;
    // ...

    if (!decompressed.empty()) {
        return parseGvas(decompressed);
    }

    // If we can't decompress, try parsing raw
    return parseGvas(rawData);
}

bool LevelSavParser::parseGvas(const std::vector<uint8_t>& data) {
    if (data.size() < 20) return false;

    // GVAS format parsing
    // This is a simplified version - full GVAS parsing is complex
    // In production, you'd want a complete GVAS parser

    std::lock_guard<std::mutex> lock(m_dataMutex);

    // Parse the GVAS property tree
    // The format is:
    // Header: magic(4) + version(4) + package_version(4) + engine_version(variable) + ...
    // Properties: type_name + property_data (recursive)

    size_t offset = 0;

    // Skip header (simplified)
    if (data[0] == 'G' && data[1] == 'V' && data[2] == 'A' && data[3] == 'S') {
        offset += 4; // magic
        // In a real parser, read version info, custom format version, etc.
        // For now, we'll mark this as a stub that needs the full GVAS implementation
    }

    // Note: Full GVAS parsing is extremely complex and game-version dependent.
    // In a production tool, you would either:
    // 1. Use palworld-save-tools (Python) as a subprocess
    // 2. Port the GVAS parser from palworld-save-tools to C++
    // 3. Use the uesave library

    // For this implementation, we provide the structure and interface
    // The actual parsing would be filled in based on the specific save format version

    return true;
}

void LevelSavParser::extractPlayers(const GvasPropertyPtr& root) {
    // Extract player data from parsed GVAS tree
    // This would navigate the property tree to find player character save data
}

void LevelSavParser::extractPals(const GvasPropertyPtr& root) {
    // Extract pal data from parsed GVAS tree
}

void LevelSavParser::extractGuilds(const GvasPropertyPtr& root) {
    // Extract guild data from parsed GVAS tree
}

void LevelSavParser::detectPlayerChanges() {
    std::vector<std::string> currentOnline;
    for (auto& [uid, player] : m_players) {
        if (player.online) currentOnline.push_back(uid);
    }

    // Find newly joined players
    for (auto& uid : currentOnline) {
        if (std::find(m_previousOnlinePlayers.begin(), m_previousOnlinePlayers.end(), uid)
            == m_previousOnlinePlayers.end()) {
            auto it = m_players.find(uid);
            std::string name = (it != m_players.end()) ? it->second.name : "Unknown";
            for (auto& cb : m_playerCallbacks) {
                cb(uid, name, true);
            }
        }
    }

    // Find players who left
    for (auto& uid : m_previousOnlinePlayers) {
        if (std::find(currentOnline.begin(), currentOnline.end(), uid) == currentOnline.end()) {
            auto it = m_players.find(uid);
            std::string name = (it != m_players.end()) ? it->second.name : "Unknown";
            for (auto& cb : m_playerCallbacks) {
                cb(uid, name, false);
            }
        }
    }

    m_previousOnlinePlayers = currentOnline;
}

void LevelSavParser::startAutoSync() {
    if (m_running) return;
    m_running = true;
    m_syncThread = std::thread(&LevelSavParser::syncThread, this);
}

void LevelSavParser::stopAutoSync() {
    m_running = false;
    if (m_syncThread.joinable()) {
        m_syncThread.join();
    }
}

bool LevelSavParser::isAutoSyncing() const {
    return m_running;
}

int64_t LevelSavParser::lastSyncTime() const {
    return m_lastSync;
}

void LevelSavParser::syncThread() {
    while (m_running) {
        parse();

        // Sleep in small intervals to allow quick shutdown
        for (int i = 0; i < m_syncInterval * 10 && m_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

std::vector<Player> LevelSavParser::getAllPlayers() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    std::vector<Player> result;
    for (auto& [uid, player] : m_players) {
        result.push_back(player);
    }
    return result;
}

Player LevelSavParser::getPlayer(const std::string& uid) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    auto it = m_players.find(uid);
    if (it != m_players.end()) return it->second;
    return Player{};
}

std::vector<Item> LevelSavParser::getPlayerInventory(const std::string& uid) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    auto it = m_inventories.find(uid);
    if (it != m_inventories.end()) return it->second;
    return {};
}

std::vector<Pal> LevelSavParser::getPlayerPals(const std::string& uid) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    auto it = m_playerPals.find(uid);
    if (it != m_playerPals.end()) return it->second;
    return {};
}

std::vector<Pal> LevelSavParser::getAllPals() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_allPals;
}

std::vector<Guild> LevelSavParser::getGuilds() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_guilds;
}

void LevelSavParser::onSync(SyncCallback callback) {
    m_syncCallbacks.push_back(std::move(callback));
}

void LevelSavParser::onPlayerEvent(PlayerEventCallback callback) {
    m_playerCallbacks.push_back(std::move(callback));
}

} // namespace PSM
