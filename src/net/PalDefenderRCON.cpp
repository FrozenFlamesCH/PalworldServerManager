#include "net/PalDefenderRCON.h"

namespace PSM {

PalDefenderRCON::PalDefenderRCON(RconClient& rcon, const std::string& host, int port, const std::string& password)
    : m_rcon(rcon), m_host(host), m_port(port), m_password(password) {}

PalDefenderRCON::~PalDefenderRCON() {}

void PalDefenderRCON::updateConnection(const std::string& host, int port, const std::string& password) {
    m_host = host;
    m_port = port;
    m_password = password;
}

bool PalDefenderRCON::connect() {
    return m_rcon.connect(m_host, m_port, m_password);
}

bool PalDefenderRCON::isConnected() const {
    return m_rcon.isConnected();
}

bool PalDefenderRCON::testConnection() {
    return m_rcon.testConnection(m_host, m_port, m_password);
}

std::string PalDefenderRCON::cmd(const std::string& command) {
    auto resp = m_rcon.sendCommand(command);
    if (resp.success) return resp.body;
    return "Error: " + resp.error;
}

// Player Management
std::string PalDefenderRCON::kickPlayer(const std::string& uid, const std::string& reason) {
    return cmd("pdkick " + uid + (reason.empty() ? "" : " " + reason));
}

std::string PalDefenderRCON::banPlayer(const std::string& uid, const std::string& reason) {
    return cmd("pdban " + uid + (reason.empty() ? "" : " " + reason));
}

std::string PalDefenderRCON::unbanPlayer(const std::string& uid) {
    return cmd("pdunban " + uid);
}

std::string PalDefenderRCON::ipBanPlayer(const std::string& uid) {
    return cmd("pdipban " + uid);
}

std::string PalDefenderRCON::banIP(const std::string& ip) {
    return cmd("pdbanip " + ip);
}

std::string PalDefenderRCON::unbanIP(const std::string& ip) {
    return cmd("pdunbanip " + ip);
}

std::string PalDefenderRCON::getPlayerIP(const std::string& uid) {
    return cmd("pdgetip " + uid);
}

std::string PalDefenderRCON::renamePlayer(const std::string& uid, const std::string& newName) {
    return cmd("pdrename " + uid + " " + newName);
}

// Whitelist
std::string PalDefenderRCON::whitelistAdd(const std::string& uid) {
    return cmd("pdwhitelist add " + uid);
}

std::string PalDefenderRCON::whitelistRemove(const std::string& uid) {
    return cmd("pdwhitelist remove " + uid);
}

std::string PalDefenderRCON::whitelistView() {
    return cmd("pdwhitelist list");
}

// Player Rewards
std::string PalDefenderRCON::giveExp(const std::string& uid, int amount) {
    return cmd("pdgiveexp " + uid + " " + std::to_string(amount));
}

std::string PalDefenderRCON::giveStatusPoints(const std::string& uid, int amount) {
    return cmd("pdgivestatuspoints " + uid + " " + std::to_string(amount));
}

std::string PalDefenderRCON::giveEffigies(const std::string& uid, int amount) {
    return cmd("pdgiveeffigies " + uid + " " + std::to_string(amount));
}

std::string PalDefenderRCON::giveTechPoints(const std::string& uid, int amount) {
    return cmd("pdgivetechpoints " + uid + " " + std::to_string(amount));
}

std::string PalDefenderRCON::giveAncientTechPoints(const std::string& uid, int amount) {
    return cmd("pdgiveancienttechpoints " + uid + " " + std::to_string(amount));
}

std::string PalDefenderRCON::unlockTech(const std::string& uid, const std::string& techId) {
    return cmd("pdunlocktech " + uid + " " + techId);
}

std::string PalDefenderRCON::unlockAllTech(const std::string& uid) {
    return cmd("pdunlockalltech " + uid);
}

std::string PalDefenderRCON::removeTech(const std::string& uid, const std::string& techId) {
    return cmd("pdremovetech " + uid + " " + techId);
}

std::string PalDefenderRCON::removeAllTech(const std::string& uid) {
    return cmd("pdremovealltech " + uid);
}

// Admin
std::string PalDefenderRCON::grantAdmin(const std::string& uid) {
    return cmd("pdgrantadmin " + uid);
}

std::string PalDefenderRCON::revokeAdmin(const std::string& uid) {
    return cmd("pdrevokeadmin " + uid);
}

std::string PalDefenderRCON::setGuildLeader(const std::string& guildId, const std::string& uid) {
    return cmd("pdsetguildleader " + guildId + " " + uid);
}

// Items & Pals
std::string PalDefenderRCON::giveItem(const std::string& uid, const std::string& itemId, int count) {
    return cmd("pdgiveitem " + uid + " " + itemId + " " + std::to_string(count));
}

std::string PalDefenderRCON::deleteItem(const std::string& uid, const std::string& itemId, int count) {
    std::string c = "pddeleteitem " + uid + " " + itemId;
    if (count > 0) c += " " + std::to_string(count);
    return cmd(c);
}

std::string PalDefenderRCON::clearInventory(const std::string& uid, const std::string& container) {
    return cmd("pdclearinventory " + uid + " " + container);
}

std::string PalDefenderRCON::givePal(const std::string& uid, const std::string& palId, int level) {
    return cmd("pdgivepal " + uid + " " + palId + " " + std::to_string(level));
}

std::string PalDefenderRCON::givePalEgg(const std::string& uid, const std::string& palId) {
    return cmd("pdgivepalegg " + uid + " " + palId);
}

std::string PalDefenderRCON::deletePals(const std::string& uid, const std::string& filter) {
    return cmd("pddeletepals " + uid + " " + filter);
}

std::string PalDefenderRCON::exportPals(const std::string& uid) {
    return cmd("pdexportpals " + uid);
}

// Spawning
std::string PalDefenderRCON::spawnPal(const std::string& palId, float x, float y, float z) {
    return cmd("pdspawnpal " + palId + " " +
        std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
}

std::string PalDefenderRCON::spawnPalFromTemplate(const std::string& templatePath, float x, float y, float z) {
    return cmd("pdspawnpaltemplate " + templatePath + " " +
        std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
}

std::string PalDefenderRCON::spawnPalFromSummon(const std::string& summonFile) {
    return cmd("pdspawnpalsummon " + summonFile);
}

// Communication
std::string PalDefenderRCON::broadcast(const std::string& message) {
    return cmd("pdbroadcast " + message);
}

std::string PalDefenderRCON::sendMessage(const std::string& uid, const std::string& message) {
    return cmd("pdsendmessage " + uid + " " + message);
}

std::string PalDefenderRCON::alert(const std::string& message) {
    return cmd("pdalert " + message);
}

// Teleport
std::string PalDefenderRCON::teleportPlayerToPlayer(const std::string& uid1, const std::string& uid2) {
    return cmd("pdteleport " + uid1 + " " + uid2);
}

// World Control
std::string PalDefenderRCON::setTime(int hour) {
    return cmd("pdsettime " + std::to_string(hour));
}

std::string PalDefenderRCON::setTimeDay() {
    return cmd("pdsettime day");
}

std::string PalDefenderRCON::setTimeNight() {
    return cmd("pdsettime night");
}

std::string PalDefenderRCON::shutdown(int countdown, const std::string& message) {
    std::string c = "pdshutdown " + std::to_string(countdown);
    if (!message.empty()) c += " " + message;
    return cmd(c);
}

std::string PalDefenderRCON::reloadConfig() {
    return cmd("pdreload");
}

std::string PalDefenderRCON::destroyBase(const std::string& baseId) {
    return cmd("pddestroybase " + baseId);
}

std::string PalDefenderRCON::getPlayerPositions() {
    return cmd("pdgetplayerpositions");
}

std::string PalDefenderRCON::getNearestBase(float x, float y, float z) {
    return cmd("pdnearestbase " +
        std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
}

std::string PalDefenderRCON::getAvailableTechIds() {
    return cmd("pdgettechids");
}

std::string PalDefenderRCON::getAvailableSkinIds() {
    return cmd("pdgetskinids");
}

std::string PalDefenderRCON::exportGuildsToJson() {
    return cmd("pdexportguilds");
}

std::string PalDefenderRCON::sendRaw(const std::string& command) {
    return cmd(command);
}

} // namespace PSM
