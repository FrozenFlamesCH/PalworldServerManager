#include "net/VanillaAPI.h"
#include <sstream>

namespace PSM {

VanillaAPI::VanillaAPI(HttpClient& http, const std::string& host, int port, const std::string& password)
    : m_http(http), m_host(host), m_port(port), m_password(password) {
    m_baseUrl = "http://" + host + ":" + std::to_string(port);
}

VanillaAPI::~VanillaAPI() {}

void VanillaAPI::updateConnection(const std::string& host, int port, const std::string& password) {
    m_host = host;
    m_port = port;
    m_password = password;
    m_baseUrl = "http://" + host + ":" + std::to_string(port);
}

std::string VanillaAPI::buildUrl(const std::string& endpoint) const {
    return m_baseUrl + "/v1/api/" + endpoint;
}

std::map<std::string, std::string> VanillaAPI::authHeaders() const {
    // Palworld uses Basic auth with admin:password
    std::string credentials = "admin:" + m_password;
    // Base64 encode
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    int val = 0, valb = -6;
    for (uint8_t c : credentials) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (encoded.size() % 4) encoded.push_back('=');

    return {
        {"Authorization", "Basic " + encoded},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"}
    };
}

bool VanillaAPI::testConnection() {
    auto resp = m_http.get(buildUrl("info"), authHeaders());
    return resp.success;
}

nlohmann::json VanillaAPI::getServerInfo() {
    auto resp = m_http.get(buildUrl("info"), authHeaders());
    return resp.json();
}

nlohmann::json VanillaAPI::getServerMetrics() {
    auto resp = m_http.get(buildUrl("metrics"), authHeaders());
    return resp.json();
}

nlohmann::json VanillaAPI::getServerSettings() {
    auto resp = m_http.get(buildUrl("settings"), authHeaders());
    return resp.json();
}

nlohmann::json VanillaAPI::getPlayers() {
    auto resp = m_http.get(buildUrl("players"), authHeaders());
    return resp.json();
}

bool VanillaAPI::kickPlayer(const std::string& uid, const std::string& reason) {
    nlohmann::json body = {{"userid", uid}, {"message", reason}};
    auto resp = m_http.post(buildUrl("kick"), body.dump(), authHeaders());
    return resp.success;
}

bool VanillaAPI::banPlayer(const std::string& uid) {
    nlohmann::json body = {{"userid", uid}};
    auto resp = m_http.post(buildUrl("ban"), body.dump(), authHeaders());
    return resp.success;
}

bool VanillaAPI::unbanPlayer(const std::string& uid) {
    nlohmann::json body = {{"userid", uid}};
    auto resp = m_http.post(buildUrl("unban"), body.dump(), authHeaders());
    return resp.success;
}

bool VanillaAPI::broadcastMessage(const std::string& message) {
    nlohmann::json body = {{"message", message}};
    auto resp = m_http.post(buildUrl("announce"), body.dump(), authHeaders());
    return resp.success;
}

bool VanillaAPI::saveWorld() {
    auto resp = m_http.post(buildUrl("save"), "{}", authHeaders());
    return resp.success;
}

bool VanillaAPI::shutdown(int countdown, const std::string& message) {
    nlohmann::json body = {{"waittime", countdown}, {"message", message}};
    auto resp = m_http.post(buildUrl("shutdown"), body.dump(), authHeaders());
    return resp.success;
}

bool VanillaAPI::forceStop() {
    auto resp = m_http.post(buildUrl("stop"), "{}", authHeaders());
    return resp.success;
}

ServerInfo VanillaAPI::fetchServerInfo() {
    ServerInfo info;
    auto data = getServerInfo();

    if (data.is_object()) {
        info.online = true;
        info.name = data.value("servername", "");
        info.version = data.value("version", "");
        info.description = data.value("description", "");
    }

    auto metrics = getServerMetrics();
    if (metrics.is_object()) {
        info.currentPlayers = metrics.value("currentplayernum", 0);
        info.maxPlayers = metrics.value("maxplayernum", 0);
        info.fps = metrics.value("serverfps", 0.0f);
        info.frameTime = metrics.value("serverframetime", 0.0f);
        info.uptime = metrics.value("uptime", 0);
        info.inGameDay = metrics.value("days", 0);
    }

    return info;
}

ServerSettings VanillaAPI::fetchServerSettings() {
    ServerSettings settings;
    auto data = getServerSettings();
    settings.raw = data;

    if (data.is_object()) {
        settings.expRate = data.value("ExpRate", 1.0f);
        settings.captureRate = data.value("PalCaptureRate", 1.0f);
        settings.palSpawnRate = data.value("PalSpawnNumRate", 1.0f);
        settings.dayTimeSpeed = data.value("DayTimeSpeedRate", 1.0f);
        settings.nightTimeSpeed = data.value("NightTimeSpeedRate", 1.0f);
        settings.deathPenalty = data.value("DeathPenalty", "None");
        settings.isPvP = data.value("bIsPvP", false);
        settings.friendlyFire = data.value("bEnablePlayerToPlayerDamage", false);
        settings.baseCampMaxNum = data.value("BaseCampMaxNum", 3);
        settings.baseCampWorkerMaxNum = data.value("BaseCampWorkerMaxNum", 15);
    }

    return settings;
}

std::vector<Player> VanillaAPI::fetchOnlinePlayers() {
    std::vector<Player> players;
    auto data = getPlayers();

    if (data.is_object() && data.contains("players")) {
        for (auto& p : data["players"]) {
            Player player;
            player.uid = p.value("userId", "");
            player.name = p.value("name", "");
            player.steamId = p.value("accountName", "");
            player.online = true;
            player.stats.level = p.value("level", 0);

            if (p.contains("location")) {
                player.position.x = p["location"].value("x", 0.0f);
                player.position.y = p["location"].value("y", 0.0f);
                player.position.z = p["location"].value("z", 0.0f);
            }

            players.push_back(player);
        }
    }

    return players;
}

} // namespace PSM
