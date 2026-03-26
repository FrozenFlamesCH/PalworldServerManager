#pragma once

#include "scanner/ScanRule.h"

namespace PSM {

// Rule: Player stats exceeding level cap
class StatsExceedingCapRule : public ScanRule {
public:
    StatsExceedingCapRule()
        : ScanRule("STAT_EXCEEDING_CAP", "Stats Exceeding Cap",
                   ScanSeverity::High, "Player stats exceed what's possible at their level") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        // Each level gives limited stat points
        // Max stat points = level * points_per_level
        int maxStatPoints = ctx.player.stats.level * 4; // rough max
        int totalStats = ctx.player.stats.hp + ctx.player.stats.stamina +
                        ctx.player.stats.attack + ctx.player.stats.defense +
                        ctx.player.stats.weight + ctx.player.stats.craftSpeed;

        if (totalStats > maxStatPoints * 3) {
            flags.push_back(createFlag(ctx.player.uid,
                "Player level " + std::to_string(ctx.player.stats.level) +
                " has suspicious total stats: " + std::to_string(totalStats)));
        }
        return flags;
    }
};

} // namespace PSM
