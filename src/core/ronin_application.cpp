#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace UI
    {
        extern void free_fonts();
        extern void ui_reset_controls();
        extern void native_draw_render(GUI *);
    } // namespace UI

    namespace Runtime
    {
#if TEST_MALLOC
        namespace RoninMemory
        {
            extern std::list<void *> allocated_leaker;
        }
#endif
        World *switched_world;

        GidResources *external_global_resources = nullptr;

        float internal_time_scale, internal_game_time, internal_delta_time;

        std::uint32_t internal_start_engine_time;
        std::uint32_t internal_frames; // framecounter
        std::vector<std::uint32_t> _watcher_time;
        int _matrix_threshold_ = 2;
        RoninInput internal_input;
        bool text_inputState;

        extern GameObject *create_empty_gameobject();

        extern void text_get(std::string &text);
        extern void text_stop_input();
        extern void text_start_input();
        extern void input_movement_update();
        extern void internal_update_input(SDL_Event *e);

        extern void load_world(World *world);
        extern bool unload_world(World *world);
    } // namespace Runtime

    SDL_Window *main_window = nullptr;
    SDL_Renderer *renderer = nullptr;
    Runtime::World *destroyableLevel = nullptr;
    Resolution active_resolution {0, 0, 0};
    TimingWatcher last_watcher = {};
    TimingWatcher queue_watcher;

    bool ronin_debug_mode = false;
    bool internal_level_loaded = false;
    bool world_can_start = false;

    void internal_init_timer()
    {
        internal_game_time = 0;
        internal_time_scale = 1;
        internal_start_engine_time = 0;
        internal_start_engine_time = TimeEngine::startUpTime();
    }

    void internal_reseting()
    {
        std::replace_if(
            internal_input._mouse_state,
            internal_input._mouse_state + sizeof(internal_input._mouse_state),
            [](auto ms_check) { return ms_check == MouseStateFlags::MouseUp; },
            0);
        internal_input._mouse_wheels = 0;
    }

    void RoninSimulator::Init(InitializeFlags flags)
    {
        std::uint32_t current_inits;

        if(main_window != nullptr)
            return;

        current_inits = SDL_INIT_EVENTS | SDL_INIT_TIMER;
        if(SDL_Init(current_inits) == -1)
            ShowMessageFail("Fail init SDL2 main system.");

        // init graphics
        if(flags & InitializeFlags::Graphics)
        {
            current_inits = SDL_INIT_VIDEO;
            if(SDL_InitSubSystem(current_inits) == -1)
                ShowMessageFail("Fail ini Video system.");

            current_inits = IMG_InitFlags::IMG_INIT_PNG | IMG_InitFlags::IMG_INIT_JPG;
            if(IMG_Init(current_inits) != current_inits)
                ShowMessageFail("Fail init Image library.");

            if(TTF_Init() == -1)
                ShowMessageFail("Fail init Font library.");
        }

        // init Audio system
        if(flags & InitializeFlags::Audio)
        {
            current_inits = MIX_InitFlags::MIX_INIT_OGG | MIX_InitFlags::MIX_INIT_MP3;
            if(Mix_Init(current_inits) != current_inits)
                ShowMessage("Fail init audio.");

            if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024))
                ShowMessage("Fail open audio.");
        }

        // setup
        switched_world = nullptr;

        // inti fonts
        UI::init_fonts(true);
    }

    void RoninSimulator::Finalize()
    {
        if(main_window == nullptr)
            return;
        SDL_DestroyWindow(main_window);
        main_window = nullptr;

        UI::free_fonts();

        // NOTE: Free Global Resources
        gid_resources_free(external_global_resources);
        external_global_resources = nullptr;

        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        TTF_Quit();

        int memory_leak = Runtime::RoninMemory::total_allocated();
        if(memory_leak > 0)
        {
#if TEST_MALLOC
            auto REF = (RoninEngine::Runtime::RoninMemory::allocated_leaker.back());
            auto o = reinterpret_cast<Object *>(REF);
            auto c = reinterpret_cast<Component *>(REF);
            auto w = reinterpret_cast<World *>(REF);
            auto i = reinterpret_cast<int *>(REF);
            auto s = reinterpret_cast<char *>(REF);
            auto d = reinterpret_cast<double *>(REF);
            auto f = reinterpret_cast<float *>(REF);
            auto l = reinterpret_cast<long *>(REF);
            auto n = reinterpret_cast<RoninEngine::AIPathFinder::NavMesh *>(REF);
            auto nav_container = reinterpret_cast<RoninEngine::AIPathFinder::NavContainer *>(REF);
#endif
            SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ronin-memory-leak count: %d", memory_leak);
        }

        memory_leak = SDL_GetNumAllocations();
        if(memory_leak > 0)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "sdl-memory-leak count: %d", memory_leak);
        }
    }

    void RoninSimulator::Show(const RoninEngine::Resolution &resolution, bool fullscreen)
    {
        if(main_window)
            return;

        std::uint32_t __flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;

        if(fullscreen)
            __flags |= SDL_WINDOW_FULLSCREEN;

        main_window = SDL_CreateWindow("Ronin Engine", 0, 0, resolution.width, resolution.height, __flags);

        // Get valid resolution size
        SDL_DisplayMode mode;
        SDL_GetWindowDisplayMode(main_window, &mode);
        SDL_SetWindowSize(main_window, mode.w, mode.h);
        SDL_SetWindowPosition(main_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        SDL_ShowWindow(main_window);
        if(!main_window)
            ShowMessageFail(SDL_GetError());

        // get activated resolution
        active_resolution = GetCurrentResolution();

        // Brightness - Яркость
        SDL_SetWindowBrightness(main_window, 1.f);
    }

    void RoninSimulator::RequestQuit()
    {
        if(switched_world)
            switched_world->RequestUnload();
    }

    void RoninSimulator::LoadWorld(World *world, bool unloadPrevious)
    {
        if(main_window == nullptr)
            RoninSimulator::ShowMessageFail("Engine not inited");

        if(world == nullptr)
        {
            RoninSimulator::ShowMessageFail("World is not inited");
        }

        if(switched_world == world)
        {
            RoninSimulator::ShowMessage("Current world is reloading state. Failed.");
            return;
        }
        if(unloadPrevious && switched_world)
        {
            destroyableLevel = switched_world;
            destroyableLevel->RequestUnload();
        }

        // switching as main
        switched_world = world;

        // on swtiched and init resources, even require
        Runtime::load_world(world);

        // set state load
        world->internal_resources->request_unloading = false;

        if(!world->isHierarchy())
        {
            // init main object
            world->internal_resources->main_object = create_empty_gameobject();
            world->internal_resources->main_object->name("Main Object");
            world->internal_resources->main_object->transform()->name("Root");
            // pickup from renders
            Matrix::matrix_nature_pickup(world->internal_resources->main_object->transform());
        }
        world_can_start = false;
        internal_level_loaded = false;
    }

    Resolution RoninSimulator::GetCurrentResolution()
    {
        if(main_window == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode display_mode;
        if(SDL_GetWindowDisplayMode(main_window, &display_mode) == -1)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return {display_mode.w, display_mode.h, display_mode.refresh_rate};
    }

    std::list<Resolution> RoninSimulator::EnumerateResolutions()
    {
        std::list<Resolution> resolutions;
        SDL_DisplayMode mode;

        int ndisplay_modes = SDL_GetNumDisplayModes(0);
        for(int x = 0; x < ndisplay_modes; ++x)
        {
            if(SDL_GetDisplayMode(0, x, &mode) == -1)
            {
                RoninSimulator::ShowMessageFail(SDL_GetError());
            }
            resolutions.emplace_back(mode.w, mode.h, mode.refresh_rate);
        }
        return resolutions;
    }

    bool RoninSimulator::SetDisplayResolution(const Resolution &new_resolution)
    {
        if(main_window == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode mode;

        mode.w = new_resolution.width;
        mode.h = new_resolution.height;
        mode.refresh_rate = new_resolution.hz;
        bool result = SDL_SetWindowDisplayMode(main_window, &mode) == 0;
        if(result)
        {
            active_resolution = new_resolution;
        }
        return result;
    }

    bool RoninSimulator::SetFullscreenMode(FullscreenMode mode)
    {
        if(main_window == nullptr)
        {
            throw ronin_init_error();
        }

        std::uint32_t flags = SDL_GetWindowFlags(main_window);
        flags &= ~(SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);

        switch(mode)
        {
            case FullscreenMode::Desktop:
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
                break;
            case FullscreenMode::Display:
                flags |= SDL_WINDOW_FULLSCREEN;
                break;
            default:
                break;
        }

        return SDL_SetWindowFullscreen(main_window, flags) == 0;
    }

    TimingWatcher RoninSimulator::GetTimingWatches()
    {
        return last_watcher;
    }

    VersionInfo RoninSimulator::GetVersion()
    {
        VersionInfo version;
        SDL_version linked;
        SDL_GetVersion(&linked);

        version.Engine_Version = {}; // TODO: Get Current Linked version
        SDL_memcpy(&version.SDL_Version, &linked, sizeof(Version));
        SDL_memcpy(&version.SDL_TTF_Version, TTF_Linked_Version(), sizeof(Version));
        SDL_memcpy(&version.SDL_IMG_Version, IMG_Linked_Version(), sizeof(Version));
        SDL_memcpy(&version.SDL_Mix_Version, Mix_Linked_Version(), sizeof(Version));

        return version;
    }

    const char *RoninSimulator::GetLinkedTargets()
    {
        return RGE_LINKED_TARGETS;
    }

    void RoninSimulator::SetDebugMode(bool state)
    {
        ronin_debug_mode = state;
    }

    void RoninSimulator::Simulate()
    {
        bool isQuiting = false;
        char windowTitle[96];
        float fps, fpsRound = 0;
        std::uint64_t delayed = 0;
        SDL_Event event;
        float secPerFrame, game_time_score;
        SDL_DisplayMode displayMode;
        SDL_WindowFlags flags;

        if(switched_world == nullptr)
        {
            ShowMessage("World not loaded");
            return;
        }

        if(main_window == nullptr)
        {
            ShowMessage("Engine not inited");
            return;
        }

        Resolution current_resolution = RoninSimulator::GetCurrentResolution();
        queue_watcher = {};
        flags = static_cast<SDL_WindowFlags>(SDL_GetWindowFlags(main_window));
        secPerFrame = 1000.f / current_resolution.hz; // refresh screen from Monitor Settings
        game_time_score = secPerFrame / 1000;
        while(!isQuiting)
        {
            // TODO: m_level->request_unloading use as WNILE block (list proc)

            TimeEngine::begin_watch();

            delayed = TimeEngine::tick_millis();

            TimeEngine::begin_watch();

            // update internal events
            SDL_PumpEvents();

            while(SDL_PollEvent(&event))
            {
                internal_update_input(&event);
                switch(event.type)
                {
                    case SDL_QUIT:
                    {
                        isQuiting = true;
                        switched_world->RequestUnload();
                        destroyableLevel = switched_world;
                        switched_world = nullptr;
                        internal_level_loaded = false;
                        break;
                    }
                }
            }

            if(internal_level_loaded == false)
            {
                // Unload old World
                if(destroyableLevel)
                {
                    unload_world(destroyableLevel);
                    destroyableLevel = nullptr;
                }
                SDL_DestroyRenderer(renderer);

                if(switched_world == nullptr)
                {
                    renderer = nullptr;
                    queue_watcher.ms_wait_internal_instructions = TimeEngine::end_watch();
                    goto end_simulate;
                }
                else
                {
                    // on first load level
                    renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
                    if(renderer == nullptr)
                        ShowMessageFail(SDL_GetError());

                    internal_init_timer();
                    switched_world->OnAwake();
                    internal_level_loaded = true;
                }
            }

            // set default color
            SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, SDL_ALPHA_OPAQUE); // back color for clear

            // Clearing
            SDL_RenderClear(renderer);

            input_movement_update();

            queue_watcher.ms_wait_internal_instructions = TimeEngine::end_watch();
            // end watcher

            // update level
            if(!switched_world->internal_resources->request_unloading)
            {
                // begin watcher
                TimeEngine::begin_watch();

                if(!world_can_start)
                {

                    switched_world->OnStart();
                    world_can_start = true;
                }

                if(!switched_world->internal_resources->request_unloading)
                {
                    switched_world->OnUpdate();
                }
                // end watcher
                queue_watcher.ms_wait_exec_world = TimeEngine::end_watch();

                if(switched_world->internal_resources->request_unloading)
                {
                    goto end_simulate; // break on unload state
                }
            }
            else
                goto end_simulate; // break on unload state

            switched_world->level_render_world();

            if(switched_world->internal_resources->request_unloading)
                goto end_simulate; // break on unload state

            // Set scale as default
            SDL_RenderSetScale(renderer, 1.f, 1.f);

            // begin watcher
            TimeEngine::begin_watch();
            UI::native_draw_render(switched_world->internal_resources->gui);
            queue_watcher.ms_wait_render_gui = TimeEngine::end_watch();
            // end watcher

            if(switched_world->internal_resources->request_unloading)
                goto end_simulate; // break on unload state

            if(!destroyableLevel)
            {
                // begin watcher
                TimeEngine::begin_watch();
                SDL_RenderPresent(renderer);
                queue_watcher.ms_wait_render_world += TimeEngine::end_watch();
                // end watcher

                switched_world->level_render_world_late();
            }

        end_simulate:

            delayed = TimeEngine::tick_millis() - delayed;

            queue_watcher.ms_wait_frame = TimeEngine::end_watch();

            // update watcher
            last_watcher = queue_watcher;

            ++internal_frames;

            if(!_watcher_time.empty())
                throw ronin_watcher_error();

            internal_delta_time = Math::min<float>(1.f, Math::max<float>(delayed / 1000.f, game_time_score));
            internal_game_time += internal_delta_time;

            if(TimeEngine::startUpTime() > fpsRound)
            {
                // SDL_METHOD:
                // fps = internal_frames / (TimeEngine::startUpTime());
                fps = 1 / internal_delta_time;
                std::sprintf(
                    windowTitle,
                    "FPS:%.1f Memory:%sMiB, "
                    "Ronin_Allocated:%s, SDL_Allocated:%s, Frames:%s",
                    fps,
                    Math::num_beautify(get_process_sizeMemory() / 1024 / 1024).c_str(),
                    Math::num_beautify(RoninMemory::total_allocated()).c_str(),
                    Math::num_beautify(SDL_GetNumAllocations()).c_str(),
                    Math::num_beautify(internal_frames).c_str());
                SDL_SetWindowTitle(main_window, windowTitle);
                fpsRound = TimeEngine::startUpTime() + .5f; // updater per 1 seconds
            }

            // delay elapsed
            delayed = Math::max(0, static_cast<int>(Math::ceil(secPerFrame) - delayed));

            if(switched_world == nullptr || switched_world->internal_resources->request_unloading)
                break; // break on unload state

            // update events
            internal_reseting();

            // delaying
            SDL_Delay(delayed);
        }
        if(switched_world)
        {
            Runtime::unload_world(switched_world);
            switched_world = nullptr;
        }
        // unload level
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    void RoninSimulator::ShowMessage(const std::string &message)
    {
        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE, "%s", message.c_str());
        printf("%s\n", message.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, nullptr, message.c_str(), main_window);
    }

    void RoninSimulator::ShowMessageFail(const std::string &message)
    {
        std::string _template = message;
        char _temp[32] {0};
        tm *ltime;
        time_t tt;
        time(&tt);

        ltime = localtime(&tt);
        strftime(_temp, sizeof(_temp), "%d.%m.%y %H:%M:%S", ltime);
        _template += "\n\n\t";
        _template += _temp;

        fprintf(stderr, "%s", _template.data());

        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_CRITICAL, "%s", _template.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Ronin Engine: failed", _template.c_str(), main_window);
        Kill();
    }

    void RoninSimulator::BreakSimulate()
    {
        ShowMessageFail("Out of memory!");
    }

    void RoninSimulator::Kill(void)
    {
        exit(EXIT_FAILURE);
    }

} // namespace RoninEngine
