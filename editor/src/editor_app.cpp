#include "editor_app.h"
#include "project_hub.h"
#include "project_manager.h"
#include "editor_icons.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <ronin/RoninSimulator.h>
#include <iostream>
#include <fstream>

namespace RoninEditor
{
    EditorApp &EditorApp::GetInstance()
    {
        static EditorApp instance;
        return instance;
    }

    EditorApp::EditorApp() = default;

    EditorApp::~EditorApp()
    {
        Shutdown();
    }

    void EditorApp::applyDarkTheme()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        ImVec4 *colors = style.Colors;

        // Modern, sleek Unity/JetBrains-inspired dark theme
        colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.56f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.18f, 0.98f);
        colors[ImGuiCol_Border] = ImVec4(0.24f, 0.24f, 0.27f, 0.80f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.26f, 0.30f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);

        colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

        colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.36f, 0.36f, 0.42f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.44f, 0.44f, 0.52f, 1.00f);

        colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.75f, 1.00f, 1.00f);

        colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.32f, 0.42f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.45f, 0.65f, 1.00f);

        colors[ImGuiCol_Header] = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.34f, 0.44f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.45f, 0.60f, 1.00f);

        colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.24f, 0.27f, 0.80f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.55f, 0.85f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.65f, 0.95f, 1.00f);

        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.26f, 0.30f, 0.60f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.35f, 0.60f, 0.90f, 0.80f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.45f, 0.70f, 1.00f, 1.00f);

        colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.34f, 0.46f, 1.00f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.22f, 0.26f, 0.35f, 1.00f);
        colors[ImGuiCol_TabDimmed] = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);

        colors[ImGuiCol_DockingPreview] = ImVec4(0.35f, 0.65f, 1.00f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

        // Geometrical adjustments
        style.WindowRounding = 4.0f;
        style.ChildRounding = 3.0f;
        style.FrameRounding = 3.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 3.0f;
        style.TabRounding = 4.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.TabBorderSize = 0.0f;

        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.ItemSpacing = ImVec2(8.0f, 5.0f);
        style.ItemInnerSpacing = ImVec2(5.0f, 4.0f);
    }

    bool EditorApp::Init(int argc, char **argv)
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            std::cerr << "Failed to initialize SDL2: " << SDL_GetError() << std::endl;
            return false;
        }

        RoninEngine::RoninSimulator::Init(RoninEngine::InitializeFlags::All);

        std::string initialProjectPath;
        for (int i = 1; i < argc; ++i)
        {
            if (std::string(argv[i]) == "--project" && i + 1 < argc)
            {
                initialProjectPath = argv[++i];
            }
            else if (argv[i][0] != '-')
            {
                initialProjectPath = argv[i];
            }
        }

        int winWidth = 960;
        int winHeight = 600;
        std::string winTitle = "Ronin Project Hub";

        if (!initialProjectPath.empty())
        {
            std::string err;
            if (ProjectManager::GetInstance().LoadProject(initialProjectPath, err))
            {
                winWidth = 1600;
                winHeight = 900;
                winTitle = "Ronin Editor - " + ProjectManager::GetInstance().GetCurrentProject().projectName;
                m_state = EditorAppState::MainEditor;
            }
            else
            {
                std::cerr << "Warning: Could not load initial project: " << err << std::endl;
            }
        }

        Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
        m_window = SDL_CreateWindow(
            winTitle.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            winWidth,
            winHeight,
            windowFlags);

        if (!m_window)
        {
            std::cerr << "Failed to create SDL_Window: " << SDL_GetError() << std::endl;
            return false;
        }

        m_renderer = SDL_CreateRenderer(
            m_window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if (!m_renderer)
        {
            std::cerr << "Failed to create SDL_Renderer: " << SDL_GetError() << std::endl;
            return false;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        applyDarkTheme();

        // Load crisp TrueType font with anti-aliasing and Cyrillic glyphs
        ImFontConfig fontConfig;
        fontConfig.OversampleH = 2;
        fontConfig.OversampleV = 2;
        fontConfig.RasterizerMultiply = 1.15f;

        //TODO: Move to Application Resoucrce outside from Operation System (Need cross-platforms)
        const char *candidateFonts[] = {
            "/usr/share/fonts/TTF/Roboto-Medium.ttf",
            "/usr/share/fonts/TTF/Roboto-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/noto/NotoSans-Regular.ttf"
        };

        ImFont *loadedFont = nullptr;
        for (const char *fontPath : candidateFonts)
        {
            if (std::filesystem::exists(fontPath))
            {
                loadedFont = io.Fonts->AddFontFromFileTTF(fontPath, 15.0f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());
                if (loadedFont)
                {
                    std::cout << "[RoninEditor] Loaded UI Font: " << fontPath << std::endl;
                    break;
                }
            }
        }
        if (!loadedFont)
        {
            io.Fonts->AddFontDefault();
        }

        ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer);
        ImGui_ImplSDLRenderer2_Init(m_renderer);

        m_sceneViewport.Init(m_renderer);
        m_spriteEditor.Init(m_renderer);

        m_assetBrowser.onOpenSpriteEditor = [this](const std::filesystem::path &p)
        {
            m_spriteEditor.OpenAsset(p);
        };
        m_inspectorPanel.onOpenSpriteEditor = [this](const std::filesystem::path &p)
        {
            m_spriteEditor.OpenAsset(p);
        };
        m_inspectorPanel.onPingAsset = [this](const std::filesystem::path &p)
        {
            m_assetBrowser.PingAsset(p);
        };

        if (m_state == EditorAppState::MainEditor)
        {
            std::filesystem::path scenePath = ProjectManager::GetInstance().GetDefaultScenePath();
            if (std::filesystem::exists(scenePath))
            {
                m_editorState.loadWorldFromFile(scenePath.string());
            }
        }

        m_hub = std::make_unique<ProjectHub>(this);
        m_running = true;

        return true;
    }

    void EditorApp::Run()
    {
        while (m_running)
        {
            handleEvents();
            renderFrame();
        }
    }

    void EditorApp::handleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
            {
                m_running = false;
            }
            else if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(m_window))
                {
                    m_running = false;
                }
            }
        }
    }

    void EditorApp::renderFrame()
    {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (m_state == EditorAppState::ProjectHub)
        {
            if (m_hub)
                m_hub->Render();
        }
        else
        {
            ImGuiIO &io = ImGui::GetIO();
            if (m_editorState.playState == EditorPlayState::Playing)
            {
                m_editorState.stepSimulation(io.DeltaTime);
            }
            renderMainEditor();
        }

        ImGui::Render();

        SDL_SetRenderDrawColor(m_renderer, 24, 24, 28, 255);
        SDL_RenderClear(m_renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_renderer);

        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        SDL_RenderPresent(m_renderer);
    }

    void EditorApp::renderMainEditor()
    {
        // 1. Fullscreen DockSpace host over main viewport
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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("RoninEditorDockSpaceHost", nullptr, hostFlags);
        ImGui::PopStyleVar(3);

        renderMainMenuBar();
        renderToolbar();

        ImGuiID dockspaceId = ImGui::GetID("RoninEditorDockSpace");

        // Setup default Unity-style docking layout on first launch or layout reset
        if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr || m_resetLayoutRequested)
        {
            m_resetLayoutRequested = false;
            ImGui::DockBuilderRemoveNode(dockspaceId);
            ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

            ImGuiID dockMain = dockspaceId;
            ImGuiID dockLeft = 0;
            ImGuiID dockRight = 0;
            ImGuiID dockBottom = 0;

            // 1. Right panel: Inspector (25% width)
            ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.25f, &dockRight, &dockMain);
            // 2. Left panel: Hierarchy (20% width)
            ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.20f, &dockLeft, &dockMain);
            // 3. Bottom panel: Project & Console (32% height of remaining center)
            ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.32f, &dockBottom, &dockMain);

            ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
            ImGui::DockBuilderDockWindow("World", dockMain);
            ImGui::DockBuilderDockWindow("Game", dockMain);
            ImGui::DockBuilderDockWindow("Sprite Editor", dockMain);
            ImGui::DockBuilderDockWindow("Project", dockBottom);
            ImGui::DockBuilderDockWindow("Console", dockBottom);
            ImGui::DockBuilderDockWindow("Terrain Editor", dockBottom);
            ImGui::DockBuilderDockWindow("Inspector", dockRight);

            ImGui::DockBuilderFinish(dockspaceId);
        }

        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::End();

        // Global Keyboard Shortcuts
        ImGuiIO &io = ImGui::GetIO();
        if (!io.WantTextInput)
        {
            if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_P, false))
            {
                m_assetFinder.Toggle();
            }
            else if (io.KeyCtrl && !io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false))
            {
                m_editorState.undo();
            }
            else if ((io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false)) ||
                     (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false)))
            {
                m_editorState.redo();
            }
            else if (io.KeyCtrl && !io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S, false))
            {
                std::filesystem::path worldPath = ProjectManager::GetInstance().GetDefaultScenePath();
                if (m_editorState.saveWorldToFile(worldPath.string()))
                {
                    m_editorState.logInfo("Saved World to " + worldPath.filename().string());
                }
            }
            else if (io.KeyCtrl && !io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_P, false))
            {
                bool isPlaying = (m_editorState.playState == EditorPlayState::Playing);
                m_editorState.setPlayState(isPlaying ? EditorPlayState::Stopped : EditorPlayState::Playing);
            }
        }

        // 2. Real dockable panels (Hierarchy, World, Game, Sprite Editor, Inspector, Project, Console, Terrain)
        m_hierarchyPanel.Render(m_editorState);
        m_sceneViewport.RenderScene(m_editorState);
        m_sceneViewport.RenderGame(m_editorState);
        m_spriteEditor.Render(m_editorState);
        m_inspectorPanel.Render(m_editorState);

        m_terrainEditor.Render(m_editorState);
        m_consolePanel.Render(m_editorState);
        m_assetBrowser.Render(m_editorState);

        // 3. Quick Asset Finder Modal
        m_assetFinder.Render(m_editorState, m_assetBrowser, [this](const std::filesystem::path &p) {
            m_spriteEditor.OpenAsset(p);
        });
    }

    void EditorApp::renderMainMenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save World", "Ctrl+S"))
                {
                    std::filesystem::path worldPath = ProjectManager::GetInstance().GetDefaultScenePath();
                    if (m_editorState.saveWorldToFile(worldPath.string()))
                    {
                        m_editorState.logInfo("Saved World to " + worldPath.filename().string());
                    }
                }
                if (ImGui::MenuItem("Save Project"))
                {
                    std::string err;
                    ProjectManager::GetInstance().SaveProject(err);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Open Project Hub..."))
                {
                    SwitchToProjectHub();
                }
                if (ImGui::MenuItem("Close Project"))
                {
                    SwitchToProjectHub();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                {
                    m_running = false;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "Ctrl+Z", false, m_editorState.canUndo()))
                {
                    m_editorState.undo();
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, m_editorState.canRedo()))
                {
                    m_editorState.redo();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Asset Finder...", "Ctrl+Shift+P"))
                {
                    m_assetFinder.Open();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Play / Stop Simulation", "Ctrl+P"))
                {
                    bool isPlaying = (m_editorState.playState == EditorPlayState::Playing);
                    m_editorState.setPlayState(isPlaying ? EditorPlayState::Stopped : EditorPlayState::Playing);
                }
                if (ImGui::MenuItem("Pause Simulation"))
                {
                    bool isPaused = (m_editorState.playState == EditorPlayState::Paused);
                    m_editorState.setPlayState(isPaused ? EditorPlayState::Playing : EditorPlayState::Paused);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Clear Console Logs"))
                {
                    m_editorState.clearLogs();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Assets"))
            {
                if (ImGui::MenuItem("Find Asset...", "Ctrl+Shift+P"))
                {
                    m_assetFinder.Open();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Create World"))
                {
                    const auto &cfg = ProjectManager::GetInstance().GetCurrentProject();
                    std::filesystem::path worldsDir = cfg.rootPath / "Assets" / "Worlds";
                    std::filesystem::create_directories(worldsDir);
                    std::filesystem::path newWorld = worldsDir / "NewWorld.world";
                    int count = 1;
                    while (std::filesystem::exists(newWorld))
                    {
                        newWorld = worldsDir / ("NewWorld" + std::to_string(count++) + ".world");
                    }
                    std::ofstream ofs(newWorld);
                    ofs << "{\n  \"formatVersion\": 1,\n  \"worldName\": \"NewWorld\",\n  \"objects\": []\n}\n";
                    ofs.close();
                    m_editorState.logInfo("Created new World: " + newWorld.filename().string());
                }
                if (ImGui::MenuItem("Create Sprite"))
                {
                    const auto &cfg = ProjectManager::GetInstance().GetCurrentProject();
                    std::filesystem::path spritesDir = cfg.rootPath / "Assets" / "Sprites";
                    std::filesystem::create_directories(spritesDir);
                    std::filesystem::path newSprite = spritesDir / "NewSprite.sprite";
                    int count = 1;
                    while (std::filesystem::exists(newSprite))
                    {
                        newSprite = spritesDir / ("NewSprite" + std::to_string(count++) + ".sprite");
                    }
                    std::ofstream ofs(newSprite);
                    ofs << "{\n  \"formatVersion\": 1,\n  \"name\": \"" << newSprite.stem().string() << "\",\n  \"texturePath\": \"\",\n  \"rect\": [0, 0, 64, 64],\n  \"pivot\": [0.5, 0.5],\n  \"pixelsPerUnit\": 100.0\n}\n";
                    ofs.close();
                    m_editorState.logInfo("Created new Sprite: " + newSprite.filename().string());
                    m_spriteEditor.OpenAsset(newSprite);
                }
                if (ImGui::MenuItem("Create C++ Script"))
                {
                    const auto &cfg = ProjectManager::GetInstance().GetCurrentProject();
                    std::filesystem::path scriptsDir = cfg.rootPath / "Scripts";
                    std::filesystem::path newScript = scriptsDir / "CustomComponent.h";
                    std::ofstream ofs(newScript);
                    ofs << "#pragma once\n#include <ronin/ronin.h>\n\nclass CustomComponent : public RoninEngine::Runtime::Component {\npublic:\n    void OnStart() override {}\n    void OnUpdate() override {}\n};\n";
                    ofs.close();
                    m_editorState.logInfo("Created new C++ Script: CustomComponent.h");
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("GameObject"))
            {
                if (ImGui::MenuItem("Create Empty"))
                {
                    m_editorState.createObject("Empty GameObject");
                }
                if (ImGui::BeginMenu("2D Object"))
                {
                    if (ImGui::MenuItem("Sprite Rectangle"))
                    {
                        auto &obj = m_editorState.createObject("Sprite Rectangle");
                        obj.sprite.enabled = true;
                        obj.sprite.primitiveShape = 0;
                        obj.sprite.size = {1.0f, 1.0f};
                        obj.sprite.color = RoninEngine::Runtime::Color{220, 220, 220, 255};
                    }
                    if (ImGui::MenuItem("Sprite Circle"))
                    {
                        auto &obj = m_editorState.createObject("Sprite Circle");
                        obj.sprite.enabled = true;
                        obj.sprite.primitiveShape = 1;
                        obj.sprite.size = {1.0f, 1.0f};
                        obj.sprite.color = RoninEngine::Runtime::Color{80, 180, 240, 255};
                    }
                    if (ImGui::MenuItem("Terrain 2D"))
                    {
                        auto &obj = m_editorState.createObject("Terrain 2D");
                        obj.terrain.enabled = true;
                        obj.terrain.width = 16;
                        obj.terrain.length = 8;
                        obj.terrain.cellScale = 1.0f;
                        obj.terrain.ensureGridInitialized(0);
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Light"))
                {
                    if (ImGui::MenuItem("Point Light"))
                    {
                        auto &obj = m_editorState.createObject("Point Light");
                        obj.light.enabled = true;
                        obj.light.lightType = 0;
                        obj.light.range = 4.0f;
                        obj.light.color = RoninEngine::Runtime::Color{255, 240, 160, 220};
                    }
                    if (ImGui::MenuItem("Directional Light"))
                    {
                        auto &obj = m_editorState.createObject("Directional Light");
                        obj.light.enabled = true;
                        obj.light.lightType = 1;
                        obj.light.range = 10.0f;
                        obj.light.color = RoninEngine::Runtime::Color{255, 255, 255, 220};
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Audio -> Audio Source"))
                {
                    auto &obj = m_editorState.createObject("Audio Source");
                    obj.audio.enabled = true;
                    obj.audio.volume = 0.85f;
                }
                if (ImGui::MenuItem("Camera"))
                {
                    auto &obj = m_editorState.createObject("Camera");
                    obj.camera.enabled = true;
                    obj.camera.zoom = 1.0f;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("Asset Finder", "Ctrl+Shift+P", m_assetFinder.IsOpen()))
                {
                    m_assetFinder.Toggle();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Sprite Editor", nullptr, m_spriteEditor.IsOpen()))
                {
                    m_spriteEditor.SetOpen(!m_spriteEditor.IsOpen());
                }
                if (ImGui::MenuItem("Terrain Editor", nullptr, m_terrainEditor.IsOpen()))
                {
                    m_terrainEditor.SetOpen(!m_terrainEditor.IsOpen());
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Layout"))
                {
                    m_resetLayoutRequested = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About RoninEditor"))
                {
                    SwitchToProjectHub();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void EditorApp::renderToolbar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

        if (ImGui::BeginChild("MainToolbar", ImVec2(0.0f, 36.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            // 1. Tool Selection Mode: Move, Rotate, Scale, Terrain
            int currentTool = static_cast<int>(m_editorState.gizmoTool);

            if (EditorIcons::IconButton("##ToolMove", IconType::Move, ImVec2(32, 26), currentTool == 0))
            {
                m_editorState.gizmoTool = GizmoToolMode::SelectMove;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move Tool (W)");
            ImGui::SameLine();

            if (EditorIcons::IconButton("##ToolRotate", IconType::Rotate, ImVec2(32, 26), currentTool == 1))
            {
                m_editorState.gizmoTool = GizmoToolMode::Rotate;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rotate Tool (E)");
            ImGui::SameLine();

            if (EditorIcons::IconButton("##ToolScale", IconType::Scale, ImVec2(32, 26), currentTool == 2))
            {
                m_editorState.gizmoTool = GizmoToolMode::Scale;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Scale Tool (R)");
            ImGui::SameLine();

            if (EditorIcons::IconButton("##ToolTerrain", IconType::Terrain, ImVec2(32, 26), currentTool == 3))
            {
                m_editorState.gizmoTool = GizmoToolMode::TerrainPaint;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Terrain Tool (T)");
            ImGui::SameLine();

            if (EditorIcons::IconButton("##ToolFinder", IconType::Search, ImVec2(32, 26), m_assetFinder.IsOpen()))
            {
                m_assetFinder.Toggle();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Quick Asset Finder (Ctrl+Shift+P)");
            ImGui::SameLine();

            ImGui::SameLine(0, 12.0f);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 12.0f);

            // 2. Playback Controls (Centered): Play, Pause, Step
            float centerOffset = (ImGui::GetWindowWidth() - 150.0f) * 0.5f;
            if (centerOffset > ImGui::GetCursorPosX())
            {
                ImGui::SameLine(centerOffset);
            }

            bool isPlaying = (m_editorState.playState == EditorPlayState::Playing);
            bool isPaused = (m_editorState.playState == EditorPlayState::Paused);

            if (isPlaying)
            {
                if (EditorIcons::IconButton("##PlayBtn", IconType::Stop, ImVec2(36, 26), true, EditorPalette::Danger))
                {
                    m_editorState.setPlayState(EditorPlayState::Stopped);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop Simulation (Ctrl+P)");
            }
            else
            {
                if (EditorIcons::IconButton("##PlayBtn", IconType::Play, ImVec2(36, 26), false, EditorPalette::Success))
                {
                    m_editorState.setPlayState(EditorPlayState::Playing);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Play Simulation (Ctrl+P)");
            }

            ImGui::SameLine();
            if (EditorIcons::IconButton("##PauseBtn", IconType::Pause, ImVec2(32, 26), isPaused, isPaused ? EditorPalette::Warning : EditorPalette::TextNormal))
            {
                m_editorState.setPlayState(isPaused ? EditorPlayState::Playing : EditorPlayState::Paused);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(isPaused ? "Resume Simulation" : "Pause Simulation");

            ImGui::SameLine();
            if (EditorIcons::IconButton("##StepBtn", IconType::Step, ImVec2(32, 26), false, EditorPalette::TextNormal))
            {
                m_editorState.stepSimulation(1.0f / 60.0f);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Step Simulation (1 Frame)");

            // 3. Build Project (Right-aligned)
            float buildWidth = 110.0f;
            float rightOffset = ImGui::GetWindowWidth() - buildWidth - 12.0f;
            if (rightOffset > ImGui::GetCursorPosX())
            {
                ImGui::SameLine(rightOffset);
            }
            if (ImGui::Button("Build Project", ImVec2(buildWidth, 26.0f)))
            {
                m_editorState.logInfo("Build Project triggered for: " + ProjectManager::GetInstance().GetCurrentProject().projectName);
            }

            ImGui::EndChild();
        }

        ImGui::PopStyleVar(2);
    }

    void EditorApp::SwitchToProjectHub()
    {
        ProjectManager::GetInstance().CloseProject();
        m_state = EditorAppState::ProjectHub;

        SDL_SetWindowTitle(m_window, "Ronin Project Hub");
        SDL_SetWindowSize(m_window, 960, 600);
        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    void EditorApp::SwitchToMainEditor(const std::string &projectPath)
    {
        (void)projectPath;
        m_state = EditorAppState::MainEditor;

        const auto &cfg = ProjectManager::GetInstance().GetCurrentProject();
        std::string title = "Ronin Editor - " + cfg.projectName + " - " + cfg.defaultSceneRelativePath;
        SDL_SetWindowTitle(m_window, title.c_str());
        SDL_SetWindowSize(m_window, 1600, 900);
        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        m_sceneViewport.Init(m_renderer);
        std::filesystem::path scenePath = cfg.rootPath / cfg.defaultSceneRelativePath;
        if (std::filesystem::exists(scenePath))
        {
            m_editorState.loadWorldFromFile(scenePath.string());
        }
    }

    void EditorApp::RequestQuit()
    {
        m_running = false;
    }

    void EditorApp::Shutdown()
    {
        m_spriteEditor.Shutdown();

        if (m_renderer)
        {
            ImGui_ImplSDLRenderer2_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();

            SDL_DestroyRenderer(m_renderer);
            m_renderer = nullptr;
        }

        if (m_window)
        {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }

        RoninEngine::RoninSimulator::Finalize();
        SDL_Quit();
    }
} // namespace RoninEditor
