#include "project_hub.h"
#include "project_manager.h"
#include "editor_app.h"
#include <imgui.h>
#include <filesystem>
#include <cstring>
#include <ronin/RoninSimulator.h>

namespace RoninEditor
{
    ProjectHub::ProjectHub(EditorApp *app)
        : m_app(app)
    {
        const char *home = std::getenv("HOME");
        if (home && *home)
        {
            std::filesystem::path defaultDir = std::filesystem::path(home) / "RoninProjects";
            std::strncpy(m_newProjectLocation, defaultDir.string().c_str(), sizeof(m_newProjectLocation) - 1);
        }
        else
        {
            std::filesystem::path defaultDir = std::filesystem::current_path() / "Projects";
            std::strncpy(m_newProjectLocation, defaultDir.string().c_str(), sizeof(m_newProjectLocation) - 1);
        }
    }

    void ProjectHub::Render()
    {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus |
                                     ImGuiWindowFlags_NoNavFocus |
                                     ImGuiWindowFlags_MenuBar;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));

        ImGui::Begin("ProjectHubMainWindow", nullptr, hostFlags);
        ImGui::PopStyleVar(3);

        renderHeader();

        ImGui::Separator();
        ImGui::Spacing();

        // Hub Navigation Tabs
        if (ImGui::BeginTabBar("ProjectHubTabs"))
        {
            if (ImGui::BeginTabItem("Projects"))
            {
                m_selectedTab = 0;
                renderProjectsTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("New Project"))
            {
                m_selectedTab = 1;
                renderNewProjectTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("About & Community"))
            {
                m_selectedTab = 2;
                renderAboutTab();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        renderModals();

        ImGui::End();
    }

    void ProjectHub::renderHeader()
    {
        ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "RONIN PROJECT HUB");

        ImGui::SameLine();
        ImGui::TextDisabled(" |  RoninEngine v%s  |  Editor v%s",
                            RoninEngine::RoninSimulator::GetVersionString(),
                            RONIN_EDITOR_VERSION_STRING);

        ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f);
        if (ImGui::Button("Quit Hub", ImVec2(130.0f, 26.0f)))
        {
            if (m_app)
                m_app->RequestQuit();
        }
    }

    void ProjectHub::renderProjectsTab()
    {
        ImGui::Spacing();

        // Top action bar
        ImGui::BeginGroup();
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputTextWithHint("##SearchProjects", "Search recent projects...", m_searchFilter, sizeof(m_searchFilter));
        ImGui::SameLine();
        if (ImGui::Button("+ New Project", ImVec2(120.0f, 0.0f)))
        {
            m_selectedTab = 1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Open from Disk...", ImVec2(140.0f, 0.0f)))
        {
            m_showOpenDialog = true;
            std::strncpy(m_openPathBuffer, m_newProjectLocation, sizeof(m_openPathBuffer) - 1);
        }
        ImGui::EndGroup();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const auto &recent = ProjectManager::GetInstance().GetRecentProjects();

        if (recent.empty())
        {
            ImGui::Spacing();
            ImGui::TextDisabled("No recent projects found.");
            ImGui::TextDisabled("Click '+ New Project' to create your first RoninEngine game!");
            return;
        }

        std::string filterLower = m_searchFilter;
        for (char &c : filterLower)
            c = static_cast<char>(std::tolower(c));

        ImGui::BeginChild("RecentProjectsList", ImVec2(0, 0), true);

        for (size_t i = 0; i < recent.size(); ++i)
        {
            const auto &proj = recent[i];

            if (!filterLower.empty())
            {
                std::string nameLower = proj.name;
                for (char &c : nameLower)
                    c = static_cast<char>(std::tolower(c));
                if (nameLower.find(filterLower) == std::string::npos &&
                    proj.path.find(filterLower) == std::string::npos)
                {
                    continue;
                }
            }

            ImGui::PushID(static_cast<int>(i));

            // Project Item Card
            ImGui::BeginGroup();
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", proj.name.c_str());

            if (!proj.exists)
            {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[Missing Directory]");
            }

            ImGui::TextDisabled("%s", proj.path.c_str());
            ImGui::TextDisabled("Last Opened: %s", proj.lastOpened.c_str());
            ImGui::EndGroup();

            ImGui::SameLine(ImGui::GetWindowWidth() - 190.0f);

            if (proj.exists)
            {
                if (ImGui::Button("Open", ImVec2(80.0f, 32.0f)))
                {
                    std::string err;
                    if (ProjectManager::GetInstance().LoadProject(proj.path, err))
                    {
                        if (m_app)
                            m_app->SwitchToMainEditor(proj.path);
                    }
                    else
                    {
                        m_errorMessage = err;
                        m_showErrorModal = true;
                    }
                }
            }
            else
            {
                ImGui::BeginDisabled();
                ImGui::Button("Open", ImVec2(80.0f, 32.0f));
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (ImGui::Button("Remove", ImVec2(80.0f, 32.0f)))
            {
                ProjectManager::GetInstance().RemoveRecentProject(proj.path);
                ImGui::PopID();
                break;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        ImGui::EndChild();
    }

    void ProjectHub::renderNewProjectTab()
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Create a New RoninEngine Project");
        ImGui::Spacing();

        ImGui::BeginChild("NewProjectForm", ImVec2(750.0f, 400.0f), true);

        // Project Name
        ImGui::Text("Project Name:");
        ImGui::SetNextItemWidth(450.0f);
        ImGui::InputText("##ProjectName", m_newProjectName, sizeof(m_newProjectName));

        ImGui::Spacing();

        // Location
        ImGui::Text("Project Location (Parent Directory):");
        ImGui::SetNextItemWidth(450.0f);
        ImGui::InputText("##ProjectLocation", m_newProjectLocation, sizeof(m_newProjectLocation));

        ImGui::Spacing();

        // Target path preview
        std::filesystem::path targetPath = std::filesystem::path(m_newProjectLocation) / m_newProjectName;
        ImGui::TextDisabled("Target Project Directory:");
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", targetPath.string().c_str());

        std::error_code ec;
        bool dirExists = std::filesystem::exists(targetPath, ec);
        bool dirNonEmpty = dirExists && !std::filesystem::is_empty(targetPath, ec);

        if (dirNonEmpty)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                               "Warning: Target directory already exists and contains files!");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Template Selection
        ImGui::Text("Template:");
        ImGui::RadioButton("2D Empty Project (Blank World, Camera, Light)", &m_selectedTemplate, 0);
        ImGui::RadioButton("2D Action Project (Sample World, Samurai Player, Dojo Terrain, C++ Script)", &m_selectedTemplate, 1);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Create Button
        bool canCreate = (std::strlen(m_newProjectName) > 0) &&
                         (std::strlen(m_newProjectLocation) > 0) &&
                         !dirNonEmpty;

        if (!canCreate)
            ImGui::BeginDisabled();

        if (ImGui::Button("Create Project", ImVec2(160.0f, 36.0f)))
        {
            std::string err;
            ProjectTemplate tmpl = (m_selectedTemplate == 1) ? ProjectTemplate::Action2D : ProjectTemplate::Empty2D;
            if (ProjectManager::GetInstance().CreateProject(m_newProjectLocation, m_newProjectName, tmpl, err))
            {
                if (m_app)
                    m_app->SwitchToMainEditor(targetPath.string());
            }
            else
            {
                m_errorMessage = err;
                m_showErrorModal = true;
            }
        }

        if (!canCreate)
            ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 36.0f)))
        {
            m_selectedTab = 0;
        }

        ImGui::EndChild();
    }

    void ProjectHub::renderAboutTab()
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "RoninEngine & Ronin Editor");
        ImGui::Spacing();

        ImGui::BeginChild("AboutBox", ImVec2(0, 0), true);
        ImGui::Text("Official Visual Game Development Editor for RoninEngine.");
        ImGui::TextDisabled("Repository: https://github.com/badcast/ronin-engine");
        ImGui::TextDisabled("License: MIT License");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Technical Stack & Architecture:");
        ImGui::BulletText("Core Engine: C++17, RoninEngine::Runtime (GameObjects, Components, Transforms)");
        ImGui::BulletText("Backend: SDL2 (Windowing, Audio, Rendering, Input)");
        ImGui::BulletText("Editor GUI: Dear ImGui (Docking Branch, Multi-Viewport)");
        ImGui::BulletText("Serialization: SimpleJSON (.roninproject, .world, ProjectSettings)");
        ImGui::BulletText("Linked Subsystems: %s", RoninEngine::RoninSimulator::GetLinkedTargets());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        RoninEngine::VersionInfo vi = RoninEngine::RoninSimulator::GetVersion();
        ImGui::Text("Library Versions:");
        ImGui::BulletText("RoninEngine: %s", RoninEngine::RoninSimulator::GetVersionString());
        ImGui::BulletText("Dear ImGui: %s (Docking)", IMGUI_VERSION);
        ImGui::BulletText("SDL2: %d.%d.%d", vi.SDL_Version.major, vi.SDL_Version.minor, vi.SDL_Version.patch);
        ImGui::BulletText("SDL_image: %d.%d.%d", vi.SDL_IMG_Version.major, vi.SDL_IMG_Version.minor, vi.SDL_IMG_Version.patch);
        ImGui::BulletText("SDL_ttf: %d.%d.%d", vi.SDL_TTF_Version.major, vi.SDL_TTF_Version.minor, vi.SDL_TTF_Version.patch);
        ImGui::BulletText("SDL_mixer: %d.%d.%d", vi.SDL_Mix_Version.major, vi.SDL_Mix_Version.minor, vi.SDL_Mix_Version.patch);

        ImGui::EndChild();
    }

    void ProjectHub::renderModals()
    {
        if (m_showErrorModal)
        {
            ImGui::OpenPopup("Project Error");
            m_showErrorModal = false;
        }

        if (ImGui::BeginPopupModal("Project Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Error:");
            ImGui::TextWrapped("%s", m_errorMessage.c_str());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (m_showOpenDialog)
        {
            ImGui::OpenPopup("Open Project from Disk");
            m_showOpenDialog = false;
        }

        if (ImGui::BeginPopupModal("Open Project from Disk", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter directory path containing 'project.ronin':");
            ImGui::SetNextItemWidth(500.0f);
            ImGui::InputText("##OpenDirPath", m_openPathBuffer, sizeof(m_openPathBuffer));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Open Project", ImVec2(130.0f, 0.0f)))
            {
                std::string err;
                if (ProjectManager::GetInstance().LoadProject(m_openPathBuffer, err))
                {
                    ImGui::CloseCurrentPopup();
                    if (m_app)
                        m_app->SwitchToMainEditor(m_openPathBuffer);
                }
                else
                {
                    m_errorMessage = err;
                    m_showErrorModal = true;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
} // namespace RoninEditor
