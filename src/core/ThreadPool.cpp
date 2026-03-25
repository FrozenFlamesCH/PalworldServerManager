#include "core/ThreadPool.h"

namespace PSM {

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_condition.wait(lock, [this] {
                        return m_stop || !m_tasks.empty();
                    });
                    if (m_stop && m_tasks.empty()) return;
                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                    ++m_activeTasks;
                }
                task();
                --m_activeTasks;
                m_allDone.notify_all();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_condition.notify_all();
    for (auto& w : m_workers) {
        if (w.joinable()) w.join();
    }
}

size_t ThreadPool::pendingTasks() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tasks.size() + m_activeTasks;
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_allDone.wait(lock, [this] {
        return m_tasks.empty() && m_activeTasks == 0;
    });
}

} // namespace PSM