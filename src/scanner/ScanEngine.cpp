#include "scanner/ScanEngine.h"
#include "pst/LevelSavParser.h"
#include "net/PalDefenderAPI.h"
#include "net/PalDefenderRCON.h"
#include "core/Database.h"
#include "core/Config.h"
#include "core/EventBus.h"

// Include all rule implementations
#include "scanner/rules/InventoryScanRules.h"
#include "scanner/rules/PalScanRules.h"
#include "scanner/rules/TechScanRules.h"
#include "scanner/rules/StatScanRules.h"
#include "scanner/rules/GuildScanRules.h"

#include <chrono>
#include <iostream>

namespace PSM {

ScanEngine::ScanEngine(LevelSavParser& pst, PalDefenderAPI& pdApi, PalDefenderRCON& pdRcon,
                       Database& db, Config& config, EventBus& eventBus)
    : m_pst(pst), m_pdApi(pdApi), m_pdRcon(pdRcon),
      m_db(db), m_config(config), m_eventBus(eventBus) {
    registerDefaultRules();
}

ScanEngine::~ScanEngine() {
    stopScheduler();
}

void ScanEngine::registerRule(std::shared_ptr<ScanRule> rule) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rules.push_back(std::move(rule));
}

void ScanEngine::registerDefaultRules() {
    registerRule(std::make_shared<LowLevelEndgameItemRule>());
    registerRule(std::make_shared<ImpossibleStackRule>());
    registerRule(std::make_shared<DebugItemRule>());
    registerRule(std::make_shared<IllegalPalStatsRule>());
    registerRule(std::make_shared<ImpossiblePassivesRule>());
    registerRule(std::make_shared<DuplicatePalUUIDRule>());
    registerRule(std::make_shared<TechBeyondLevelRule>());
    registerRule(std::make_shared<StatsExceedingCapRule>());
    registerRule(std::make_shared<AbnormalBasePalCountRule>());
}

ScanContext ScanEngine::buildContext(const std::string& uid) {
    ScanContext ctx;
    ctx.player = m_pst.getPlayer(uid);
    ctx.pals = m_pst.getPlayerPals(uid);
    ctx.inventory = m_pst.getPlayerInventory(uid);
    ctx.guilds = m_pst.getGuilds();

    // Get playtime from database
    auto sessions = m_db.getPlayerSessions(uid);
    for (auto& s : sessions) {
        if (s.contains("duration_seconds")) {
            try { ctx.totalPlaytime += std::stoll(s["duration_seconds"].get<std::string>()); }
            catch (...) {}
        }
    }

    return ctx;
}

std::vector<ScanFlag> ScanEngine::scanPlayer(const std::string& uid) {
    if (m_db.isScanWhitelisted(uid)) return {};

    ScanContext ctx = buildContext(uid);
    std::vector<ScanFlag> allFlags;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& rule : m_rules) {
        if (!rule->isEnabled()) continue;

        auto flags = rule->evaluate(ctx);
        for (auto& flag : flags) {
            flag.playerName = ctx.player.name;
            m_db.recordScanResult(flag.playerUid, flag.ruleId,
                severityToString(flag.severity), flag.description,
                flag.scoreImpact, flag.timestamp);
            allFlags.push_back(flag);
        }
    }

    if (!allFlags.empty()) {
        m_eventBus.publishDeferred(EventBus::EVT_SCAN_FLAG, uid);
    }

    return allFlags;
}

std::vector<ScanFlag> ScanEngine::scanAllPlayers() {
    auto players = m_pst.getAllPlayers();
    std::vector<ScanFlag> allFlags;

    for (auto& player : players) {
        auto flags = scanPlayer(player.uid);
        allFlags.insert(allFlags.end(), flags.begin(), flags.end());
    }

    m_eventBus.publishDeferred(EventBus::EVT_SCAN_COMPLETE);
    return allFlags;
}

void ScanEngine::startScheduler() {
    if (m_running) return;
    m_running = true;
    m_schedulerThread = std::thread(&ScanEngine::schedulerThread, this);
}

void ScanEngine::stopScheduler() {
    m_running = false;
    if (m_schedulerThread.joinable()) {
        m_schedulerThread.join();
    }
}

void ScanEngine::schedulerThread() {
    while (m_running) {
        int intervalMinutes = m_config.getInt("scan.intervalMinutes", 30);

        for (int i = 0; i < intervalMinutes * 60 * 10 && m_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (m_running) {
            auto flags = scanAllPlayers();
            if (m_config.getBool("scan.autoAction", false)) {
                processAutoActions(flags);
            }
        }
    }
}

void ScanEngine::processAutoActions(const std::vector<ScanFlag>& flags) {
    int threshold = m_config.getInt("scan.suspicionThreshold", 50);

    // Group flags by player
    std::map<std::string, int> playerScores;
    for (auto& flag : flags) {
        playerScores[flag.playerUid] += flag.scoreImpact;
    }

    for (auto& [uid, score] : playerScores) {
        if (score >= threshold) {
            // Auto-action: warn, kick, or ban based on severity
            if (score >= threshold * 3) {
                m_pdRcon.banPlayer(uid, "Auto-ban: Cheat detection score " + std::to_string(score));
            } else if (score >= threshold * 2) {
                m_pdRcon.kickPlayer(uid, "Auto-kick: Suspicious activity detected");
            } else {
                m_pdRcon.sendMessage(uid, "Warning: Suspicious activity detected on your account.");
            }
        }
    }
}

std::vector<ScanFlag> ScanEngine::getFlaggedPlayers() {
    auto history = m_db.getScanHistory("", 200);
    std::vector<ScanFlag> flags;
    for (auto& row : history) {
        ScanFlag flag;
        flag.id = std::stoi(row.value("id", "0"));
        flag.playerUid = row.value("player_uid", "");
        flag.ruleId = row.value("rule_id", "");
        flag.description = row.value("description", "");
        flag.scoreImpact = std::stoi(row.value("score_impact", "0"));
        flag.timestamp = std::stoll(row.value("timestamp", "0"));
        flag.dismissed = row.value("dismissed", "0") == "1";
        flag.actionTaken = row.value("action_taken", "");
        flags.push_back(flag);
    }
    return flags;
}

std::vector<PlayerScanReport> ScanEngine::getReports() {
    auto flags = getFlaggedPlayers();
    std::map<std::string, PlayerScanReport> reportMap;

    for (auto& flag : flags) {
        auto& report = reportMap[flag.playerUid];
        report.playerUid = flag.playerUid;
        report.playerName = flag.playerName;
        report.totalScore += flag.scoreImpact;
        report.flags.push_back(flag);
    }

    std::vector<PlayerScanReport> reports;
    for (auto& [uid, report] : reportMap) {
        reports.push_back(report);
    }
    return reports;
}

void ScanEngine::dismissFlag(int flagId) {
    m_db.dismissScanFlag(flagId);
}

void ScanEngine::actionFlag(int flagId, const std::string& action) {
    m_db.setFlagActioned(flagId, action);
}

const std::vector<std::shared_ptr<ScanRule>>& ScanEngine::getRules() const {
    return m_rules;
}

} // namespace PSM
