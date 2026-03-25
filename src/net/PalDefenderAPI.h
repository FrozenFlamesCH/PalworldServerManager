#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "net/HttpClient.h"
#include "models/Guild.h"

namespace PSM {

class PalDefenderAPI {
public:
    PalDefenderAPI(HttpClient& http, const std::string& host, int port, const std::string& apiKey);
    ~PalDefenderAPI();

    void updateConnection(const std::string& host, int port, const std::string& apiKey);
    bool testConnection();

    // Version
    std::string getVersion();

    // Player
    bool giveExp(const std::string& uid, int amount);
    bool giveEffigies(const std::string& uid, int amount);
    bool giveTechPoints(const std::string& uid, int amount);
    bool giveAncientTechPoints(const std::string& uid, int amount);
    bool unlockTech(const std::string& uid, const std::string& techId);
    bool unlockAllTech(const std::string& uid);
    bool giveItem(const std::string& uid, const std::string& itemId, int count);
    bool givePal(const std::string& uid, const std::string& palId, int level = 1);
    bool givePalEgg(const std::string& uid, const std::string& palId);
    bool givePalEggFromTemplate(const std::string& uid, const std::string& templatePath);

    // Guilds
    nlohmann::json getGuilds();
    nlohmann::json getGuildDetail(const std::string& guildId);
    std::vector<Guild> fetchAllGuilds();

    // Bases
    nlohmann::json getBases();
    nlohmann::json getBaseDetail(const std::string& baseId);

private:
    std::string buildUrl(const std::string& endpoint) const;
    std::map<std::string, std::string> apiHeaders() const;

    HttpClient& m_http;
    std::string m_host;
    int m_port;
    std::string m_apiKey;
    std::string m_baseUrl;
};

} // namespace PSM
