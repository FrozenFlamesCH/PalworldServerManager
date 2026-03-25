#pragma once

#include "scanner/ScanRule.h"

namespace PSM {

// Rule: Base with abnormal pal count
class AbnormalBasePalCountRule : public ScanRule {
public:
    AbnormalBasePalCountRule()
        : ScanRule("GUILD_ABNORMAL_PAL_COUNT", "Abnormal Base Pal Count",
                   ScanSeverity::Medium, "Base has abnormally high pal count") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        for (auto& guild : ctx.guilds) {
            for (auto& base : guild.bases) {
                if (base.palCount > 50) { // configurable threshold
                    flags.push_back(createFlag(ctx.player.uid,
                        "Guild '" + guild.name + "' base has " +
                        std::to_string(base.palCount) + " pals"));
                }
            }
        }
        return flags;
    }
};

} // namespace PSM
