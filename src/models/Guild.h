#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace PSM {

struct GuildMember {
    std::string uid;
    std::string name;
    bool online = false;
    bool isLeader = false;
};

struct GuildBase {
    std::string id;
    std::string name;
    float x = 0, y = 0, z = 0;
    int palCount = 0;
    int level = 0;
    std::string state;
    bool hasShinyPals = false;
    bool hasLegendaryPals = false;
};

struct GuildExpedition {
    std::string id;
    std::string status;
    float progress = 0;
};

struct GuildLabResearch {
    std::string id;
    std::string name;
    std::string status;
    float progress = 0;
};

struct Guild {
    std::string id;
    std::string name;
    std::string leaderId;
    std::string leaderName;
    int memberCount = 0;
    std::vector<GuildMember> members;
    std::vector<GuildBase> bases;
    std::vector<GuildExpedition> expeditions;
    std::vector<GuildLabResearch> labResearch;
    nlohmann::json chestInventory;

    nlohmann::json toJson() const {
        nlohmann::json j;
        j["id"] = id;
        j["name"] = name;
        j["leaderId"] = leaderId;
        j["leaderName"] = leaderName;
        j["memberCount"] = memberCount;
        j["members"] = nlohmann::json::array();
        for (auto& m : members) {
            j["members"].push_back({
                {"uid", m.uid}, {"name", m.name},
                {"online", m.online}, {"isLeader", m.isLeader}
            });
        }
        j["baseCount"] = bases.size();
        return j;
    }
};

} // namespace PSM
