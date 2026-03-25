#include "ui/panels/CommunicationPanel.h"
#include "Application.h"
#include "net/PalDefenderRCON.h"
#include "server/ServerManager.h"
#include "ui/Widgets.h"
#include <imgui.h>
#include <cstring>

namespace PSM {

void CommunicationPanel::render() {
    ImGui::Text("💬 Communication");
    ImGui::Separator();
    ImGui::Spacing();

    Widgets::SeparatorText("Broadcast to All");
    ImGui::InputTextMultiline("##broadcast", m_broadcastBuf, sizeof(m_broadcastBuf), ImVec2(-1, 60));
    if (ImGui::Button("Send Broadcast") && strlen(m_broadcastBuf) > 0) {
        m_app.serverManager().broadcastMessage(m_broadcastBuf);
        m_broadcastBuf[0] = '\0';
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Alert (Prominent)");
    ImGui::InputText("##alert", m_alertBuf, sizeof(m_alertBuf));
    ImGui::SameLine();
    if (ImGui::Button("Send Alert") && strlen(m_alertBuf) > 0) {
        m_app.palDefenderRCON().alert(m_alertBuf);
        m_alertBuf[0] = '\0';
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Direct Message");
    ImGui::InputText("Target UID##dm", m_msgTargetUid, sizeof(m_msgTargetUid));
    ImGui::InputText("Message##dm", m_msgBuf, sizeof(m_msgBuf));
    if (ImGui::Button("Send##dm") && strlen(m_msgTargetUid) > 0 && strlen(m_msgBuf) > 0) {
        m_app.palDefenderRCON().sendMessage(m_msgTargetUid, m_msgBuf);
        m_msgBuf[0] = '\0';
    }

    ImGui::Spacing();
    Widgets::SeparatorText("Announcement Toggles");
    Widgets::ToggleSwitch("Login/Logout", &m_loginAnnounce);
    Widgets::ToggleSwitch("Deaths", &m_deathAnnounce);
    Widgets::ToggleSwitch("Punishments", &m_punishAnnounce);
}

} // namespace PSM
