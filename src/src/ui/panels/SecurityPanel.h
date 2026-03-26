// SecurityPanel.h
#pragma once
class Application;
namespace PSM {
class SecurityPanel {
public:
    explicit SecurityPanel(Application& app) : m_app(app) {}
    void render();
private:
    Application& m_app;
    bool m_autoKick = false, m_whitelist = false, m_illegalItem = false;
    bool m_illegalPal = false, m_towerBlock = false, m_steamDupe = false;
    char m_ipBuf[64] = "";
};
} // namespace PSM

cpp
