// AppSettingsPanel.h
#pragma once
class Application;
namespace PSM {
class AppSettingsPanel {
public:
    explicit AppSettingsPanel(Application& app) : m_app(app) {}
    void render(); // Full implementation already in UIManager::renderAppSettings()
private:
    Application& m_app;
};
} // namespace PSM
