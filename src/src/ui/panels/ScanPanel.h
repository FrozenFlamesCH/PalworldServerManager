#pragma once

#include <string>
#include <vector>
#include "models/ScanResult.h"

class Application;

namespace PSM {

class ScanPanel {
public:
    explicit ScanPanel(Application& app);
    ~ScanPanel();

    void render();

private:
    void renderControls();
    void renderRules();
    void renderFlagQueue();
    void renderPlayerReport(const std::string& uid);
    void renderScanHistory();

    // Helper used in renderRules() and renderFlagQueue()
    static std::string severityToString(ScanSeverity sev) {
        switch (sev) {
            case ScanSeverity::Critical: return "Critical";
            case ScanSeverity::High:     return "High";
            case ScanSeverity::Medium:   return "Medium";
            case ScanSeverity::Low:      return "Low";
            default:                     return "Info";
        }
    }

    Application& m_app;

    std::string m_selectedReportUid;
    bool m_scanRunning = false;
    char m_manualScanUid[128] = "";
};

} // namespace PSM