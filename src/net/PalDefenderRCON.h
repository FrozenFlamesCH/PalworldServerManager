#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "net/RconClient.h"

namespace PSM {

class PalDefenderRCON {
public:
    PalDefenderRCON(RconClient& rcon, const std::string& host, int port, const std::string& password);
    ~PalDefenderRCON();

    void updateConnection(const std::string& host, int port, const std::string& password);
    bool connect();
    bool isConnected() const;
    bool testConnection();

    // Player Management
    std::string kickPlayer(const std::string& uid, const std::string& reason = "");
    std::string banPlayer(const std::string& uid, const std::string& reason = "");
    std::string unbanPlayer(const std::string& uid);
    std::string ipBanPlayer(const std::string& uid);
    std::string banIP(const std::string& ip);
    std::string unbanIP(const std::string& ip);
    std::string getPlayerIP(const std::string& uid);
    std::string renamePlayer(const std::string& uid, const std::string& newName);

    // Whitelist
    std::string whitelistAdd(const std::string& uid);
    std::string whitelistRemove(const std::string& uid);
    std::string whitelistView();

    // Player Rewards
    std::string giveExp(const std::string& uid, int amount);
    std::string giveStatusPoints(const std::string& uid, int amount);
    std::string giveEffigies(const std::string& uid, int amount);
    std::string giveTechPoints(const std::string& uid, int amount);
    std::string giveAncientTechPoints(const std::string& uid, int amount);
    std::string unlockTech(const std::string& uid, const std::string& techId);
    std::string unlockAllTech(const std::string& uid);
    std::string removeTech(const std::string& uid, const std::string& techId);
    std::string removeAllTech(const std::string& uid);

    // Admin
    std::string grantAdmin(const std::string& uid);
    std::string revokeAdmin(const std::string& uid);
    std::string setGuildLeader(const std::string& guildId, const std::string& uid);

    // Items & Pals
    std::string giveItem(const std::string& uid, const std::string& itemId, int count);
    std::string deleteItem(const std::string& uid, const std::string& itemId, int count = 0);
    std::string clearInventory(const std::string& uid, const std::string& container);
    std::string givePal(const std::string& uid, const std::string& palId, int level = 1);
    std::string givePalEgg(const std::string& uid, const std::string& palId);
    std::string deletePals(const std::string& uid, const std::string& filter);
    std::string exportPals(const std::string& uid);

    // Spawning
    std::string spawnPal(const std::string& palId, float x, float y, float z);
    std::string spawnPalFromTemplate(const std::string& templatePath, float x, float y, float z);
    std::string spawnPalFromSummon(const std::string& summonFile);

    // Communication
    std::string broadcast(const std::string& message);
    std::string sendMessage(const std::string& uid, const std::string& message);
    std::string alert(const std::string& message);

    // Teleport
    std::string teleportPlayerToPlayer(const std::string& uid1, const std::string& uid2);

    // World Control
    std::string setTime(int hour);
    std::string setTimeDay();
    std::string setTimeNight();
    std::string shutdown(int countdown = 0, const std::string& message = "");
    std::string reloadConfig();

    // Destroy
    std::string destroyBase(const std::string& baseId);

    // Info queries
    std::string getPlayerPositions();
    std::string getNearestBase(float x, float y, float z);
    std::string getAvailableTechIds();
    std::string getAvailableSkinIds();

    // Guild
    std::string exportGuildsToJson();

    // Raw command
    std::string sendRaw(const std::string& command);

private:
    std::string cmd(const std::string& command);

    RconClient& m_rcon;
    std::string m_host;
    int m_port;
    std::string m_password;
};

} // namespace PSM
