#include "scanner/FlagQueue.h"
#include <algorithm>

namespace PSM {

FlagQueue::FlagQueue() {}
FlagQueue::~FlagQueue() {}

void FlagQueue::enqueue(const ScanFlag& flag) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(flag);

    for (auto& cb : m_callbacks) {
        cb(flag);
    }
}

void FlagQueue::enqueueMultiple(const std::vector<ScanFlag>& flags) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& flag : flags) {
        m_queue.push_back(flag);
        for (auto& cb : m_callbacks) {
            cb(flag);
        }
    }
}

std::optional<ScanFlag> FlagQueue::peek() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) return std::nullopt;
    return m_queue.front();
}

std::optional<ScanFlag> FlagQueue::dequeue() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) return std::nullopt;
    ScanFlag flag = m_queue.front();
    m_queue.pop_front();
    return flag;
}

size_t FlagQueue::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

bool FlagQueue::empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

void FlagQueue::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.clear();
}

std::vector<ScanFlag> FlagQueue::getByPlayer(const std::string& uid) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ScanFlag> result;
    for (auto& flag : m_queue) {
        if (flag.playerUid == uid) result.push_back(flag);
    }
    return result;
}

std::vector<ScanFlag> FlagQueue::getBySeverity(ScanSeverity minSeverity) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ScanFlag> result;
    for (auto& flag : m_queue) {
        if (flag.severity >= minSeverity) result.push_back(flag);
    }
    return result;
}

std::vector<ScanFlag> FlagQueue::getUndismissed() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ScanFlag> result;
    for (auto& flag : m_queue) {
        if (!flag.dismissed) result.push_back(flag);
    }
    return result;
}

void FlagQueue::dismissAll(const std::string& playerUid) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& flag : m_queue) {
        if (flag.playerUid == playerUid) {
            flag.dismissed = true;
        }
    }
}

void FlagQueue::dismissByRule(const std::string& ruleId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& flag : m_queue) {
        if (flag.ruleId == ruleId) {
            flag.dismissed = true;
        }
    }
}

std::vector<ScanFlag> FlagQueue::getSorted(SortOrder order) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ScanFlag> sorted(m_queue.begin(), m_queue.end());

    switch (order) {
        case SortOrder::ScoreDesc:
            std::sort(sorted.begin(), sorted.end(),
                [](const ScanFlag& a, const ScanFlag& b) {
                    return a.scoreImpact > b.scoreImpact;
                });
            break;
        case SortOrder::TimestampDesc:
            std::sort(sorted.begin(), sorted.end(),
                [](const ScanFlag& a, const ScanFlag& b) {
                    return a.timestamp > b.timestamp;
                });
            break;
        case SortOrder::SeverityDesc:
            std::sort(sorted.begin(), sorted.end(),
                [](const ScanFlag& a, const ScanFlag& b) {
                    return a.severity > b.severity;
                });
            break;
        case SortOrder::PlayerAsc:
            std::sort(sorted.begin(), sorted.end(),
                [](const ScanFlag& a, const ScanFlag& b) {
                    return a.playerName < b.playerName;
                });
            break;
    }

    return sorted;
}

void FlagQueue::onFlagAdded(FlagCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callbacks.push_back(std::move(callback));
}

} // namespace PSM
