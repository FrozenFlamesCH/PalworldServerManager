// ResearchPanel.h
#pragma once
class Application;
namespace PSM {
class ResearchPanel {
public:
    explicit ResearchPanel(Application& app) : m_app(app) {}
    void render();
private:
    Application& m_app;
    char m_uid[128] = "";
    char m_techId[128] = "";
    int m_techPts = 0;
    int m_ancientPts = 0;
};
} // namespace PSM

cpp
