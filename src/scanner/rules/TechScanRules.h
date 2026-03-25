#pragma once

#include "scanner/ScanRule.h"

namespace PSM {

// Rule: Technologies unlocked beyond player level
class TechBeyondLevelRule : public ScanRule {
public:
    TechBeyondLevelRule()
        : ScanRule("TECH_BEYOND_LEVEL", "Tech Beyond Level",
                   ScanSeverity::Medium, "Player has unlocked technologies beyond their level") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        // This would require a mapping of tech ID -> required level
        // For now, flag if player has more than expected tech count for their level
        int expectedMaxTechs = ctx.player.stats.level * 3; // rough estimate
        if (static_cast<int>(ctx.player.unlockedTechs.size()) > expectedMaxTechs * 2) {
            flags.push_back(createFlag(ctx.player.uid,
                "Player level " + std::to_string(ctx.player.stats.level) +
                " has " + std::to_string(ctx.player.unlockedTechs.size()) +
                " techs (expected max ~" + std::to_string(expectedMaxTechs) + ")"));
        }
        return flags;
    }
};

} // namespace PSM
