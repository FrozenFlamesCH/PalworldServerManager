// ServerSettingsPanel.h
#pragma once
class Application;
namespace PSM {
class ServerSettingsPanel {
public:
    explicit ServerSettingsPanel(Application& app) : m_app(app) {}
    void render();
private:
    Application& m_app;
};
} // namespace PSM

cpp
