#include "ui/panels/ScanPanel.h"
#include "Application.h"
#include "scanner/ScanEngine.h"
#include "core/Database.h"
#include "core/ThreadPool.h"
#include "net/PalDefenderRCON.h"
#include "ui/Widgets.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>

namespace PSM {

ScanPanel::ScanPanel(Application& app) : m_app(app) {}
ScanPanel::~ScanPanel() {}

void ScanPanel::render() {
    ImGui::Text("🆕 Custom Scan Rules");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##ScanTabs")) {
        if (ImGui::BeginTabItem("Controls")) {
            renderControls();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Rules")) {
            renderRules();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Flag Queue")) {
            renderFlagQueue();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("History")) {
            renderScanHistory();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void ScanPanel::renderControls() {
    // Full scan
    if (m_scanRunning) {
        ImGui::BeginDisabled();
        ImGui::Button("⏳ Scanning...", ImVec2(200, 35));
        ImGui::EndDisabled();
    } else {
        if (ImGui::Button("🔍 Run Full Scan", ImVec2(200, 35))) {
            m_scanRunning = true;
            m_app.threadPool().enqueue([this] {
                m_app.scanEngine().scanAllPlayers();
                m_scanRunning = false;
            });
        }
    }

    ImGui::Spacing();

    // Manual single player scan
    ImGui::InputText("Player UID", m_manualScanUid, sizeof(m_manualScanUid));
    ImGui::SameLine();
    if (ImGui::Button("Scan Player") && strlen(m_manualScanUid) > 0) {
        m_app.threadPool().enqueue([this] {
            std::string uid(m_manualScanUid);
            m_app.scanEngine().scanPlayer(uid);
        });
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Scan Whitelist (Trusted)");

    auto whitelist = m_app.database().getScanWhitelist();
    for (auto& uid : whitelist) {
        ImGui::PushID(uid.c_str());
        ImGui::Text("%s", uid.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("Remove")) {
            m_app.database().removeScanWhitelist(uid);
        }
        ImGui::PopID();
    }

    static char wlUid[128] = "";
    ImGui::InputText("##wlAdd", wlUid, sizeof(wlUid));
    ImGui::SameLine();
    if (ImGui::Button("Add to Whitelist") && strlen(wlUid) > 0) {
        m_app.database().addScanWhitelist(wlUid);
        wlUid[0] = '\0';
    }
}

void ScanPanel::renderRules() {
    auto& rules = m_app.scanEngine().getRules();

    if (ImGui::BeginTable("##RuleTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Rule");
        ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("ID");
        ImGui::TableHeadersRow();

        for (auto& rule : rules) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            bool enabled = rule->isEnabled();
            ImGui::PushID(rule->id().c_str());
            if (ImGui::Checkbox("##en", &enabled)) {
                rule->setEnabled(enabled);
            }
            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::Text("%s", rule->name().c_str());

            ImGui::TableNextColumn();
            std::string sev = severityToString(rule->severity());
            ImVec4 sevColor;
            switch (rule->severity()) {
                case ScanSeverity::Critical: sevColor = ImVec4(0.9f, 0.15f, 0.15f, 1); break;
                case ScanSeverity::High: sevColor = ImVec4(0.9f, 0.4f, 0.2f, 1); break;
                case ScanSeverity::Medium: sevColor = ImVec4(0.9f, 0.7f, 0.2f, 1); break;
                case ScanSeverity::Low: sevColor = ImVec4(0.5f, 0.5f, 0.55f, 1); break;
                default: sevColor = ImVec4(0.4f, 0.4f, 0.45f, 1); break;
            }
            ImGui::TextColored(sevColor, "%s", sev.c_str());

            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "%s", rule->id().c_str());
        }
        ImGui::EndTable();
    }
}

void ScanPanel::renderFlagQueue() {
    auto reports = m_app.scanEngine().getReports();
    std::sort(reports.begin(), reports.end(),
        [](const PlayerScanReport& a, const PlayerScanReport& b) {
            return a.totalScore > b.totalScore;
        });

    // Summary
    int totalFlags = 0;
    int criticalPlayers = 0;
    for (auto& r : reports) {
        totalFlags += static_cast<int>(r.flags.size());
        if (r.totalScore >= 50) criticalPlayers++;
    }
    ImGui::Text("Flagged Players: %zu | Total Flags: %d | Critical: %d",
        reports.size(), totalFlags, criticalPlayers);
    ImGui::Spacing();

    // Main table + detail
    float tableWidth = m_selectedReportUid.empty() ?
        ImGui::GetContentRegionAvail().x : ImGui::GetContentRegionAvail().x - 350;

    ImGui::BeginChild("##FlagTable", ImVec2(tableWidth, 0));

    if (ImGui::BeginTable("##Flags", 5,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("Player");
        ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("Highest", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableHeadersRow();

        for (auto& report : reports) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable(report.playerName.c_str(),
                m_selectedReportUid == report.playerUid,
                ImGuiSelectableFlags_SpanAllColumns)) {
                m_selectedReportUid = report.playerUid;
            }

            ImGui::TableNextColumn();
            ImVec4 sc = report.totalScore >= 100 ? ImVec4(0.9f, 0.15f, 0.15f, 1) :
                       (report.totalScore >= 50 ? ImVec4(0.9f, 0.4f, 0.2f, 1) :
                        ImVec4(0.9f, 0.7f, 0.2f, 1));
            ImGui::TextColored(sc, "%d", report.totalScore);

            ImGui::TableNextColumn();
            ImGui::Text("%zu", report.flags.size());

            ImGui::TableNextColumn();
            if (!report.flags.empty()) {
                ImGui::TextWrapped("%s", report.flags[0].description.c_str());
            }

            ImGui::TableNextColumn();
            ImGui::PushID(report.playerUid.c_str());
            if (ImGui::SmallButton("Dismiss")) {
                for (auto& f : report.flags) m_app.scanEngine().dismissFlag(f.id);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Warn")) {
                m_app.palDefenderRCON().sendMessage(report.playerUid,
                    "Admin Warning: Suspicious activity detected.");
                for (auto& f : report.flags) m_app.scanEngine().actionFlag(f.id, "warn");
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Kick")) {
                m_app.palDefenderRCON().kickPlayer(report.playerUid, "Cheat detection");
                for (auto& f : report.flags) m_app.scanEngine().actionFlag(f.id, "kick");
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Ban")) {
                m_app.palDefenderRCON().banPlayer(report.playerUid, "Cheat detection");
                for (auto& f : report.flags) m_app.scanEngine().actionFlag(f.id, "ban");
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    // Detail panel
    if (!m_selectedReportUid.empty()) {
        ImGui::SameLine();
        ImGui::BeginChild("##FlagDetail", ImVec2(340, 0), true);
        renderPlayerReport(m_selectedReportUid);
        ImGui::EndChild();
    }
}

void ScanPanel::renderPlayerReport(const std::string& uid) {
    if (ImGui::Button("✕ Close Detail")) {
        m_selectedReportUid.clear();
        return;
    }

    auto reports = m_app.scanEngine().getReports();
    for (auto& r : reports) {
        if (r.playerUid != uid) continue;

        ImGui::Text("%s", r.playerName.c_str());
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1), "%s", r.playerUid.c_str());
        ImGui::Text("Total Score: %d", r.totalScore);
        ImGui::Text("Flags: %zu", r.flags.size());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        for (auto& flag : r.flags) {
            ImVec4 sevColor;
            switch (flag.severity) {
                case ScanSeverity::Critical: sevColor = ImVec4(0.9f, 0.15f, 0.15f, 1); break;
                case ScanSeverity::High: sevColor = ImVec4(0.9f, 0.4f, 0.2f, 1); break;
                case ScanSeverity::Medium: sevColor = ImVec4(0.9f, 0.7f, 0.2f, 1); break;
                default: sevColor = ImVec4(0.5f, 0.5f, 0.55f, 1); break;
            }

            ImGui::PushID(flag.id);
            ImGui::TextColored(sevColor, "[%s] +%d",
                severityToString(flag.severity).c_str(), flag.scoreImpact);
            ImGui::TextWrapped("%s", flag.description.c_str());
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.45f, 1), "Rule: %s", flag.ruleId.c_str());

            if (!flag.dismissed) {
                if (ImGui::SmallButton("Dismiss##flag")) {
                    m_app.scanEngine().dismissFlag(flag.id);
                }
            } else {
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.45f, 1), "(Dismissed)");
            }
            if (!flag.actionTaken.empty()) {
                ImGui::TextColored(ImVec4(0.85f, 0.65f, 0.2f, 1), "Action: %s", flag.actionTaken.c_str());
            }

            ImGui::Separator();
            ImGui::PopID();
        }
        break;
    }
}

void ScanPanel::renderScanHistory() {
    auto history = m_app.database().getScanHistory("", 200);

    ImGui::Text("Recent scan results (%zu entries)", history.size());
    ImGui::Spacing();

    if (ImGui::BeginTable("##ScanHistory", 5,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Player");
        ImGui::TableSetupColumn("Rule");
        ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("Description");
        ImGui::TableHeadersRow();

        for (auto& row : history) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%s", row.value("timestamp", "").c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", row.value("player_uid", "").c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", row.value("rule_id", "").c_str());

            ImGui::TableNextColumn();
            std::string sev = row.value("severity", "");
            ImVec4 sevColor = ImVec4(0.5f, 0.5f, 0.55f, 1);
            if (sev == "Critical") sevColor = ImVec4(0.9f, 0.15f, 0.15f, 1);
            else if (sev == "High") sevColor = ImVec4(0.9f, 0.4f, 0.2f, 1);
            else if (sev == "Medium") sevColor = ImVec4(0.9f, 0.7f, 0.2f, 1);
            ImGui::TextColored(sevColor, "%s", sev.c_str());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", row.value("description", "").c_str());
        }
        ImGui::EndTable();
    }
}

} // namespace PSM