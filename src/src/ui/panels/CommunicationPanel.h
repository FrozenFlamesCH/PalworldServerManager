#pragma once
class Application;
namespace PSM {
class CommunicationPanel {
public:
    explicit CommunicationPanel(Application& app) : m_app(app) {}
    void render(); // Implemented in UIManager::renderCommunication()
private:
    Application& m_app;
    char m_broadcastBuf[512] = "";
    char m_alertBuf[512] = "";
    char m_msgTargetUid[128] = "";
    char m_msgBuf[512] = "";
    bool m_loginAnnounce = true;
    bool m_deathAnnounce = true;
    bool m_punishAnnounce = true;
};
} // namespace PSM
