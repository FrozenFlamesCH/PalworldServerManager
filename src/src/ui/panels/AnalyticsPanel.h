// AnalyticsPanel.h
#pragma once
class Application;
namespace PSM {
class AnalyticsPanel {
public:
    explicit AnalyticsPanel(Application& app) : m_app(app) {}
    void render();
private:
    Application& m_app;
    int m_selectedCategory = 0;
};
} // namespace PSM

cpp
