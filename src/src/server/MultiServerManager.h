#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace PSM {

class VanillaAPI;
class PalDefenderAPI;
class PalDefenderRCON;
class ServerManager;

struct ServerProfile {
    std::string id;
    std::string name;
    std::string vanillaHost = "127.0.0.1";
    int vanillaPort = 8212;
    std::string vanillaPassword;
    std::string pdHost = "127.0.0.1";
    int pdPort = 8214;
    std::string pdApiKey;
    std::string rconHost = "127.0.0.1";
    int rconPort = 25575;
    std::string rconPassword;
    std::string exePath;
    std::string savePath;
    bool isActive = false;
};

class MultiServerManager {
public:
    MultiServerManager();
    ~MultiServerManager();

    void addServer(const ServerProfile& profile);
    void removeServer(const std::string& id);
    void setActiveServer(const std::string& id);

    ServerProfile* getActiveServer();
    const std::vector<ServerProfile>& getAllServers() const;
    ServerProfile* getServer(const std::string& id);

    // Convenience
    bool switchTo(const std::string& id);
    std::string activeServerId() const;

    // Serialization
    void loadFromJson(const nlohmann::json& data);
    nlohmann::json toJson() const;

private:
    std::vector<ServerProfile> m_servers;
    std::string m_activeServerId;
};

} // namespace PSM