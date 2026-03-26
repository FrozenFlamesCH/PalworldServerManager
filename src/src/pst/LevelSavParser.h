#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include "models/Player.h"
#include "models/Pal.h"
#include "models/Item.h"
#include "models/Guild.h"
#include "pst/GvasProperty.h"

namespace PSM {

class LevelSavParser {
public:
    LevelSavParser(const std::string& levelSavPath, int syncIntervalSeconds);
    ~LevelSavParser();

    void setPath(const std::string& path);
    void setSyncInterval(int seconds);

    bool parse();
    bool testParse();

    void startAutoSync();
    void stopAutoSync();
    bool isAutoSyncing() const;
    int64_t lastSyncTime() const;

    // Parsed data accessors
    std::vector<Player> getAllPlayers() const;
    Player getPlayer(const std::string& uid) const;
    std::vector<Item> getPlayerInventory(const std::string& uid) const;
    std::vector<Pal> getPlayerPals(const std::string& uid) const;
    std::vector<Pal> getAllPals() const;
    std::vector<Guild> getGuilds() const;

    // Callbacks
    using SyncCallback = std::function<void()>;
    void onSync(SyncCallback callback);

    // Player join/leave detection
    using PlayerEventCallback = std::function<void(const std::string& uid, const std::string& name, bool joined)>;
    void onPlayerEvent(PlayerEventCallback callback);

private:
    bool decompressAndParse();
    bool parseGvas(const std::vector<uint8_t>& data);
    void extractPlayers(const GvasPropertyPtr& root);
    void extractPals(const GvasPropertyPtr& root);
    void extractGuilds(const GvasPropertyPtr& root);
    void detectPlayerChanges();

    void syncThread();

    std::string m_path;
    int m_syncInterval;
    std::atomic<bool> m_running{false};
    std::thread m_syncThread;
    std::atomic<int64_t> m_lastSync{0};

    mutable std::mutex m_dataMutex;
    std::map<std::string, Player> m_players;
    std::map<std::string, std::vector<Item>> m_inventories;
    std::map<std::string, std::vector<Pal>> m_playerPals;
    std::vector<Pal> m_allPals;
    std::vector<Guild> m_guilds;
    std::vector<std::string> m_previousOnlinePlayers;

    std::vector<SyncCallback> m_syncCallbacks;
    std::vector<PlayerEventCallback> m_playerCallbacks;
};

} // namespace PSM
