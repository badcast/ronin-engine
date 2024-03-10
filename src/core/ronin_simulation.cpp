#include "ronin.h"
#include "ronin_matrix.h"
#include "ronin_audio.h"

#ifdef __linux__
// For malloc_trim
#include <malloc.h>
#endif

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

        std::list<Asset> loaded_assets;
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

    RoninEnvironment env;

    void internal_apply_settings()
    {
        if(env.simConfig.conf == CONF_RENDER_NOCONF)
            return;

        if(env.simConfig.conf & CONF_RENDER_CHANGED)
        {
            if(env.simConfig.conf & CONF_RENDER_SOFTWARE == CONF_RENDER_SOFTWARE)
                env.simConfig.renderBackend = RenderBackend::CPU;
            else
                env.simConfig.renderBackend = RenderBackend::GPU;
        }

        if(env.simConfig.conf & CONF_RELOAD_WORLD)
        {
            last_switched_world = switched_world; // Switched world first unload after load
            preload_world = switched_world;       // Switched world as Newer is preload

            env.internal_world_loaded = false;
        }

        env.simConfig.conf = CONF_RENDER_NOCONF;
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

        if(env.active_window != nullptr)
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
        if(env.active_window == nullptr)
            return;

        // unload existence worlds
        for(auto &pinned : Runtime::pinned_worlds)
        {
            Runtime::internal_unload_world(pinned);
        }

        Runtime::pinned_worlds.clear();

        SDL_DestroyWindow(env.active_window);
        env.active_window = nullptr;

        UI::free_fonts();

        // NOTE: Free Global Resources
        if(external_global_resources)
        {
            gid_resources_free(external_global_resources);
            RoninMemory::free(external_global_resources);
            external_global_resources = nullptr;
        }
        Runtime::internal_free_loaded_assets();

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

#ifdef __linux__
        // On Unix/Linux - free RSS reserved memory
        malloc_trim(0);
#endif
    }

    void RoninSimulator::Show(const RoninEngine::Resolution &resolution, bool fullscreen)
    {
        if(env.active_window)
            return;

        std::uint32_t __flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;

        if(fullscreen)
            __flags |= SDL_WINDOW_FULLSCREEN;

        env.active_window =
            SDL_CreateWindow("Ronin Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resolution.width, resolution.height, __flags);

        // Get valid resolution size
        SDL_DisplayMode mode;
        SDL_GetWindowDisplayMode(env.active_window, &mode);
        // SDL_SetWindowSize(active_window, mode.w, mode.h);

        SDL_ShowWindow(env.active_window);
        if(!env.active_window)
            ShowMessageFail(SDL_GetError());

        // get activated resolution
        env.active_resolution = GetCurrentResolution();
    }

    std::pair<bool, Resolution> RoninSimulator::ShowSplashScreen()
    {
        bool resultIsOK = false;
        RoninEngine::Resolution __templs[3];
        SDL_MessageBoxButtonData btdt[4];
        std::string _lstrs[3];
        SDL_MessageBoxData msgbd {};
        SDL_DisplayMode mode;

        btdt[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
        btdt[0].buttonid = 0;
        btdt[0].text = "Cancel";

        __templs[0] = Resolution::GetMinResolution();
        __templs[1] = Resolution::GetMidResolution();
        __templs[2] = Resolution::GetMaxResolution();

        for(int s = 1; s < 4; ++s)
        {
            btdt[s].buttonid = s;

            _lstrs[s - 1] += std::to_string(__templs[s - 1].width);
            _lstrs[s - 1] += "x";

            _lstrs[s - 1] += std::to_string(__templs[s - 1].height);
            _lstrs[s - 1] += "x";

            _lstrs[s - 1] += std::to_string(__templs[s - 1].hz);

            btdt[s].text = _lstrs[s - 1].data();
        }

        msgbd.flags = SDL_MESSAGEBOX_INFORMATION;
        msgbd.title = "Screen Resolution";
        msgbd.message = "Select screen resolution";
        msgbd.numbuttons = sizeof(btdt) / sizeof(btdt[0]);
        msgbd.buttons = btdt;

        int btId;
        if(!SDL_ShowMessageBox(&msgbd, &btId) && btId)
        {
            Show(__templs[btId - 1], false);
            resultIsOK = true;
        }

        return {resultIsOK, __templs[btId]};
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
                bool hasError = env.active_window == nullptr;
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
                    Log("Current world is loading state. Failed.");
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

        env.internal_world_can_start = false;
        env.internal_world_loaded = false;

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
        env.simConfig.conf |= CONF_RELOAD_WORLD;
        return true;
    }

    bool RoninSimulator::CancelReload()
    {
        int last_flag = (env.simConfig.conf & CONF_RELOAD_WORLD);
        // clear reload flag
        env.simConfig.conf &= ~CONF_RELOAD_WORLD;
        return last_flag != 0;
    }

    Resolution RoninSimulator::GetCurrentResolution()
    {
        if(env.active_window == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode displayMode;

        int dpIndex = SDL_GetWindowDisplayIndex(env.active_window);
        int mdIndex = SDL_GetNumDisplayModes(dpIndex);

        if(SDL_GetDisplayMode(dpIndex, 0, &displayMode) == -1)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }

        SDL_GetWindowSize(env.active_window, &displayMode.w, &displayMode.h);

        return {displayMode.w, displayMode.h, displayMode.refresh_rate};
    }

    Resolution RoninSimulator::GetCurrentDisplayResolution()
    {
        if(env.active_window == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode display_mode;

        if(SDL_GetWindowDisplayMode(env.active_window, &display_mode) == -1)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }
        return {display_mode.w, display_mode.h, display_mode.refresh_rate};
    }

    std::list<Resolution> RoninSimulator::EnumerateResolutions()
    {
        std::list<Resolution> resolutions;
        SDL_DisplayMode mode;
        int active_display;

        if(env.active_window == nullptr)
            active_display = SDL_GetNumVideoDisplays() > 0 ? 0 : -1;
        else
            active_display = SDL_GetWindowDisplayIndex(env.active_window);

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
        if(env.active_window == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode mode;

        mode.w = new_resolution.width;
        mode.h = new_resolution.height;
        mode.refresh_rate = new_resolution.hz;
        bool result = SDL_SetWindowDisplayMode(env.active_window, &mode) == 0;
        if(result)
        {
            env.active_resolution = new_resolution;
        }
        else
        {
            RoninSimulator::ShowMessage(SDL_GetError());
        }
        return result;
    }

    bool RoninSimulator::SetFullscreenMode(FullscreenMode mode)
    {
        if(env.active_window == nullptr)
        {
            return false;
        }

        std::uint32_t flags = SDL_GetWindowFlags(env.active_window);
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

        return SDL_SetWindowFullscreen(env.active_window, flags) == 0;
    }

    TimingWatcher RoninSimulator::GetTimingWatches()
    {
        return env.last_watcher;
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
        env.ronin_debug_mode = state;
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

        if(env.active_window == nullptr)
        {
            ShowMessage("Engine not inited");
            return;
        }

        if(switched_world == nullptr)
        {
            ShowMessage("World not loaded");
            return;
        }

        Resolution current_resolution = RoninSimulator::GetCurrentResolution();
        env.queue_watcher = {};

        secPerFrame = 1000.f / current_resolution.hz; // refresh screen from Monitor Settings
        game_time_score = secPerFrame / 1000;
        while(!isQuiting)
        {
            // TODO: m_level->request_unloading use as WNILE block (list proc)
            delayed = TimeEngine::millis();

            // Timing for FrameTime
            TimeEngine::BeginWatch();
            // Timing for System delay
            TimeEngine::BeginWatch();

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
                        env.simConfig.conf = CONF_RENDER_NOCONF;
                        env.internal_world_loaded = false;
                        switched_world = nullptr;
                        break;
                    }
                }
            }

            // Apply changed settings
            internal_apply_settings();

            if(!env.internal_world_loaded)
            {
                // Unload old World
                if(last_switched_world)
                {
                    internal_unload_world(last_switched_world);
                    last_switched_world = nullptr;
                }

                if(env.renderer != nullptr)
                {
                    // Destroy last renderer
                    SDL_DestroyRenderer(env.renderer);
                    env.renderer = nullptr;
                }

                if(switched_world == nullptr)
                {
                    env.queue_watcher.delaySystem = TimeEngine::EndWatch();
                    goto end_simulate;
                }
                else
                {
                    // On Runtime Loading world

                    bool software_render = true;
                    std::uint32_t renderFlags = SDL_RENDERER_TARGETTEXTURE;

                    if(env.simConfig.renderBackend == RenderBackend::CPU)
                    {
                        renderFlags |= SDL_RENDERER_SOFTWARE;
                    }
                    else
                    {
                        renderFlags |= SDL_RENDERER_ACCELERATED;
                    }

                    // on first load level
                    env.renderer = SDL_CreateRenderer(env.active_window, -1, renderFlags);

                    if(env.renderer == nullptr)
                        ShowMessageFail(SDL_GetError());

                    // Set blendmode
                    SDL_SetRenderDrawBlendMode(env.renderer, SDL_BLENDMODE_BLEND);

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
                        Matrix::matrix_remove(switched_world->irs->main_object->transform());
                    }

                    env.internal_world_can_start = false;

                    switched_world->OnAwake();

                    preload_world = nullptr;
                    env.internal_world_loaded = true;
                }
            }

            input_movement_update();

            env.queue_watcher.delaySystem = TimeEngine::EndWatch();
            // end watcher

            // update level
            if(!switched_world->irs->request_unloading)
            {
                // begin watcher
                TimeEngine::BeginWatch();

                if(!env.internal_world_can_start)
                {
                    switched_world->OnStart();
                    env.internal_world_can_start = true;
                }

                if(!switched_world->irs->request_unloading)
                {
                    switched_world->OnUpdate();
                }
                // end watcher
                env.queue_watcher.delayExecWorld = TimeEngine::EndWatch();

                if(switched_world->irs->request_unloading)
                {
                    goto end_simulate; // break on unload state
                }
            }
            else
                goto end_simulate; // break on unload state

            level_render_world();

            // Run Collector
            TimeEngine::BeginWatch();
            if(!switched_world->irs->request_unloading)
                Bushido_Tradition_Harakiri();
            // end watcher
            env.queue_watcher.delayHarakiring = TimeEngine::EndWatch();

            if(switched_world->irs->request_unloading)
                goto end_simulate; // break on unload state

            // Set scale to default
            SDL_RenderSetScale(env.renderer, 1.f, 1.f);

            // begin watcher
            TimeEngine::BeginWatch();
            UI::native_draw_render(switched_world->irs->gui);
            env.queue_watcher.delayRenderGUI = TimeEngine::EndWatch();
            // end watcher

            if(switched_world->irs->request_unloading)
                goto end_simulate; // break on unload state

            if(!last_switched_world)
            {
                // begin watcher
                TimeEngine::BeginWatch();
                SDL_RenderPresent(env.renderer);
                env.queue_watcher.delayRenderWorld += TimeEngine::EndWatch();
                // end watcher
            }

        end_simulate:

            delayed = TimeEngine::millis() - delayed;

            env.queue_watcher.delayFrameRate = TimeEngine::EndWatch();

            // update watcher
            env.last_watcher = env.queue_watcher;

            ++internal_frames;

            if(!_watcher_time.empty())
                throw ronin_watcher_error();

            internal_delta_time = Math::Min<float>(1.f, Math::Max<float>(delayed / 1000.f, game_time_score));
            internal_game_time += internal_delta_time;

            if(env.ronin_debug_mode && TimeEngine::startUpTime() > fps)
            {
                // SDL_METHOD:
                // fps = internal_frames / (TimeEngine::startUpTime());
                fps = 1 / internal_delta_time;
                std::sprintf(
                    title,
                    "FPS:%.1f Memory:%sMiB, "
                    "Ronin Objects:%s, Internal Objects:%s, Frames:%s",
                    fps,
                    Math::NumBeautify(Perfomances::GetMemoryUsed() / 1024 / 1024).c_str(),
                    Math::NumBeautify(RoninMemory::total_allocated()).c_str(),
                    Math::NumBeautify(SDL_GetNumAllocations()).c_str(),
                    Math::NumBeautify(internal_frames).c_str());
                SDL_SetWindowTitle(env.active_window, title);
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

        if(env.renderer)
        {
            SDL_DestroyRenderer(env.renderer);
            env.renderer = nullptr;
        }
    }

    void RoninSimulator::ShowMessage(const std::string &message)
    {
        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE, "%s", message.c_str());
        printf("%s\n", message.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, nullptr, message.c_str(), env.active_window);
    }

    void RoninSimulator::ShowMessageFail(const std::string &message)
    {
        // fprintf(stderr, "%s", message.data());
        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_CRITICAL, "%s", message.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Ronin Engine: failed", message.c_str(), env.active_window);
        Kill();
    }

    void RoninSimulator::Log(const char *str)
    {
        SDL_Log("%s", str);
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
                (rdi.flags & SDL_RENDERER_SOFTWARE ? RenderBackend::CPU : RenderBackend::GPU),
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
        if(env.active_window == nullptr)
        {
            ShowMessage("Engine not inited");
            return;
        }

        SDL_RendererInfo info;
        if(env.renderer != nullptr && SDL_GetRendererInfo(env.renderer, &info) == 0)
        {
            settings->selectRenderBackend = (info.flags & SDL_RENDERER_SOFTWARE) ? RenderBackend::CPU : RenderBackend::GPU;
        }

        settings->brightness = SDL_GetWindowBrightness(env.active_window);

        SDL_GetWindowOpacity(env.active_window, &settings->windowOpacity);
    }

    bool RoninSimulator::SetSettings(const RoninSettings *settings)
    {
        if(env.active_window == nullptr)
        {
            ShowMessage("Engine not inited");
            return false;
        }

        RoninSettings refSettings;
        GetSettings(&refSettings);

        std::function<bool(void)> applies[] {// Apply Render Backend
                                             [&]()
                                             {
                                                 bool state = false;
                                                 if(refSettings.selectRenderBackend != settings->selectRenderBackend)
                                                 {
                                                     switch(settings->selectRenderBackend)
                                                     {
                                                         case RenderBackend::CPU:
                                                             env.simConfig.conf |= CONF_RENDER_SOFTWARE;
                                                             break;
                                                         case RenderBackend::GPU:
                                                             env.simConfig.conf |= CONF_RENDER_CHANGED; // set as HARDWARE
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
                                                     refSettings.brightness != settings->brightness &&
                                                     SDL_SetWindowBrightness(env.active_window, settings->brightness) == 0);
                                             },
                                             // Apply Window Opacity
                                             [&]() {
                                                 return (
                                                     refSettings.windowOpacity != settings->windowOpacity &&
                                                     SDL_SetWindowOpacity(env.active_window, settings->windowOpacity) == 0);
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
