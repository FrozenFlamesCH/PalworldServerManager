#pragma once

#include <string>
#include <vector>
#include <functional>
#include "models/ScanResult.h"
#include "models/Player.h"
#include "models/Pal.h"
#include "models/Item.h"
#include "models/Guild.h"

namespace PSM {

struct ScanContext {
    Player player;
    std::vector<Pal> pals;
    std::vector<Item> inventory;
    std::vector<Guild> guilds;
    // Additional context
    int64_t totalPlaytime = 0;
};

class ScanRule {
public:
    ScanRule(const std::string& id, const std::string& name,
             ScanSeverity severity, const std::string& description);
    virtual ~ScanRule() = default;

    virtual std::vector<ScanFlag> evaluate(const ScanContext& context) = 0;

    const std::string& id() const { return m_id; }
    const std::string& name() const { return m_name; }
    ScanSeverity severity() const { return m_severity; }
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

protected:
    ScanFlag createFlag(const std::string& playerUid, const std::string& description) const;

    std::string m_id;
    std::string m_name;
    ScanSeverity m_severity;
    std::string m_description;
    bool m_enabled = true;
};

} // namespace PSM
