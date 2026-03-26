#include "core/EventBus.h"

namespace PSM {

EventBus::EventBus() {}
EventBus::~EventBus() {}

EventBus::HandlerId EventBus::subscribe(const std::string& eventType, Handler handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    HandlerId id = m_nextId++;
    m_subscriptions.push_back({id, eventType, std::move(handler)});
    return id;
}

void EventBus::unsubscribe(HandlerId id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_subscriptions.erase(
        std::remove_if(m_subscriptions.begin(), m_subscriptions.end(),
            [id](const Subscription& s) { return s.id == id; }),
        m_subscriptions.end()
    );
}

void EventBus::publish(const std::string& eventType, std::any data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    Event evt;
    evt.type = eventType;
    evt.data = std::move(data);
    evt.timestamp = std::chrono::system_clock::now();

    for (auto& sub : m_subscriptions) {
        if (sub.eventType == eventType || sub.eventType == "*") {
            sub.handler(evt);
        }
    }
}

void EventBus::publishDeferred(const std::string& eventType, std::any data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    Event evt;
    evt.type = eventType;
    evt.data = std::move(data);
    evt.timestamp = std::chrono::system_clock::now();
    m_deferredQueue.push(std::move(evt));
}

void EventBus::processQueue() {
    std::queue<Event> toProcess;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::swap(toProcess, m_deferredQueue);
    }

    while (!toProcess.empty()) {
        Event evt = std::move(toProcess.front());
        toProcess.pop();

        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& sub : m_subscriptions) {
            if (sub.eventType == evt.type || sub.eventType == "*") {
                sub.handler(evt);
            }
        }
    }
}

} // namespace PSM