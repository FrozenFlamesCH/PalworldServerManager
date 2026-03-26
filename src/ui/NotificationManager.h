#pragma once

#include <string>
#include <vector>
#include <deque>
#include <imgui.h>

namespace PSM {

struct Notification {
    std::string title;
    std::string message;
    ImVec4 color;
    float duration = 5.0f;
    float elapsed = 0.0f;
    float alpha = 1.0f;
    bool dismissed = false;
};

class NotificationManager {
public:
    NotificationManager();
    ~NotificationManager();

    void push(const std::string& title, const std::string& message,
              const ImVec4& color = ImVec4(0.4f, 0.65f, 0.95f, 1.0f),
              float duration = 5.0f);

    void pushSuccess(const std::string& title, const std::string& message);
    void pushWarning(const std::string& title, const std::string& message);
    void pushError(const std::string& title, const std::string& message);
    void pushInfo(const std::string& title, const std::string& message);

    void render();
    void update(float deltaTime);

    void showDesktopNotification(const std::string& title, const std::string& message);
    void playAlertSound();
    void setSoundEnabled(bool enabled);

private:
    std::deque<Notification> m_notifications;
    int m_maxVisible = 5;
    bool m_soundEnabled = true;

    float m_fadeInTime = 0.3f;
    float m_fadeOutTime = 0.5f;
    float m_slideDistance = 30.0f;
};

} // namespace PSM