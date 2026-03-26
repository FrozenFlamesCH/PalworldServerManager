#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace PSM {

struct PlayerPosition {
    float x = 0, y = 0, z = 0;
};

struct PlayerStats {
    int level = 0;
    int64_t exp = 0;
    int hp = 0;
    int maxHp = 0;
    int stamina = 0;
    int attack = 0;
    int defense = 0;
    int weight = 0;
    int craftSpeed = 0;
    int techPoints = 0;
    int ancientTechPoints = 0;
    int effigies = 0;
};

struct Player {
    std::string uid;
    std::string steamId;
    std::string name;
    std::string ip;
    bool online = false;
    PlayerPosition position;
    PlayerStats stats;
    std::string guildId;
    std::string guildName;
    int suspicionScore = 0;
    int64_t lastSeen = 0;
    int64_t totalPlaytime = 0;
    std::vector<std::string> unlockedTechs;

    nlohmann::json toJson() const {
        return {
            {"uid", uid},
            {"steamId", steamId},
            {"name", name},
            {"ip", ip},
            {"online", online},
            {"position", {{"x", position.x}, {"y", position.y}, {"z", position.z}}},
            {"level", stats.level},
            {"guildId", guildId},
            {"guildName", guildName},
            {"suspicionScore", suspicionScore}
        };
    }
};

} // namespace PSM