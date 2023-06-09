#include "ronin.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine
{
    namespace Runtime
    {
        World* switched_world;

        float internal_time_scale, internal_game_time, internal_delta_time;

        std::uint32_t internal_start_engine_time;
        std::uint32_t internal_frames; // framecounter
        std::vector<std::uint32_t> _watcher_time;

        Vec2Int internal_mouse_point;
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

        extern void load_world(World* world);
        extern bool unload_world(World* world);
    }

    namespace UI
    {
        extern void ui_controls_init();
        extern void ui_free_controls();
        extern void ui_reset_controls();
    }

    static bool m_inited = false;
    static bool internal_level_loaded = false;
    static bool m_levelAccept = false;
    static Runtime::World* destroyableLevel = nullptr;
    static SDL_Renderer* renderer = nullptr;
    static SDL_Window* windowOwner = nullptr;
    static bool isQuiting;
    static ScoreWatcher _wwatcher = {};

    void internal_init_timer()
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

        // Init level
        switched_world = nullptr;

        // Инициализация инструментов
        UI::ui_controls_init();

        // Set cursor
        // SDL_SetCursor(GC::GetCursor("cursor", {1, 1}));
        m_inited = true;
        isQuiting = false;
    }

    void Application::create_window(const int width, const int height, bool fullscreen)
    {
        if (!m_inited)
            Application::fail("Application not Inited");

        if (windowOwner)
            Application::fail("Window exists, active self");

        std::uint32_t __flags = SDL_WINDOW_SHOWN;

        if (fullscreen)
            __flags |= SDL_WINDOW_FULLSCREEN;

        windowOwner = SDL_CreateWindow("Ronin Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, __flags);
        if (!windowOwner)
            fail(SDL_GetError());

        // Brightness - Яркость
        SDL_SetWindowBrightness(windowOwner, 1);
    }

    void Application::request_quit() { isQuiting = true; }

    void Application::load_world(World* world, bool unloadPrevious)
    {
        if (windowOwner == nullptr || world == nullptr)
            Application::fail("world is null object, widowOwner have-nullpointer");

        if (switched_world == world)
            Application::fail("Current world is reloading state. Failed.");

        if (switched_world) {
            destroyableLevel = switched_world;
            destroyableLevel->request_unload();
        }

        // switching as main
        switched_world = world;

        // on swtiched and init resources, even require
        Runtime::load_world(world);

        // set state load
        world->internal_resources->request_unloading = false;

        if (!world->is_hierarchy()) {
            // init main object
            world->internal_resources->main_object = create_empty_gameobject();
            world->internal_resources->main_object->name("Main Object");
            world->internal_resources->main_object->transform()->name("Root");
            // pickup from renders
            world->matrix_nature_pickup(world->internal_resources->main_object->transform());
        }
        m_levelAccept = false;
        internal_level_loaded = false;
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

        if (switched_world == nullptr) {
            show_message("World not loaded");
            return false;
        }

        if (windowOwner == nullptr) {
            show_message("Window not inited");
            return false;
        }

        flags = static_cast<SDL_WindowFlags>(SDL_GetWindowFlags(windowOwner));
        displayMode = Application::get_display_mode();
        secPerFrame = 1000.f / displayMode.refresh_rate; // refresh screen from Monitor Settings
        game_time_score = secPerFrame / 1000;
        while (!isQuiting) {
            // TODO: m_level->request_unloading use as WNILE block (list proc)

            if (!internal_level_loaded) {
                // free cache
                if (destroyableLevel) {
                    // RoninMemory::free(destroyableLevel);
                    SDL_DestroyRenderer(renderer);
                    destroyableLevel = nullptr;
                }
                renderer = SDL_CreateRenderer(windowOwner, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE /*| SDL_RENDERER_PRESENTVSYNC*/);

                if (!renderer)
                    fail(SDL_GetError());

                //        SDL_RendererInfo rfio;
                //        SDL_GetRendererInfo(renderer, &rfio);
                internal_init_timer();

                // on first load level
                switched_world->on_awake();
                internal_level_loaded = true;
            }

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

            _wwatcher.ms_wait_internal_instructions = TimeEngine::end_watch();

            // update level
            if (!switched_world->internal_resources->request_unloading) {
                // begin watcher
                TimeEngine::begin_watch();

                if (!m_levelAccept) {

                    switched_world->on_start();
                    m_levelAccept = true;
                }
                if (!switched_world->internal_resources->request_unloading) {

                    switched_world->on_update();

                    // end watcher
                    _wwatcher.ms_wait_exec_level = TimeEngine::end_watch();

                } else {
                    // end watcher
                    _wwatcher.ms_wait_exec_level = TimeEngine::end_watch();

                    goto end_simulate; // break on unload state
                }
            } else
                goto end_simulate; // break on unload state

            switched_world->level_render_world(renderer, &_wwatcher);

            if (switched_world->internal_resources->request_unloading)
                goto end_simulate; // break on unload state

            // Set scale as default
            SDL_RenderSetScale(renderer, 1, 1);

            // begin watcher
            TimeEngine::begin_watch();
            switched_world->level_render_ui(renderer);
            _wwatcher.ms_wait_render_ui = TimeEngine::end_watch();
            // end watcher

            if (switched_world->internal_resources->request_unloading)
                goto end_simulate; // break on unload state

            if (!destroyableLevel) {
                // begin watcher
                TimeEngine::begin_watch();
                SDL_RenderPresent(renderer);
                _wwatcher.ms_wait_render_level = TimeEngine::end_watch();
                // end watcher

                switched_world->level_render_world_late(renderer);
            }

        end_simulate:
            // awake
            SDL_RenderFlush(renderer); // flush renderer

            delayed = TimeEngine::tick_millis() - delayed;

            _wwatcher.ms_wait_frame = TimeEngine::end_watch();

            ++internal_frames;

            if (!_watcher_time.empty())
                throw std::runtime_error("TimeEngine::end_watcher() - not released stack.");

            internal_delta_time = Math::min<float>(1.f, Math::max<float>(delayed / 1000.f, game_time_score));
            internal_game_time += internal_delta_time;

            if (TimeEngine::startUpTime() > fpsRound) {
                // SDL_METHOD:
                // fps = internal_frames / (TimeEngine::startUpTime());
                fps = 1 / internal_delta_time;
                std::sprintf(
                    windowTitle,
                    "FPS:%.1f Memory:%sMiB, "
                    "Ronin_Allocated:%s, SDL_Allocated:%s, Frames:%u",
                    fps, Math::num_beautify(get_process_sizeMemory() / 1024 / 1024).c_str(), Math::num_beautify(RoninMemory::total_allocated()).c_str(), Math::num_beautify(SDL_GetNumAllocations()).c_str(), internal_frames);
                SDL_SetWindowTitle(Application::get_window(), windowTitle);
                fpsRound = TimeEngine::startUpTime() + .5f; // updater per 1 seconds
            }

            if (switched_world->internal_resources->request_unloading)
                break; // break on unload state

            // delay elapsed
            delayed = Math::max(0, static_cast<int>(Math::ceil(secPerFrame) - delayed));
        }

        // unload level
        Runtime::unload_world(switched_world);

        switched_world = nullptr;

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(windowOwner);

        UI::ui_free_controls();

        Mix_Quit();
        IMG_Quit();
        SDL_Quit();

        m_inited = false;
        int memory_leak = Runtime::RoninMemory::total_allocated();
        if (memory_leak > 0)
            SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ronin-memory-leak count: %d", memory_leak);
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
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Ronin Engine: failed", _template.c_str(), windowOwner);
        back_fail();
    }

    void Application::fail_oom_kill() throw() { fail("Out of memory!"); }
} // namespace RoninEngine
