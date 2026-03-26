#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace PSM {

struct PalPassive {
    std::string id;
    std::string name;
    int rank = 0; // 1-4
};

struct PalSkill {
    std::string id;
    std::string name;
    int level = 0;
};

struct Pal {
    std::string instanceId;
    std::string characterId;  // e.g. "Lambball"
    std::string nickname;
    std::string ownerUid;
    int level = 0;
    int rank = 0;
    bool isShiny = false;
    bool isLegendary = false;
    bool isBoss = false;
    int gender = 0; // 0=male, 1=female
    float hp = 0;
    float maxHp = 0;
    float attack = 0;
    float defense = 0;
    std::vector<PalPassive> passives;
    std::vector<PalSkill> skills;
    float talent_hp = 0;
    float talent_melee = 0;
    float talent_shot = 0;
    float talent_defense = 0;

    nlohmann::json toJson() const {
        return {
            {"instanceId", instanceId},
            {"characterId", characterId},
            {"nickname", nickname},
            {"ownerUid", ownerUid},
            {"level", level},
            {"rank", rank},
            {"isShiny", isShiny},
            {"isLegendary", isLegendary},
            {"gender", gender}
        };
    }
};

} // namespace PSM