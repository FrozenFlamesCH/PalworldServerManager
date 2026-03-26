// WorldControlPanel.h
#pragma once
class Application;
namespace PSM {
class WorldControlPanel {
public:
    explicit WorldControlPanel(Application& app) : m_app(app) {}
    void render();
private:
    Application& m_app;
    int m_hour = 12;
    bool m_shutdownConfirm = false;
};
} // namespace PSM

cpp
