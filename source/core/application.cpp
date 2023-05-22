#include "ronin.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine
{
    namespace Runtime
    {
        float internal_time_scale, internal_game_time, internal_delta_time;

        std::uint32_t internal_start_engine_time;
        std::uint32_t internal_frames; // framecounter
        std::vector<std::uint32_t> _watcher_time;

        Vec2Int m_mousePoint;
        std::uint8_t mouseState;
        std::uint8_t lastMouseState;
        std::uint8_t mouseWheels;
        Vec2 m_axis;
        bool text_inputState;

        extern GameObject* create_empty_gameobject();

        extern void manager_init();

        extern void text_get(std::string& text);
        extern void text_stop_input();
        extern void text_start_input();
        extern void input_movement_update();
        extern void internal_update_input(SDL_Event* e);

        extern bool unload_level(Level* level);
    }

    namespace UI
    {
        extern void ui_controls_init();
        extern void ui_free_controls();
        extern void ui_reset_controls();
    }

    static bool m_inited = false;
    static bool m_levelLoaded = false;
    static bool m_levelAccept = false;
    static Runtime::Level* destroyableLevel = nullptr;
    static SDL_Renderer* renderer = nullptr;
    static SDL_Window* windowOwner = nullptr;
    static Runtime::Level* m_level = nullptr;
    static bool isQuiting;
    static ScoreWatcher _wwatcher = {};

    void internal_init_TimeEngine()
    {
        internal_game_time = 0;
        internal_time_scale = 1;
        internal_start_engine_time = 0;
        internal_start_engine_time = TimeEngine::startUpTime();
    }

    void internal_reseting()
    {
        mouseState = 0;
        mouseWheels = 0;
    }

    void Application::init()
    {
        char errorStr[128];
        if (m_inited)
            return;

        if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO))
            fail("Fail init main system.");

        // init graphics
        if (!IMG_Init(IMG_InitFlags::IMG_INIT_PNG | IMG_InitFlags::IMG_INIT_JPG))
            fail("Fail init imageformats. (libPNG, libJPG) not defined");

        // init Audio system
        if (!Mix_Init(MIX_InitFlags::MIX_INIT_OGG | MIX_InitFlags::MIX_INIT_MP3))
            fail("Fail init audio.");

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512))
            fail("Fail open audio.");

        /*if (initResources) {
            std::string path = getDataFrom(FolderKind::LOADER);
            std::string temp = path + "graphics.conf";
            GC::LoadImages(temp.c_str());

            // load textures
            path = getDataFrom(FolderKind::LOADER);
            temp = path + "sprites.conf";
            GC::LoadImages(temp.c_str());

            // Загрузка шрифта и оптимизация дэффектов
            UI::Initialize_Fonts(true);
            Levels::Level_Init();
        }*/
        // Инициализация инструментов
        UI::ui_controls_init();

        // Set cursor
        // SDL_SetCursor(GC::GetCursor("cursor", {1, 1}));
        m_inited = true;
        isQuiting = false;
    }

    void Application::create_window(const int& width, const int& height, bool fullscreen)
    {
        if (!m_inited || windowOwner)
            Application::fail("Application not Inited");

        std::uint32_t __flags = SDL_WINDOW_SHOWN;

        if (fullscreen)
            __flags |= SDL_WINDOW_FULLSCREEN;

        windowOwner = SDL_CreateWindow("Ronin Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, __flags);
        if (!windowOwner)
            fail(SDL_GetError());

        renderer = SDL_CreateRenderer(windowOwner, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE /*| SDL_RENDERER_PRESENTVSYNC*/);

        if (!renderer)
            fail(SDL_GetError());

        //        SDL_RendererInfo rfio;
        //        SDL_GetRendererInfo(renderer, &rfio);

        // Brightness - Яркость
        SDL_SetWindowBrightness(windowOwner, 1);
    }

    void Application::request_quit() { isQuiting = true; }

    void Application::load_level(Level* level)
    {
        if (windowOwner == nullptr || !level || m_level == level)
            throw std::bad_cast();

        if (m_level) {
            destroyableLevel = m_level;
            m_level->request_unload();
        }

        if (!level->is_hierarchy()) {
            // init main object
            level->main_object = create_empty_gameobject();
            level->main_object->name("Main Object");
            level->main_object->transform()->name("Root");
            // pickup from renders
            level->matrix_nature_pickup(level->main_object->transform());
        }

        m_level = level;
        m_levelAccept = false;
        m_levelLoaded = false;
    }

    SDL_Surface* Application::get_screen()
    {
        SDL_Rect rect;
        void* pixels;
        int pitch;

        SDL_RenderGetViewport(renderer, &rect);
        pitch = (rect.w - rect.x) * 4;
        pixels = malloc(pitch * (rect.h - rect.y));
        // read the Texture buffer
        SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA8888, pixels, pitch);

        SDL_Surface* su = SDL_CreateRGBSurfaceFrom(pixels, pitch / 4, rect.h - rect.y, 32, pitch, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        return su;
    }

    void Application::get_screen(const char* filename)
    {
        SDL_Surface* surf = get_screen();
        IMG_SavePNG(surf, filename);
        SDL_FreeSurface(surf);
    }

    SDL_DisplayMode Application::get_display_mode()
    {
        SDL_DisplayMode display;
        SDL_GetWindowDisplayMode(windowOwner, &display);
        return display;
    }

    RoninEngine::Resolution RoninEngine::Application::get_resolution()
    {
        Resolution res;
        SDL_RendererInfo rf;
        SDL_GetRendererInfo(renderer, &rf);

        SDL_GetWindowSize(windowOwner, &res.width, &res.height);
        return res;
    }

    ScoreWatcher Application::get_watches() { return _wwatcher; }

    bool Application::simulate()
    {

        char windowTitle[96];
        float fps, fpsRound = 0;
        int delayed = 0;
        SDL_Event event;
        float secPerFrame, game_time_score;
        SDL_DisplayMode displayMode;
        SDL_WindowFlags flags;

        if (m_level == nullptr) {
            show_message("Level not loaded");
            return false;
        }

        if (windowOwner == nullptr) {
            show_message("Window not inited");
            return false;
        }

        internal_init_TimeEngine();
        flags = static_cast<SDL_WindowFlags>(SDL_GetWindowFlags(windowOwner));
        displayMode = Application::get_display_mode();
        secPerFrame = 1000.f / displayMode.refresh_rate; // refresh screen from Monitor Settings
        game_time_score = secPerFrame * 0.001f;
        while (!isQuiting) {
            // TODO: m_level->request_unloading use as WNILE block (list proc)

            // delaying
            SDL_Delay(delayed);

            // update events
            internal_reseting();

            TimeEngine::begin_watch();

            delayed = TimeEngine::tick_millis();

            TimeEngine::begin_watch();

            while (SDL_PollEvent(&event)) {
                internal_update_input(&event);
                if (event.type == SDL_QUIT)
                    isQuiting = true;
            }

            // set default color
            SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, SDL_ALPHA_OPAQUE); // back color for clear

            // Clearing
            SDL_RenderClear(renderer);

            input_movement_update();

            if (!m_levelLoaded) {
                // free cache
                if (destroyableLevel) {
                    destroyableLevel->on_unloading();

                    RoninMemory::free(destroyableLevel);
                    destroyableLevel = nullptr;
                }

                SDL_RenderFlush(renderer); // flush renderer before first render

                // Start first init
                m_level->on_awake();
                m_levelLoaded = true;
            }
            _wwatcher.ms_wait_internal_instructions = TimeEngine::end_watch();

            // update level
            if (!m_level->request_unloading) {
                // begin watcher
                TimeEngine::begin_watch();

                if (!m_levelAccept) {

                    m_level->on_start();
                    m_levelAccept = true;
                }
                if (!m_level->request_unloading) {

                    m_level->on_update();

                    // end watcher
                    _wwatcher.ms_wait_exec_level = TimeEngine::end_watch();

                } else {
                    // end watcher
                    _wwatcher.ms_wait_exec_level = TimeEngine::end_watch();

                    goto end_simulate; // break on unload state
                }
            } else
                goto end_simulate; // break on unload state

            m_level->level_render_world(renderer, &_wwatcher);

            // begin watcher
            TimeEngine::begin_watch();
            if (Camera::main_camera()) {
                if (m_level->_realtimeScripts) {
                    for (auto n : *m_level->_realtimeScripts) {
                        n->OnGizmos();
                    };
                }
                m_level->on_gizmo(); // Draw gizmos
            }
            _wwatcher.ms_wait_render_gizmos = TimeEngine::end_watch();
            // end watcher

            if (m_level->request_unloading)
                goto end_simulate; // break on unload state

            // Set scale as default
            SDL_RenderSetScale(renderer, 1, 1);

            // begin watcher
            TimeEngine::begin_watch();
            m_level->level_render_ui(renderer);
            _wwatcher.ms_wait_render_ui = TimeEngine::end_watch();
            // end watcher

            if (m_level->request_unloading)
                goto end_simulate; // break on unload state

            if (!destroyableLevel) {
                // begin watcher
                TimeEngine::begin_watch();
                SDL_RenderPresent(renderer);
                _wwatcher.ms_wait_render_level = TimeEngine::end_watch();
                // end watcher

                m_level->level_render_world_late(renderer);
            }

        end_simulate:
            delayed = TimeEngine::tick_millis() - delayed;

            _wwatcher.ms_wait_frame = TimeEngine::end_watch();

            ++internal_frames;

            if (!_watcher_time.empty())
                throw std::runtime_error("TimeEngine::end_watcher() - not released stack.");

            internal_delta_time = delayed / 1000.f;
            //                        if (internal_delta_time > 1) {
            //                            internal_delta_time -= 1;
            //                            internal_delta_time = Math::clamp01(internal_delta_time);
            //                        }

            internal_game_time += game_time_score;

            if (TimeEngine::startUpTime() > fpsRound) {
                // SDL_METHOD:
                // fps = internal_frames / (TimeEngine::startUpTime());
                fps = 1 / internal_delta_time;
                std::sprintf(
                    windowTitle,
                    "FPS:%d Memory:%sMiB, "
                    "Ronin_Allocated:%s, SDL_Allocated:%s, Frames:%u",
                    static_cast<int>(fps), Math::num_beautify(get_process_sizeMemory() / 1024 / 1024).c_str(), Math::num_beautify(RoninMemory::total_allocated()).c_str(), Math::num_beautify(SDL_GetNumAllocations()).c_str(), internal_frames);
                SDL_SetWindowTitle(Application::get_window(), windowTitle);
                fpsRound = TimeEngine::startUpTime() + .5f; // updater per 1 seconds
            }

            if (m_level->request_unloading)
                break; // break on unload state

            // delay elapsed
            delayed = Math::max(0, static_cast<int>(Math::ceil(secPerFrame) - delayed));
        }

        // unload level
        Runtime::unload_level(m_level);

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(windowOwner);

        UI::ui_free_controls();
        // sdl quit
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();

        m_inited = false;
        int r = Runtime::RoninMemory::total_allocated();
        return isQuiting;
    }

    SDL_Window* Application::get_window() { return windowOwner; }

    SDL_Renderer* Application::get_renderer() { return renderer; }

    void Application::back_fail(void) { exit(EXIT_FAILURE); }

    void Application::show_message(const std::string& message)
    {
        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE, "%s", message.c_str());
        printf("%s\n", message.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, nullptr, message.c_str(), windowOwner);
    }

    void Application::fail(const std::string& message)
    {
        std::string _template = message;
        char _temp[32] { 0 };
        tm* ltime;
        time_t tt;
        time(&tt);

        ltime = localtime(&tt);
        strftime(_temp, sizeof(_temp), "%d.%m.%y %H:%M:%S", ltime);
        _template += "\n\n\t";
        _template += _temp;

        fprintf(stderr, "%s", _template.data());

        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_CRITICAL, "%s", _template.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, "Ronin Engine: failed", _template.c_str(), windowOwner);
        back_fail();
    }

    void Application::fail_oom_kill() throw() { fail("Out of memory!"); }
} // namespace RoninEngine
