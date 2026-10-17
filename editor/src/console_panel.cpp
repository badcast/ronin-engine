#include "console_panel.h"
#include <imgui.h>
#include <algorithm>
#include <string>

namespace RoninEditor
{
    ConsolePanel::ConsolePanel() = default;
    ConsolePanel::~ConsolePanel() = default;

    void ConsolePanel::Render(EditorState &state)
    {
        if (ImGui::Begin("Console"))
        {
            // 1. Count logs by level
            int infoCount = 0;
            int warnCount = 0;
            int errorCount = 0;

            for (const auto &log : state.logs)
            {
                if (log.level == "ERROR") errorCount++;
                else if (log.level == "WARN") warnCount++;
                else infoCount++;
            }

            // 2. Toolbar
            if (ImGui::Button("Clear"))
            {
                state.clearLogs();
                m_selectedLogIndex = -1;
            }
            ImGui::SameLine();

            ImGui::Checkbox("Collapse", &m_collapse);
            ImGui::SameLine();
            ImGui::Checkbox("Clear on Play", &m_clearOnPlay);
            ImGui::SameLine();
            ImGui::Checkbox("Auto-scroll", &m_autoScroll);
            ImGui::SameLine();

            // Search filter
            ImGui::SetNextItemWidth(160.0f);
            ImGui::InputTextWithHint("##SearchLog", "Search log...", m_searchFilter, sizeof(m_searchFilter));

            // Right-aligned level toggles with counts
            float rightWidth = 260.0f;
            float rightOffset = ImGui::GetWindowWidth() - rightWidth - 10.0f;
            if (rightOffset > ImGui::GetCursorPosX())
            {
                ImGui::SameLine(rightOffset);
            }

            // Info filter
            std::string infoLabel = "Info (" + std::to_string(infoCount) + ")";
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.5f, 1.0f));
            ImGui::Checkbox(infoLabel.c_str(), &m_showInfo);
            ImGui::PopStyleColor();
            ImGui::SameLine();

            // Warn filter
            std::string warnLabel = "Warn (" + std::to_string(warnCount) + ")";
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
            ImGui::Checkbox(warnLabel.c_str(), &m_showWarning);
            ImGui::PopStyleColor();
            ImGui::SameLine();

            // Error filter
            std::string errLabel = "Error (" + std::to_string(errorCount) + ")";
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
            ImGui::Checkbox(errLabel.c_str(), &m_showError);
            ImGui::PopStyleColor();

            ImGui::Separator();

            // 3. Log list area (split top/bottom if log selected)
            float listHeight = (m_selectedLogIndex >= 0 && m_selectedLogIndex < static_cast<int>(state.logs.size()))
                                   ? ImGui::GetContentRegionAvail().y - 80.0f
                                   : 0.0f;

            ImGui::BeginChild("LogList", ImVec2(0, listHeight), false);

            struct DisplayLogItem
            {
                std::string level;
                std::string message;
                std::string timestamp;
                int count = 1;
                int originalIndex = 0;
            };
            std::vector<DisplayLogItem> displayLogs;

            if (m_collapse)
            {
                std::vector<size_t> keyOrder;
                for (size_t i = 0; i < state.logs.size(); ++i)
                {
                    const auto &log = state.logs[i];
                    bool found = false;
                    for (auto &item : displayLogs)
                    {
                        if (item.level == log.level && item.message == log.message)
                        {
                            item.count++;
                            item.timestamp = log.timestamp;
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        displayLogs.push_back({log.level, log.message, log.timestamp, 1, static_cast<int>(i)});
                    }
                }
            }
            else
            {
                for (size_t i = 0; i < state.logs.size(); ++i)
                {
                    const auto &log = state.logs[i];
                    displayLogs.push_back({log.level, log.message, log.timestamp, 1, static_cast<int>(i)});
                }
            }

            for (size_t i = 0; i < displayLogs.size(); ++i)
            {
                const auto &log = displayLogs[i];

                if (log.level == "ERROR" && !m_showError) continue;
                if (log.level == "WARN" && !m_showWarning) continue;
                if (log.level != "ERROR" && log.level != "WARN" && !m_showInfo) continue;

                // Search check
                if (m_searchFilter[0] != '\0')
                {
                    std::string msgLower = log.message;
                    std::string filterLower = m_searchFilter;
                    std::transform(msgLower.begin(), msgLower.end(), msgLower.begin(), ::tolower);
                    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
                    if (msgLower.find(filterLower) == std::string::npos)
                        continue;
                }

                ImVec4 col = ImVec4(0.85f, 0.85f, 0.90f, 1.0f);
                const char *icon = "[INFO]";
                if (log.level == "ERROR")
                {
                    col = ImVec4(1.0f, 0.35f, 0.35f, 1.0f);
                    icon = "[ERROR]";
                }
                else if (log.level == "WARN")
                {
                    col = ImVec4(1.0f, 0.80f, 0.25f, 1.0f);
                    icon = "[WARN]";
                }

                ImGui::PushID(static_cast<int>(i));

                bool isSelected = (m_selectedLogIndex == log.originalIndex);
                std::string lineStr = log.timestamp + " " + icon + " " + log.message;

                ImGui::PushStyleColor(ImGuiCol_Text, col);
                if (ImGui::Selectable(lineStr.c_str(), isSelected))
                {
                    m_selectedLogIndex = log.originalIndex;
                }
                ImGui::PopStyleColor();

                // If collapsed and count > 1, show counter badge
                if (m_collapse && log.count > 1)
                {
                    std::string countStr = std::to_string(log.count);
                    float badgeW = ImGui::CalcTextSize(countStr.c_str()).x + 18.0f;
                    float posX = ImGui::GetWindowWidth() - badgeW - 24.0f;
                    if (posX > ImGui::GetCursorPosX())
                    {
                        ImGui::SameLine(posX);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.28f, 0.36f, 0.9f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 1.0f, 1.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 1.0f));
                        ImGui::SmallButton(countStr.c_str());
                        ImGui::PopStyleVar(2);
                        ImGui::PopStyleColor(2);
                    }
                }

                ImGui::PopID();
            }

            if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();

            // 4. Log detail area
            if (m_selectedLogIndex >= 0 && m_selectedLogIndex < static_cast<int>(state.logs.size()))
            {
                ImGui::Separator();
                ImGui::BeginChild("LogDetail", ImVec2(0, 0), false);
                const auto &sel = state.logs[m_selectedLogIndex];
                ImGui::TextDisabled("Timestamp: %s  |  Level: %s", sel.timestamp.c_str(), sel.level.c_str());
                ImGui::TextWrapped("%s", sel.message.c_str());
                ImGui::EndChild();
            }
        }
        ImGui::End();
    }
} // namespace RoninEditor
