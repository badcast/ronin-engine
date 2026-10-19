#pragma once

#include <string>
#include <vector>

struct SDL_Texture;

namespace RoninEditor
{
    class EditorApp;

    class ProjectHub
    {
    public:
        explicit ProjectHub(EditorApp *app);
        ~ProjectHub();

        void Render();
        void OpenAboutDialog();

    private:
        void renderHeader();
        void renderProjectsTab();
        void renderNewProjectTab();
        void renderAboutTab();
        void renderModals();
        void renderAboutDialog();
        void ensureLogoLoaded();

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

        // About modal & embedded logo
        bool m_showAboutDialog = false;
        ::SDL_Texture *m_logoTexture = nullptr;
        int m_logoWidth = 0;
        int m_logoHeight = 0;
    };
} // namespace RoninEditor
