#pragma once

#include <string>
#include <vector>

namespace RoninEditor
{
    class EditorApp;

    class ProjectHub
    {
    public:
        explicit ProjectHub(EditorApp *app);
        ~ProjectHub() = default;

        void Render();

    private:
        void renderHeader();
        void renderProjectsTab();
        void renderNewProjectTab();
        void renderAboutTab();
        void renderModals();

        EditorApp *m_app = nullptr;

        int m_selectedTab = 0; // 0 = Projects, 1 = New Project, 2 = About
        char m_searchFilter[128] = "";

        // New Project fields
        char m_newProjectName[128] = "MyRoninGame";
        char m_newProjectLocation[512] = "";
        int m_selectedTemplate = 0; // 0 = Empty 2D, 1 = Action 2D

        // Open Project field
        char m_openPathBuffer[512] = "";
        bool m_showOpenDialog = false;

        // Error modal
        bool m_showErrorModal = false;
        std::string m_errorMessage;
    };
} // namespace RoninEditor
