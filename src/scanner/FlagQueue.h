#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <optional>
#include "models/ScanResult.h"

namespace PSM {

class FlagQueue {
public:
    FlagQueue();
    ~FlagQueue();

    // Add flags to review queue
    void enqueue(const ScanFlag& flag);
    void enqueueMultiple(const std::vector<ScanFlag>& flags);

    // Review operations
    std::optional<ScanFlag> peek() const;
    std::optional<ScanFlag> dequeue();
    size_t size() const;
    bool empty() const;
    void clear();

    // Filtered views
    std::vector<ScanFlag> getByPlayer(const std::string& uid) const;
    std::vector<ScanFlag> getBySeverity(ScanSeverity minSeverity) const;
    std::vector<ScanFlag> getUndismissed() const;

    // Batch operations
    void dismissAll(const std::string& playerUid);
    void dismissByRule(const std::string& ruleId);

    // Sort
    enum class SortOrder {
        ScoreDesc,      // Highest score first
        TimestampDesc,  // Most recent first
        SeverityDesc,   // Most severe first
        PlayerAsc       // Alphabetical by player
    };

    std::vector<ScanFlag> getSorted(SortOrder order) const;

    // Callbacks
    using FlagCallback = std::function<void(const ScanFlag&)>;
    void onFlagAdded(FlagCallback callback);

private:
    mutable std::mutex m_mutex;
    std::deque<ScanFlag> m_queue;
    std::vector<FlagCallback> m_callbacks;
};

} // namespace PSM
