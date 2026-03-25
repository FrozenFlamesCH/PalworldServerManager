// BackupPanel.cpp
#include "ui/panels/BackupPanel.h"
#include "Application.h"
#include "server/BackupManager.h"
#include "ui/Widgets.h"
#include <imgui.h>
#include <ctime>

namespace PSM {
void BackupPanel::render() {
    ImGui::Text("💾 Backup & Storage");
    ImGui::Separator();
    ImGui::Spacing();
    if (ImGui::Button("Create Backup Now", ImVec2(200, 35)))
        m_app.backupManager().createBackup();
    ImGui::Spacing();
    auto backups = m_app.backupManager().getBackupList();
    if (ImGui::BeginTable("##BK", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Date"); ImGui::TableSetupColumn("Path");
        ImGui::TableSetupColumn("Size"); ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();
        for (auto& b : backups) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            auto t = static_cast<time_t>(b.timestamp);
            std::tm tm{}; 
#ifdef _WIN32
            localtime_s(&tm, &t);
#else
            localtime_r(&t, &tm);
#endif
            char buf[32]; std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm);
            ImGui::Text("%s", buf);
            ImGui::TableNextColumn(); ImGui::Text("%s", b.path.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%.1f MB", b.sizeBytes / (1024.f*1024.f));
            ImGui::TableNextColumn();
            ImGui::PushID(b.id);
            if (ImGui::SmallButton("Restore")) { m_restorePath = b.path; m_restoreConfirm = true; }
            ImGui::SameLine();
            if (ImGui::SmallButton("Delete")) m_app.backupManager().deleteBackup(b.id);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (Widgets::ConfirmDialog("Restore Backup", "Restore this backup? Current save will be overwritten.",
        &m_restoreConfirm))
        m_app.backupManager().restoreBackup(m_restorePath);
}
} // namespace PSM
