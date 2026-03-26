#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace PSM {

enum class ScanSeverity {
    Info,
    Low,
    Medium,
    High,
    Critical
};

struct ScanFlag {
    int id = 0;
    std::string playerUid;
    std::string playerName;
    std::string ruleId;
    std::string ruleName;
    ScanSeverity severity = ScanSeverity::Low;
    std::string description;
    int scoreImpact = 0;
    int64_t timestamp = 0;
    bool dismissed = false;
    std::string actionTaken;
};

struct PlayerScanReport {
    std::string playerUid;
    std::string playerName;
    int totalScore = 0;
    std::vector<ScanFlag> flags;
    int64_t lastScanned = 0;
};

inline std::string severityToString(ScanSeverity s) {
    switch (s) {
        case ScanSeverity::Info: return "Info";
        case ScanSeverity::Low: return "Low";
        case ScanSeverity::Medium: return "Medium";
        case ScanSeverity::High: return "High";
        case ScanSeverity::Critical: return "Critical";
    }
    return "Unknown";
}

inline int severityToScore(ScanSeverity s) {
    switch (s) {
        case ScanSeverity::Info: return 1;
        case ScanSeverity::Low: return 5;
        case ScanSeverity::Medium: return 15;
        case ScanSeverity::High: return 30;
        case ScanSeverity::Critical: return 50;
    }
    return 0;
}

} // namespace PSM
