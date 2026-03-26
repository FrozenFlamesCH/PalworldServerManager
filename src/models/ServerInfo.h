#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace PSM {

struct ServerInfo {
    std::string name;
    std::string version;
    std::string palDefenderVersion;
    std::string description;
    bool online = false;
    int currentPlayers = 0;
    int maxPlayers = 0;
    float fps = 0;
    float frameTime = 0;
    int uptime = 0;
    int inGameDay = 0;
    float inGameTime = 0;

    nlohmann::json toJson() const {
        return {
            {"name", name},
            {"version", version},
            {"online", online},
            {"currentPlayers", currentPlayers},
            {"maxPlayers", maxPlayers},
            {"fps", fps},
            {"uptime", uptime},
            {"inGameDay", inGameDay}
        };
    }
};

struct ServerSettings {
    nlohmann::json raw;

    float expRate = 1.0f;
    float captureRate = 1.0f;
    float palSpawnRate = 1.0f;
    float dayTimeSpeed = 1.0f;
    float nightTimeSpeed = 1.0f;
    std::string deathPenalty;
    bool isPvP = false;
    bool isHardcore = false;
    bool friendlyFire = false;
    int baseCampMaxNum = 3;
    float hatchingSpeed = 1.0f;
    int baseCampWorkerMaxNum = 15;
};

} // namespace PSM
