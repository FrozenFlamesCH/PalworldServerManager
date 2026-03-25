#pragma once

#include "scanner/ScanRule.h"
#include <set>

namespace PSM {

// Rule: Pal with stats exceeding limits
class IllegalPalStatsRule : public ScanRule {
public:
    IllegalPalStatsRule()
        : ScanRule("PAL_ILLEGAL_STATS", "Illegal Pal Stats",
                   ScanSeverity::High, "Pal has stats exceeding normal limits") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        for (auto& pal : ctx.pals) {
            if (pal.rank > 4) {
                flags.push_back(createFlag(ctx.player.uid,
                    "Pal " + pal.characterId + " has rank " + std::to_string(pal.rank) +
                    " (max: 4)"));
            }
            if (pal.level > 55) {
                flags.push_back(createFlag(ctx.player.uid,
                    "Pal " + pal.characterId + " has level " + std::to_string(pal.level) +
                    " (max: 55)"));
            }
        }
        return flags;
    }
};

// Rule: Impossible passive combinations
class ImpossiblePassivesRule : public ScanRule {
public:
    ImpossiblePassivesRule()
        : ScanRule("PAL_IMPOSSIBLE_PASSIVES", "Impossible Passive Combination",
                   ScanSeverity::Medium, "Pal has impossible passive skill combination") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        for (auto& pal : ctx.pals) {
            if (pal.passives.size() > 4) {
                flags.push_back(createFlag(ctx.player.uid,
                    "Pal " + pal.characterId + " has " + std::to_string(pal.passives.size()) +
                    " passives (max: 4)"));
            }

            // Check for mutually exclusive passives
            std::set<std::string> passiveIds;
            for (auto& p : pal.passives) {
                if (passiveIds.count(p.id)) {
                    flags.push_back(createFlag(ctx.player.uid,
                        "Pal " + pal.characterId + " has duplicate passive: " + p.name));
                }
                passiveIds.insert(p.id);
            }
        }
        return flags;
    }
};

// Rule: Duplicated pal UUIDs (cloned pals)
class DuplicatePalUUIDRule : public ScanRule {
public:
    DuplicatePalUUIDRule()
        : ScanRule("PAL_DUPLICATE_UUID", "Cloned Pal Detected",
                   ScanSeverity::Critical, "Same pal UUID appears multiple times") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        std::set<std::string> seenIds;
        for (auto& pal : ctx.pals) {
            if (!pal.instanceId.empty() && seenIds.count(pal.instanceId)) {
                flags.push_back(createFlag(ctx.player.uid,
                    "Duplicate pal UUID: " + pal.instanceId +
                    " (" + pal.characterId + ")"));
            }
            seenIds.insert(pal.instanceId);
        }
        return flags;
    }
};

} // namespace PSM
