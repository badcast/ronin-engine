#pragma once

#include <string>
#include <memory>
#include <SDL2/SDL.h>

#include "editor_state.h"
#include "scene_viewport.h"
#include "hierarchy_panel.h"
#include "inspector_panel.h"
#include "asset_browser.h"
#include "console_panel.h"
#include "terrain_editor.h"
#include "sprite_editor.h"
#include "asset_finder.h"

namespace RoninEditor
{
    class ProjectHub;

    enum class EditorAppState
    {
        ProjectHub = 0,
        MainEditor = 1
    };

    class EditorApp
    {
    public:
        static EditorApp &GetInstance();

        bool Init(int argc, char **argv);
        void Run();
        void Shutdown();

        void SwitchToProjectHub();
        void SwitchToMainEditor(const std::string &projectPath);
        void RequestQuit();

        SDL_Window *GetWindow() const { return m_window; }
        SDL_Renderer *GetRenderer() const { return m_renderer; }
        EditorAppState GetState() const { return m_state; }
        EditorState &GetEditorState() { return m_editorState; }
        SpriteEditor &GetSpriteEditor() { return m_spriteEditor; }
        AssetFinder &GetAssetFinder() { return m_assetFinder; }

    private:
        EditorApp();
        ~EditorApp();
        EditorApp(const EditorApp &) = delete;
        EditorApp &operator=(const EditorApp &) = delete;

        void handleEvents();
        void renderFrame();
        void renderMainEditor();
        void renderMainMenuBar();
        void renderToolbar();
        void applyDarkTheme();

        SDL_Window *m_window = nullptr;
        SDL_Renderer *m_renderer = nullptr;

        EditorAppState m_state = EditorAppState::ProjectHub;
        bool m_running = false;
        bool m_resetLayoutRequested = false;

        std::unique_ptr<ProjectHub> m_hub;
        EditorState m_editorState;
        SceneViewport m_sceneViewport;
        HierarchyPanel m_hierarchyPanel;
        InspectorPanel m_inspectorPanel;
        AssetBrowser m_assetBrowser;
        AssetFinder m_assetFinder;
        ConsolePanel m_consolePanel;
        TerrainEditorPanel m_terrainEditor;
        SpriteEditor m_spriteEditor;
    };
} // namespace RoninEditor
