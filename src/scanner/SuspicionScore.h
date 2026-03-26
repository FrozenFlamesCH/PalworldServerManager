#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "models/ScanResult.h"

namespace PSM {

class SuspicionScoreCalculator {
public:
    SuspicionScoreCalculator();
    ~SuspicionScoreCalculator();

    // Configure weights
    void setSeverityWeight(ScanSeverity severity, float weight);
    void setDecayRate(float decayPerDay);
    void setRecencyBoost(float boostFactor);

    // Calculate
    int calculateScore(const std::vector<ScanFlag>& flags) const;
    int calculateScoreWithDecay(const std::vector<ScanFlag>& flags, int64_t currentTime) const;

    // Threshold checks
    bool isAboveThreshold(int score, int threshold) const;
    std::string getRecommendedAction(int score) const;

    // Score breakdown
    struct ScoreBreakdown {
        int totalScore = 0;
        int criticalCount = 0;
        int highCount = 0;
        int mediumCount = 0;
        int lowCount = 0;
        int infoCount = 0;
        std::string highestSeverityRule;
        int64_t mostRecentFlag = 0;
        float decayFactor = 1.0f;
    };

    ScoreBreakdown getBreakdown(const std::vector<ScanFlag>& flags, int64_t currentTime = 0) const;

private:
    std::map<ScanSeverity, float> m_severityWeights;
    float m_decayRate = 0.1f;       // Score decay per day (10%)
    float m_recencyBoost = 1.5f;    // Recent flags weighted more heavily
    int m_recencyWindowHours = 24;  // "Recent" means within 24 hours
};

} // namespace PSM
