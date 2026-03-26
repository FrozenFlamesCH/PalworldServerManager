// ThemePanel.h
#pragma once
class Application;
namespace PSM {
class ThemePanel {
public:
    explicit ThemePanel(Application& app) : m_app(app) {}
    void render(); // Implemented in UIManager::renderThemes()
private:
    Application& m_app;
};
} // namespace PSM


