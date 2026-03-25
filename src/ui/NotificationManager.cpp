#include "ui/NotificationManager.h"
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "user32.lib")
#endif

namespace PSM {

NotificationManager::NotificationManager() {}
NotificationManager::~NotificationManager() {}

void NotificationManager::push(const std::string& title, const std::string& message,
                                const ImVec4& color, float duration) {
    Notification n;
    n.title = title;
    n.message = message;
    n.color = color;
    n.duration = duration;
    n.elapsed = 0.0f;
    m_notifications.push_front(n);

    // Limit total notifications
    while (m_notifications.size() > 20) {
        m_notifications.pop_back();
    }
}

void NotificationManager::pushSuccess(const std::string& title, const std::string& message) {
    push(title, message, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
}

void NotificationManager::pushWarning(const std::string& title, const std::string& message) {
    push(title, message, ImVec4(0.95f, 0.75f, 0.2f, 1.0f));
}

void NotificationManager::pushError(const std::string& title, const std::string& message) {
    push(title, message, ImVec4(0.9f, 0.3f, 0.25f, 1.0f));
    if (m_soundEnabled) playAlertSound();
}

void NotificationManager::pushInfo(const std::string& title, const std::string& message) {
    push(title, message, ImVec4(0.4f, 0.65f, 0.95f, 1.0f));
}

void NotificationManager::update(float deltaTime) {
    for (auto& n : m_notifications) {
        n.elapsed += deltaTime;

        // Calculate alpha
        if (n.elapsed < m_fadeInTime) {
            n.alpha = n.elapsed / m_fadeInTime;
        } else if (n.elapsed > n.duration - m_fadeOutTime) {
            float fadeProgress = (n.elapsed - (n.duration - m_fadeOutTime)) / m_fadeOutTime;
            n.alpha = 1.0f - std::clamp(fadeProgress, 0.0f, 1.0f);
        } else {
            n.alpha = 1.0f;
        }

        if (n.elapsed >= n.duration) {
            n.dismissed = true;
        }
    }

    // Remove dismissed notifications
    m_notifications.erase(
        std::remove_if(m_notifications.begin(), m_notifications.end(),
            [](const Notification& n) { return n.dismissed; }),
        m_notifications.end());
}

void NotificationManager::render() {
    update(ImGui::GetIO().DeltaTime);

    if (m_notifications.empty()) return;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float notifWidth = 350.0f;
    float notifHeight = 65.0f;
    float padding = 10.0f;
    float startX = viewport->WorkPos.x + viewport->WorkSize.x - notifWidth - padding;
    float startY = viewport->WorkPos.y + viewport->WorkSize.y - padding;

    int displayed = 0;
    for (auto& n : m_notifications) {
        if (displayed >= m_maxVisible) break;

        float yOffset = startY - (displayed + 1) * (notifHeight + 6);

        // Slide animation
        float slideOffset = 0;
        if (n.elapsed < m_fadeInTime) {
            slideOffset = (1.0f - n.elapsed / m_fadeInTime) * m_slideDistance;
        }

        ImVec2 pos(startX + slideOffset, yOffset);
        ImVec2 size(notifWidth, notifHeight);

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);

        char windowId[64];
        snprintf(windowId, sizeof(windowId), "##Notif_%p", &n);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, n.alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.13f, 0.15f, 0.95f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize;

        if (ImGui::Begin(windowId, nullptr, flags)) {
            // Left color bar
            ImVec2 wPos = ImGui::GetWindowPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                wPos, ImVec2(wPos.x + 4, wPos.y + notifHeight),
                ImGui::ColorConvertFloat4ToU32(n.color), 6.0f);

            ImGui::SetCursorPosX(14);
            ImGui::TextColored(n.color, "%s", n.title.c_str());

            ImGui::SetCursorPosX(14);
            ImGui::TextWrapped("%s", n.message.c_str());

            // Close button
            ImGui::SetCursorPos(ImVec2(notifWidth - 25, 5));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            if (ImGui::SmallButton("×")) {
                n.dismissed = true;
            }
            ImGui::PopStyleColor();
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);

        displayed++;
    }
}

void NotificationManager::showDesktopNotification(const std::string& title,
                                                    const std::string& message) {
#ifdef _WIN32
    // Windows toast notification (simplified)
    // In production, use WinRT or Shell_NotifyIcon for proper toasts
    MessageBoxA(nullptr, message.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
#elif defined(__linux__)
    // Linux: use notify-send
    std::string cmd = "notify-send '" + title + "' '" + message + "'";
    system(cmd.c_str());
#elif defined(__APPLE__)
    // macOS: use osascript
    std::string cmd = "osascript -e 'display notification \"" + message +
                      "\" with title \"" + title + "\"'";
    system(cmd.c_str());
#endif
}

void NotificationManager::playAlertSound() {
#ifdef _WIN32
    MessageBeep(MB_ICONHAND);
#elif defined(__linux__)
    system("paplay /usr/share/sounds/freedesktop/stereo/alarm-clock-elapsed.oga &");
#elif defined(__APPLE__)
    system("afplay /System/Library/Sounds/Glass.aiff &");
#endif
}

void NotificationManager::setSoundEnabled(bool enabled) {
    m_soundEnabled = enabled;
}

} // namespace PSM

Multi-Server Support Stub
cpp

Copy code
// src/server/MultiServerManager.h
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace PSM {

class VanillaAPI;
class PalDefenderAPI;
class PalDefenderRCON;
class ServerManager;

struct ServerProfile {
    std::string id;
    std::string name;
    std::string vanillaHost;
    int vanillaPort = 8212;
    std::string vanillaPassword;
    std::string pdHost;
    int pdPort = 8214;
    std::string pdApiKey;
    std::string rconHost;
    int rconPort = 25575;
    std::string rconPassword;
    std::string exePath;
    std::string savePath;
    bool isActive = false;
};

class MultiServerManager {
public:
    MultiServerManager();
    ~MultiServerManager();

    void addServer(const ServerProfile& profile);
    void removeServer(const std::string& id);
    void setActiveServer(const std::string& id);

    ServerProfile* getActiveServer();
    const std::vector<ServerProfile>& getAllServers() const;
    ServerProfile* getServer(const std::string& id);

    // Convenience
    bool switchTo(const std::string& id);
    std::string activeServerId() const;

    // Serialization
    void loadFromJson(const nlohmann::json& data);
    nlohmann::json toJson() const;

private:
    std::vector<ServerProfile> m_servers;
    std::string m_activeServerId;
};

} // namespace PSM
