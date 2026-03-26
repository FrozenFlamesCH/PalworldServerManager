#pragma once

#include "scanner/ScanRule.h"

namespace PSM {

// Rule: Low level player with endgame items
class LowLevelEndgameItemRule : public ScanRule {
public:
    LowLevelEndgameItemRule()
        : ScanRule("INV_LOW_LEVEL_ENDGAME", "Low Level Endgame Items",
                   ScanSeverity::High, "Player below level 10 owns endgame weapon") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        if (ctx.player.stats.level >= 10) return flags;

        // List of endgame item IDs (would be configurable)
        static const std::vector<std::string> endgameItems = {
            "AssaultRifle", "RocketLauncher", "MissileGuidedRifle",
            "LaserRifle", "GatlingGun", "GrenadeLauncher"
        };

        for (auto& item : ctx.inventory) {
            for (auto& eg : endgameItems) {
                if (item.staticId.find(eg) != std::string::npos) {
                    flags.push_back(createFlag(ctx.player.uid,
                        "Level " + std::to_string(ctx.player.stats.level) +
                        " player has endgame item: " + item.name));
                }
            }
        }
        return flags;
    }
};

// Rule: Impossible stack counts
class ImpossibleStackRule : public ScanRule {
public:
    ImpossibleStackRule()
        : ScanRule("INV_IMPOSSIBLE_STACK", "Impossible Stack Count",
                   ScanSeverity::High, "Item has impossible stack count") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        for (auto& item : ctx.inventory) {
            if (item.maxStack > 0 && item.count > item.maxStack * 2) {
                flags.push_back(createFlag(ctx.player.uid,
                    "Item " + item.name + " has count " + std::to_string(item.count) +
                    " (max: " + std::to_string(item.maxStack) + ")"));
            }
        }
        return flags;
    }
};

// Rule: Unobtainable / debug items
class DebugItemRule : public ScanRule {
public:
    DebugItemRule()
        : ScanRule("INV_DEBUG_ITEM", "Unobtainable Item Detected",
                   ScanSeverity::Critical, "Player has debug/unobtainable items") {}

    std::vector<ScanFlag> evaluate(const ScanContext& ctx) override {
        std::vector<ScanFlag> flags;
        static const std::vector<std::string> debugItems = {
            "Debug_", "Test_", "GM_", "Admin_", "DevTool"
        };

        for (auto& item : ctx.inventory) {
            for (auto& prefix : debugItems) {
                if (item.staticId.find(prefix) == 0) {
                    flags.push_back(createFlag(ctx.player.uid,
                        "Debug/unobtainable item: " + item.name + " (" + item.staticId + ")"));
                }
            }
        }
        return flags;
    }
};

} // namespace PSM
