#include "net/PalDefenderAPI.h"

namespace PSM {

PalDefenderAPI::PalDefenderAPI(HttpClient& http, const std::string& host, int port, const std::string& apiKey)
    : m_http(http), m_host(host), m_port(port), m_apiKey(apiKey) {
    m_baseUrl = "http://" + host + ":" + std::to_string(port);
}

PalDefenderAPI::~PalDefenderAPI() {}

void PalDefenderAPI::updateConnection(const std::string& host, int port, const std::string& apiKey) {
    m_host = host;
    m_port = port;
    m_apiKey = apiKey;
    m_baseUrl = "http://" + host + ":" + std::to_string(port);
}

std::string PalDefenderAPI::buildUrl(const std::string& endpoint) const {
    return m_baseUrl + "/api/" + endpoint;
}

std::map<std::string, std::string> PalDefenderAPI::apiHeaders() const {
    return {
        {"X-API-Key", m_apiKey},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"}
    };
}

bool PalDefenderAPI::testConnection() {
    auto resp = m_http.get(buildUrl("version"), apiHeaders());
    return resp.success;
}

std::string PalDefenderAPI::getVersion() {
    auto resp = m_http.get(buildUrl("version"), apiHeaders());
    if (resp.success) {
        auto j = resp.json();
        return j.value("version", "unknown");
    }
    return "";
}

bool PalDefenderAPI::giveExp(const std::string& uid, int amount) {
    nlohmann::json body = {{"uid", uid}, {"amount", amount}};
    auto resp = m_http.post(buildUrl("player/exp"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::giveEffigies(const std::string& uid, int amount) {
    nlohmann::json body = {{"uid", uid}, {"amount", amount}};
    auto resp = m_http.post(buildUrl("player/effigies"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::giveTechPoints(const std::string& uid, int amount) {
    nlohmann::json body = {{"uid", uid}, {"amount", amount}};
    auto resp = m_http.post(buildUrl("player/techpoints"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::giveAncientTechPoints(const std::string& uid, int amount) {
    nlohmann::json body = {{"uid", uid}, {"amount", amount}};
    auto resp = m_http.post(buildUrl("player/ancienttechpoints"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::unlockTech(const std::string& uid, const std::string& techId) {
    nlohmann::json body = {{"uid", uid}, {"techId", techId}};
    auto resp = m_http.post(buildUrl("player/unlocktech"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::unlockAllTech(const std::string& uid) {
    nlohmann::json body = {{"uid", uid}};
    auto resp = m_http.post(buildUrl("player/unlockalltech"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::giveItem(const std::string& uid, const std::string& itemId, int count) {
    nlohmann::json body = {{"uid", uid}, {"itemId", itemId}, {"count", count}};
    auto resp = m_http.post(buildUrl("player/giveitem"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::givePal(const std::string& uid, const std::string& palId, int level) {
    nlohmann::json body = {{"uid", uid}, {"palId", palId}, {"level", level}};
    auto resp = m_http.post(buildUrl("player/givepal"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::givePalEgg(const std::string& uid, const std::string& palId) {
    nlohmann::json body = {{"uid", uid}, {"palId", palId}};
    auto resp = m_http.post(buildUrl("player/givepalegg"), body.dump(), apiHeaders());
    return resp.success;
}

bool PalDefenderAPI::givePalEggFromTemplate(const std::string& uid, const std::string& templatePath) {
    nlohmann::json body = {{"uid", uid}, {"templatePath", templatePath}};
    auto resp = m_http.post(buildUrl("player/givepaleggtemplate"), body.dump(), apiHeaders());
    return resp.success;
}

nlohmann::json PalDefenderAPI::getGuilds() {
    auto resp = m_http.get(buildUrl("guilds"), apiHeaders());
    return resp.json();
}

nlohmann::json PalDefenderAPI::getGuildDetail(const std::string& guildId) {
    auto resp = m_http.get(buildUrl("guilds/" + guildId), apiHeaders());
    return resp.json();
}

std::vector<Guild> PalDefenderAPI::fetchAllGuilds() {
    std::vector<Guild> guilds;
    auto data = getGuilds();

    if (data.is_array()) {
        for (auto& gd : data) {
            Guild g;
            g.id = gd.value("id", "");
            g.name = gd.value("name", "");
            g.leaderId = gd.value("leaderId", "");
            g.leaderName = gd.value("leaderName", "");
            g.memberCount = gd.value("memberCount", 0);

            if (gd.contains("members") && gd["members"].is_array()) {
                for (auto& md : gd["members"]) {
                    GuildMember m;
                    m.uid = md.value("uid", "");
                    m.name = md.value("name", "");
                    m.online = md.value("online", false);
                    m.isLeader = md.value("isLeader", false);
                    g.members.push_back(m);
                }
            }

            if (gd.contains("bases") && gd["bases"].is_array()) {
                for (auto& bd : gd["bases"]) {
                    GuildBase b;
                    b.id = bd.value("id", "");
                    b.name = bd.value("name", "");
                    b.x = bd.value("x", 0.0f);
                    b.y = bd.value("y", 0.0f);
                    b.z = bd.value("z", 0.0f);
                    b.palCount = bd.value("palCount", 0);
                    b.level = bd.value("level", 0);
                    b.state = bd.value("state", "");
                    b.hasShinyPals = bd.value("hasShinyPals", false);
                    b.hasLegendaryPals = bd.value("hasLegendaryPals", false);
                    g.bases.push_back(b);
                }
            }

            guilds.push_back(g);
        }
    }

    return guilds;
}

nlohmann::json PalDefenderAPI::getBases() {
    auto resp = m_http.get(buildUrl("bases"), apiHeaders());
    return resp.json();
}

nlohmann::json PalDefenderAPI::getBaseDetail(const std::string& baseId) {
    auto resp = m_http.get(buildUrl("bases/" + baseId), apiHeaders());
    return resp.json();
}

} // namespace PSM
