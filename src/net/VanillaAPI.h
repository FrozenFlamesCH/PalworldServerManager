#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "net/HttpClient.h"
#include "models/ServerInfo.h"
#include "models/Player.h"

namespace PSM {

class VanillaAPI {
public:
    VanillaAPI(HttpClient& http, const std::string& host, int port, const std::string& password);
    ~VanillaAPI();

    void updateConnection(const std::string& host, int port, const std::string& password);
    bool testConnection();

    // Server Info
    nlohmann::json getServerInfo();
    nlohmann::json getServerMetrics();
    nlohmann::json getServerSettings();

    // Players
    nlohmann::json getPlayers();

    // Actions
    bool kickPlayer(const std::string& uid, const std::string& reason = "");
    bool banPlayer(const std::string& uid);
    bool unbanPlayer(const std::string& uid);
    bool broadcastMessage(const std::string& message);

    // World
    bool saveWorld();
    bool shutdown(int countdown = 0, const std::string& message = "");
    bool forceStop();

    // Parsed data
    ServerInfo fetchServerInfo();
    ServerSettings fetchServerSettings();
    std::vector<Player> fetchOnlinePlayers();

private:
    std::string buildUrl(const std::string& endpoint) const;
    std::map<std::string, std::string> authHeaders() const;

    HttpClient& m_http;
    std::string m_host;
    int m_port;
    std::string m_password;
    std::string m_baseUrl;
};

} // namespace PSM
