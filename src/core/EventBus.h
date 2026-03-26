#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <any>
#include <typeindex>

namespace PSM {

struct Event {
    std::string type;
    std::any data;
    std::chrono::system_clock::time_point timestamp;
};

class EventBus {
public:
    using Handler = std::function<void(const Event&)>;
    using HandlerId = size_t;

    EventBus();
    ~EventBus();

    HandlerId subscribe(const std::string& eventType, Handler handler);
    void unsubscribe(HandlerId id);

    void publish(const std::string& eventType, std::any data = {});
    void publishDeferred(const std::string& eventType, std::any data = {});
    void processQueue();

    // Common event types
    static constexpr const char* EVT_SERVER_STARTED = "server.started";
    static constexpr const char* EVT_SERVER_STOPPED = "server.stopped";
    static constexpr const char* EVT_SERVER_CRASHED = "server.crashed";
    static constexpr const char* EVT_PLAYER_JOINED = "player.joined";
    static constexpr const char* EVT_PLAYER_LEFT = "player.left";
    static constexpr const char* EVT_PLAYER_KICKED = "player.kicked";
    static constexpr const char* EVT_PLAYER_BANNED = "player.banned";
    static constexpr const char* EVT_SCAN_COMPLETE = "scan.complete";
    static constexpr const char* EVT_SCAN_FLAG = "scan.flag";
    static constexpr const char* EVT_BACKUP_COMPLETE = "backup.complete";
    static constexpr const char* EVT_CHEAT_DETECTED = "cheat.detected";
    static constexpr const char* EVT_PST_SYNCED = "pst.synced";
    static constexpr const char* EVT_CONFIG_CHANGED = "config.changed";

private:
    struct Subscription {
        HandlerId id;
        std::string eventType;
        Handler handler;
    };

    std::mutex m_mutex;
    std::vector<Subscription> m_subscriptions;
    std::queue<Event> m_deferredQueue;
    HandlerId m_nextId = 1;
};

} // namespace PSM