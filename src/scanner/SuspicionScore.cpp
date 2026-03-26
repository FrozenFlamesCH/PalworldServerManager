#include "scanner/SuspicionScore.h"
#include <algorithm>
#include <cmath>
#include <chrono>

namespace PSM {

SuspicionScoreCalculator::SuspicionScoreCalculator() {
    m_severityWeights = {
        {ScanSeverity::Info, 1.0f},
        {ScanSeverity::Low, 5.0f},
        {ScanSeverity::Medium, 15.0f},
        {ScanSeverity::High, 30.0f},
        {ScanSeverity::Critical, 50.0f}
    };
}

SuspicionScoreCalculator::~SuspicionScoreCalculator() {}

void SuspicionScoreCalculator::setSeverityWeight(ScanSeverity severity, float weight) {
    m_severityWeights[severity] = weight;
}

void SuspicionScoreCalculator::setDecayRate(float decayPerDay) {
    m_decayRate = decayPerDay;
}

void SuspicionScoreCalculator::setRecencyBoost(float boostFactor) {
    m_recencyBoost = boostFactor;
}

int SuspicionScoreCalculator::calculateScore(const std::vector<ScanFlag>& flags) const {
    float total = 0.0f;
    for (const auto& flag : flags) {
        if (flag.dismissed) continue;
        auto it = m_severityWeights.find(flag.severity);
        float weight = (it != m_severityWeights.end()) ? it->second : 1.0f;
        total += weight;
    }
    return static_cast<int>(total);
}

int SuspicionScoreCalculator::calculateScoreWithDecay(const std::vector<ScanFlag>& flags,
                                                        int64_t currentTime) const {
    if (currentTime == 0) {
        currentTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    float total = 0.0f;
    int64_t recencyThreshold = currentTime - (m_recencyWindowHours * 3600);

    for (const auto& flag : flags) {
        if (flag.dismissed) continue;

        auto it = m_severityWeights.find(flag.severity);
        float baseWeight = (it != m_severityWeights.end()) ? it->second : 1.0f;

        // Calculate age in days
        float ageDays = static_cast<float>(currentTime - flag.timestamp) / 86400.0f;
        if (ageDays < 0) ageDays = 0;

        // Apply exponential decay
        float decayMultiplier = std::exp(-m_decayRate * ageDays);
        decayMultiplier = std::max(decayMultiplier, 0.1f); // minimum 10% of original

        // Apply recency boost
        float recencyMultiplier = (flag.timestamp >= recencyThreshold) ? m_recencyBoost : 1.0f;

        total += baseWeight * decayMultiplier * recencyMultiplier;
    }

    return static_cast<int>(total);
}

bool SuspicionScoreCalculator::isAboveThreshold(int score, int threshold) const {
    return score >= threshold;
}

std::string SuspicionScoreCalculator::getRecommendedAction(int score) const {
    if (score >= 150) return "BAN - Extremely suspicious activity";
    if (score >= 100) return "KICK + INVESTIGATE - Highly suspicious";
    if (score >= 50) return "WARN + MONITOR - Moderately suspicious";
    if (score >= 20) return "MONITOR - Some flags detected";
    if (score >= 5) return "NOTE - Minor anomalies";
    return "CLEAN - No action needed";
}

SuspicionScoreCalculator::ScoreBreakdown
SuspicionScoreCalculator::getBreakdown(const std::vector<ScanFlag>& flags, int64_t currentTime) const {
    ScoreBreakdown breakdown;

    if (currentTime == 0) {
        currentTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    ScanSeverity highestSeverity = ScanSeverity::Info;

    for (const auto& flag : flags) {
        if (flag.dismissed) continue;

        switch (flag.severity) {
            case ScanSeverity::Critical: breakdown.criticalCount++; break;
            case ScanSeverity::High: breakdown.highCount++; break;
            case ScanSeverity::Medium: breakdown.mediumCount++; break;
            case ScanSeverity::Low: breakdown.lowCount++; break;
            case ScanSeverity::Info: breakdown.infoCount++; break;
        }

        if (flag.severity > highestSeverity) {
            highestSeverity = flag.severity;
            breakdown.highestSeverityRule = flag.ruleId;
        }

        if (flag.timestamp > breakdown.mostRecentFlag) {
            breakdown.mostRecentFlag = flag.timestamp;
        }
    }

    breakdown.totalScore = calculateScoreWithDecay(flags, currentTime);

    // Calculate overall decay factor
    if (breakdown.mostRecentFlag > 0) {
        float ageDays = static_cast<float>(currentTime - breakdown.mostRecentFlag) / 86400.0f;
        breakdown.decayFactor = std::exp(-m_decayRate * ageDays);
        breakdown.decayFactor = std::max(breakdown.decayFactor, 0.1f);
    }

    return breakdown;
}

} // namespace PSM
