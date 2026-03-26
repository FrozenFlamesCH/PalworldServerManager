#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace PSM {

struct Base {
    std::string id;
    std::string guildId;
    std::string guildName;
    float x = 0, y = 0, z = 0;
    int level = 0;
    int palCount = 0;
    std::string state;
    bool hasShinyPals = false;
    bool hasLegendaryPals = false;
    std::vector<std::string> palIds;
};

} // namespace PSM
