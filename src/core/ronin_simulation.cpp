#include "ronin.h"
#include "ronin_matrix.h"
#include "ronin_audio.h"

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
        World *preload_world;
        World *last_switched_world;
        std::set<World *> pinned_worlds;
        std::vector<std::uint32_t> _watcher_time;

        GidResources *external_global_resources = nullptr;

        float internal_time_scale, internal_game_time, internal_delta_time;

        std::uint32_t internal_start_engine_time;
        std::uint32_t internal_frames; // framecounter
        int _matrix_pack_ = 1;
        RoninInput internal_input;
        bool text_inputState;

        extern GameObject *create_empty_gameobject();

        extern void text_get(std::string &text);
        extern void text_stop_input();
        extern void text_start_input();
        extern void input_movement_update();
        extern void internal_update_input(SDL_Event *e);

        extern void internal_load_world(World *world);
        extern bool internal_unload_world(World *world);
    } // namespace Runtime

    enum ConfigState : int
    {
        CONF_RENDER_NOCONF = 0,

        CONF_RENDER_CHANGED = 1,
        CONF_RELOAD_WORLD = 2,
        CONF_RESERVED2 = 4,
        CONF_RENDER_SOFTWARE = CONF_RENDER_CHANGED | 8,
    };
    struct
    {
        // this is variable for apply settings
        int conf;
        SDL_Surface *software_surface = nullptr;
        SDL_Renderer *renderer_hardware = nullptr;
        SDL_Renderer *renderer_software = nullptr;
    } simConfig;

    SDL_Renderer *renderer = nullptr;

    SDL_Window *active_window = nullptr;
    Resolution active_resolution {0, 0, 0};
    TimingWatcher last_watcher {};
    TimingWatcher queue_watcher {};

    bool ronin_debug_mode = false;
    bool internal_world_loaded = false;
    bool internal_world_can_start = false;

    void internal_apply_settings()
    {
        if(simConfig.conf == CONF_RENDER_NOCONF)
            return;

        if(simConfig.conf & CONF_RENDER_CHANGED)
        {
            if(simConfig.conf & CONF_RENDER_SOFTWARE == CONF_RENDER_SOFTWARE)
            {
                renderer = simConfig.renderer_software;
            }
            else
            {
                renderer = simConfig.renderer_hardware;
            }
        }

        if(simConfig.conf & CONF_RELOAD_WORLD)
        {
            last_switched_world = switched_world; // Switched world first unload after load
            preload_world = switched_world;       // Switched world as Newer is preload

            internal_world_loaded = false;
        }

        simConfig.conf = CONF_RENDER_NOCONF;
    }

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

        if(active_window != nullptr)
            return;

        current_inits = SDL_INIT_EVENTS | SDL_INIT_TIMER;
        if(SDL_Init(current_inits) == -1)
            ShowMessageFail("Fail init SDL2 main system.");

        // init graphics
        if(flags & InitializeFlags::Graphics)
        {
            current_inits = SDL_INIT_VIDEO;

            if(SDL_InitSubSystem(current_inits) == -1)
                ShowMessageFail("Fail init Video system.");

            current_inits = IMG_InitFlags::IMG_INIT_PNG | IMG_InitFlags::IMG_INIT_JPG;
            if(IMG_Init(current_inits) != current_inits)
                ShowMessageFail("Fail init Image library.");

            if(TTF_Init() == -1)
                ShowMessageFail("Fail init Font library.");
        }

        // init Audio system
        if(flags & InitializeFlags::Audio)
        {
            if(Runtime::RoninAudio::Init() == -1)
                ShowMessage("Fail init audio.");
        }

        // setup
        switched_world = nullptr;

        // inti fonts
        UI::init_legacy_font(true);
    }

    void RoninSimulator::Finalize()
    {
        if(active_window == nullptr)
            return;

        // unload existence worlds
        for(auto &pinned : Runtime::pinned_worlds)
        {
            Runtime::internal_unload_world(pinned);
        }

        Runtime::pinned_worlds.clear();

        SDL_DestroyWindow(active_window);
        active_window = nullptr;

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
        if(active_window)
            return;

        std::uint32_t __flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;

        if(fullscreen)
            __flags |= SDL_WINDOW_FULLSCREEN;

        active_window = SDL_CreateWindow("Ronin Engine", 0, 0, resolution.width, resolution.height, __flags);

        // Get valid resolution size
        SDL_DisplayMode mode;
        SDL_GetWindowDisplayMode(active_window, &mode);
        SDL_SetWindowSize(active_window, mode.w, mode.h);
        SDL_SetWindowPosition(active_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        SDL_ShowWindow(active_window);
        if(!active_window)
            ShowMessageFail(SDL_GetError());

        // get activated resolution
        active_resolution = GetCurrentResolution();

        // Brightness
        SDL_SetWindowBrightness(active_window, 1.f);
    }

    void RoninSimulator::RequestQuit()
    {
        if(switched_world)
            switched_world->RequestUnload();
    }

    bool RoninSimulator::LoadWorld(World *world, bool unloadPrevious)
    {
        const std::function<bool(void)> checks_queue[] {
            [=]() -> bool const
            {
                bool hasError = active_window == nullptr;
                if(hasError)
                    Log("Engine not inited");
                return hasError;
            },
            [=]() -> bool const
            {
                bool hasError = world == nullptr;
                if(hasError)
                    Log("World is not defined");
                return hasError;
            },
            [=]() -> bool const
            {
                bool hasError = switched_world == world || last_switched_world != nullptr || preload_world == world;
                if(hasError)
                    Log("Current world is reloading state. Failed.");
                return hasError;
            }};

        // Run checks
        for(const auto &err : checks_queue)
        {
            if(err())
            {
                return false;
            }
        }

        if(unloadPrevious && switched_world)
        {
            last_switched_world = switched_world;
            last_switched_world->RequestUnload();
        }

        // cancelation of reload
        CancelReload();

        // switching as main
        if(switched_world == nullptr)
            switched_world = world;

        // preload world
        preload_world = world;

        internal_world_can_start = false;
        internal_world_loaded = false;

        return true;
    }

    bool RoninSimulator::ReloadWorld()
    {
        if(switched_world == nullptr || preload_world != nullptr)
        {
            Log("Active world not loaded");
            return false;
        }

        // set config for reload
        simConfig.conf |= CONF_RELOAD_WORLD;
        return true;
    }

    bool RoninSimulator::CancelReload()
    {
        int last_flag = (simConfig.conf & CONF_RELOAD_WORLD);
        // clear reload flag
        simConfig.conf &= ~CONF_RELOAD_WORLD;
        return last_flag != 0;
    }

    Resolution RoninSimulator::GetCurrentResolution()
    {
        if(active_window == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode display_mode;
        if(SDL_GetWindowDisplayMode(active_window, &display_mode) == -1)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return {display_mode.w, display_mode.h, display_mode.refresh_rate};
    }

    std::list<Resolution> RoninSimulator::EnumerateResolutions()
    {
        std::list<Resolution> resolutions;
        SDL_DisplayMode mode;

        if(active_window == nullptr)
        {
            throw ronin_init_error();
        }

        int active_display = SDL_GetWindowDisplayIndex(active_window);
        if(active_display < 0)
        {
            RoninSimulator::ShowMessage(SDL_GetError());
        }
        else
        {
            int n, ndisplay_modes = SDL_GetNumDisplayModes(active_display);
            for(n = 0; n < ndisplay_modes; ++n)
            {
                if(SDL_GetDisplayMode(active_display, n, &mode) == -1)
                {
                    RoninSimulator::ShowMessageFail(SDL_GetError());
                }
                resolutions.emplace_back(mode.w, mode.h, mode.refresh_rate);
            }
        }
        return resolutions;
    }

    bool RoninSimulator::SetWindowResolution(const Resolution &new_resolution)
    {
        if(active_window == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode mode;

        mode.w = new_resolution.width;
        mode.h = new_resolution.height;
        mode.refresh_rate = new_resolution.hz;
        bool result = SDL_SetWindowDisplayMode(active_window, &mode) == 0;
        if(result)
        {
            active_resolution = new_resolution;
        }
        else
        {
            RoninSimulator::ShowMessage(SDL_GetError());
        }
        return result;
    }

    bool RoninSimulator::SetFullscreenMode(FullscreenMode mode)
    {
        if(active_window == nullptr)
        {
            throw ronin_init_error();
        }

        std::uint32_t flags = SDL_GetWindowFlags(active_window);
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

        return SDL_SetWindowFullscreen(active_window, flags) == 0;
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

        version.Engine_Version = {0, 0, 0}; // TODO: Get Current Linked version
        SDL_memcpy(&version.SDL_Version, &linked, sizeof(Version));
        SDL_memcpy(&version.SDL_TTF_Version, TTF_Linked_Version(), sizeof(Version));
        SDL_memcpy(&version.SDL_IMG_Version, IMG_Linked_Version(), sizeof(Version));
        SDL_memcpy(&version.SDL_Mix_Version, Mix_Linked_Version(), sizeof(Version));
        version.SDL_GFX_Version = {SDL2_GFXPRIMITIVES_MAJOR, SDL2_GFXPRIMITIVES_MINOR, SDL2_GFXPRIMITIVES_MICRO};

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
        SDL_Event event;
        SDL_DisplayMode displayMode;

        bool isQuiting = false;
        char title[96];
        float fps = 0;
        float secPerFrame, game_time_score;
        std::uint64_t delayed = 0;

        if(switched_world == nullptr)
        {
            ShowMessage("World not loaded");
            return;
        }

        if(active_window == nullptr)
        {
            ShowMessage("Engine not inited");
            return;
        }

        Resolution current_resolution = RoninSimulator::GetCurrentResolution();
        queue_watcher = {};

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
                        last_switched_world = switched_world;
                        simConfig.conf = CONF_RENDER_NOCONF;
                        switched_world = nullptr;
                        internal_world_loaded = false;
                        break;
                    }
                }
            }

            // Apply changed settings
            internal_apply_settings();

            if(!internal_world_loaded)
            {
                // Unload old World
                if(last_switched_world)
                {
                    internal_unload_world(last_switched_world);
                    last_switched_world = nullptr;
                }

                // Destroy last renderer
                SDL_DestroyRenderer(simConfig.renderer_hardware);

                if(switched_world == nullptr)
                {
                    renderer = nullptr;
                    simConfig.renderer_hardware = nullptr;

                    queue_watcher.ms_wait_internal_instructions = TimeEngine::end_watch();
                    goto end_simulate;
                }
                else
                {
                    // on first load level
                    renderer = simConfig.renderer_hardware =
                        SDL_CreateRenderer(active_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

                    if(renderer == nullptr)
                        ShowMessageFail(SDL_GetError());

                    // on swtiched and init resources, even require
                    Runtime::internal_load_world(preload_world);

                    switched_world = preload_world;

                    // Init Internal World Timer IIWT
                    internal_init_timer();

                    if(!switched_world->isHierarchy())
                    {
                        // init main object
                        switched_world->irs->main_object = create_empty_gameobject();
                        switched_world->irs->main_object->name("Main Object");
                        switched_world->irs->main_object->transform()->name("Root");
                        // pickup from renders
                        Matrix::matrix_nature_pickup(switched_world->irs->main_object->transform());
                    }

                    internal_world_can_start = false;
                    switched_world->OnAwake();

                    preload_world = nullptr;
                    internal_world_loaded = true;
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
            if(!switched_world->irs->request_unloading)
            {
                // begin watcher
                TimeEngine::begin_watch();

                if(!internal_world_can_start)
                {
                    switched_world->OnStart();
                    internal_world_can_start = true;
                }

                if(!switched_world->irs->request_unloading)
                {
                    switched_world->OnUpdate();
                }
                // end watcher
                queue_watcher.ms_wait_exec_world = TimeEngine::end_watch();

                if(switched_world->irs->request_unloading)
                {
                    goto end_simulate; // break on unload state
                }
            }
            else
                goto end_simulate; // break on unload state

            level_render_world();

            if(switched_world->irs->request_unloading)
                goto end_simulate; // break on unload state

            // Set scale as default
            SDL_RenderSetScale(renderer, 1.f, 1.f);

            // begin watcher
            TimeEngine::begin_watch();
            UI::native_draw_render(switched_world->irs->gui);
            queue_watcher.ms_wait_render_gui = TimeEngine::end_watch();
            // end watcher

            if(switched_world->irs->request_unloading)
                goto end_simulate; // break on unload state

            if(!last_switched_world)
            {
                // begin watcher
                TimeEngine::begin_watch();
                SDL_RenderPresent(renderer);
                queue_watcher.ms_wait_render_world += TimeEngine::end_watch();
                // end watcher

                level_render_world_late();
            }

        end_simulate:

            delayed = TimeEngine::tick_millis() - delayed;

            queue_watcher.ms_wait_frame = TimeEngine::end_watch();

            // update watcher
            last_watcher = queue_watcher;

            ++internal_frames;

            if(!_watcher_time.empty())
                throw ronin_watcher_error();

            internal_delta_time = Math::Min<float>(1.f, Math::Max<float>(delayed / 1000.f, game_time_score));
            internal_game_time += internal_delta_time;

            if(ronin_debug_mode && TimeEngine::startUpTime() > fps)
            {
                // SDL_METHOD:
                // fps = internal_frames / (TimeEngine::startUpTime());
                fps = 1 / internal_delta_time;
                std::sprintf(
                    title,
                    "FPS:%.1f Memory:%sMiB, "
                    "Ronin_Allocated:%s, SDL_Allocated:%s, Frames:%s",
                    fps,
                    Math::NumBeautify(get_process_sizeMemory() / 1024 / 1024).c_str(),
                    Math::NumBeautify(RoninMemory::total_allocated()).c_str(),
                    Math::NumBeautify(SDL_GetNumAllocations()).c_str(),
                    Math::NumBeautify(internal_frames).c_str());
                SDL_SetWindowTitle(active_window, title);
                fps = TimeEngine::startUpTime() + .5f; // updater per N seconds
            }

            // delay elapsed
            delayed = Math::Max(0, static_cast<int>(Math::Ceil(secPerFrame) - delayed));

            if(switched_world == nullptr || switched_world->irs->request_unloading && preload_world == nullptr)
                break; // break on unload state

            // update events
            internal_reseting();

            // delaying
            SDL_Delay(delayed);
        }

        // unload world
        if(switched_world)
        {
            Runtime::internal_unload_world(switched_world);
            switched_world = nullptr;
        }

        if(simConfig.renderer_hardware)
        {
            SDL_DestroyRenderer(simConfig.renderer_hardware);
            simConfig.renderer_hardware = nullptr;
        }

        if(simConfig.renderer_software)
        {
            SDL_DestroyRenderer(simConfig.renderer_software);
            if(simConfig.software_surface)
                SDL_FreeSurface(simConfig.software_surface);

            simConfig.software_surface = nullptr;
            simConfig.renderer_software = nullptr;
        }
        renderer = nullptr;
    }

    void RoninSimulator::ShowMessage(const std::string &message)
    {
        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE, "%s", message.c_str());
        printf("%s\n", message.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, nullptr, message.c_str(), active_window);
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
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Ronin Engine: failed", _template.c_str(), active_window);
        Kill();
    }

    void RoninSimulator::Log(const std::string &str)
    {
        SDL_Log("%s", str.c_str());
    }

    void RoninSimulator::BreakSimulate()
    {
        ShowMessageFail("Out of memory!");
    }

    void RoninSimulator::Kill(void)
    {
        exit(EXIT_FAILURE);
    }

    std::vector<RenderDriverInfo> RoninSimulator::EnumerateRenderDrivers()
    {
        SDL_RendererInfo rdi;

        std::vector<RenderDriverInfo> drivers;
        int num = SDL_GetNumRenderDrivers();

        for(; num-- > 0;)
        {
            if(SDL_GetRenderDriverInfo(num, &rdi))
            {
                Log(SDL_GetError());
                continue;
            }

            drivers.emplace_back(
                rdi.name,
                RendererFlags(rdi.flags ^ SDL_RENDERER_TARGETTEXTURE),
                (rdi.flags & SDL_RENDERER_SOFTWARE ? RendererBackend::Software : RendererBackend::Hardware),
                rdi.max_texture_width,
                rdi.max_texture_height);
        }

        return drivers;
    }

    std::vector<std::string> RoninSimulator::EnumerateVideoDrivers()
    {
        std::vector<std::string> vids;
        int num = SDL_GetNumVideoDrivers();

        SDL_ClearError();

        if(num < 1)
            Log(SDL_GetError());

        for(; num-- > 0;)
            vids.emplace_back(SDL_GetVideoDriver(num));

        return vids;
    }

    void RoninSimulator::GetSettings(RoninSettings *settings)
    {
        if(active_window == nullptr)
        {
            ShowMessage("Engine not inited");
            return;
        }

        SDL_RendererInfo info;
        if(renderer != nullptr && SDL_GetRendererInfo(renderer, &info) == 0)
        {
            settings->selectRenderBackend = (info.flags & SDL_RENDERER_SOFTWARE) ? RendererBackend::Software : RendererBackend::Hardware;
        }

        settings->brightness = SDL_GetWindowBrightness(active_window);

        SDL_GetWindowOpacity(active_window, &settings->windowOpacity);
    }

    bool RoninSimulator::SetSettings(const RoninSettings *settings)
    {
        if(active_window == nullptr)
        {
            ShowMessage("Engine not inited");
            return false;
        }

        RoninSettings refSettings;
        GetSettings(&refSettings);

        std::function<bool(void)> applies[] {
            // Apply Render Backend
            [&]()
            {
                bool state = false;
                if(refSettings.selectRenderBackend != settings->selectRenderBackend)
                {
                    switch(settings->selectRenderBackend)
                    {
                        case RendererBackend::Software:
                            if(simConfig.renderer_software == nullptr)
                            {
                                if(simConfig.software_surface == nullptr ||
                                   (simConfig.software_surface->w != active_resolution.width ||
                                    simConfig.software_surface->h != active_resolution.height))
                                {
                                    // Free old
                                    if(simConfig.software_surface != nullptr)
                                        SDL_FreeSurface(simConfig.software_surface);

                                    simConfig.software_surface = SDL_CreateRGBSurfaceWithFormat(
                                        0, active_resolution.width, active_resolution.height, 32, sdl_default_pixelformat);
                                }

                                if(simConfig.renderer_software)
                                    SDL_DestroyRenderer(simConfig.renderer_software);

                                simConfig.renderer_software = SDL_CreateSoftwareRenderer(simConfig.software_surface);
                            }
                            simConfig.conf |= CONF_RENDER_SOFTWARE;
                            break;
                        case RendererBackend::Hardware:
                            simConfig.conf |= CONF_RENDER_CHANGED; // set as HARDWARE
                            break;
                    }
                    state = true;
                }

                return state;
            },
            // Apply Video Driver
            [&]() { return false; },
            // Apply Render Driver
            [&]() { return false; },
            // Apply Brightness
            [&]() {
                return (
                    refSettings.brightness != settings->brightness && SDL_SetWindowBrightness(active_window, settings->brightness) == 0);
            },
            // Apply Window Opacity
            [&]() {
                return (
                    refSettings.windowOpacity != settings->windowOpacity &&
                    SDL_SetWindowOpacity(active_window, settings->windowOpacity) == 0);
            }};
        bool apply_any = false;
        for(auto &apply : applies)
        {
            apply() && !apply_any && (apply_any = true);
        }

        if(apply_any)
        {
            Log("Settings apply");
        }

        return apply_any;
    }
} // namespace RoninEngine
