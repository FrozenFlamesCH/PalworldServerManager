#include "scanner/ScanRule.h"
#include <chrono>

namespace PSM {

ScanRule::ScanRule(const std::string& id, const std::string& name,
                   ScanSeverity severity, const std::string& description)
    : m_id(id), m_name(name), m_severity(severity), m_description(description) {}

ScanFlag ScanRule::createFlag(const std::string& playerUid, const std::string& description) const {
    ScanFlag flag;
    flag.playerUid = playerUid;
    flag.ruleId = m_id;
    flag.ruleName = m_name;
    flag.severity = m_severity;
    flag.description = description;
    flag.scoreImpact = severityToScore(m_severity);
    flag.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return flag;
}

} // namespace PSM
