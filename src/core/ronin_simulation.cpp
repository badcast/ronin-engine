#include "ronin.h"
#include "ronin_std.h"
#include "ronin_matrix.h"
#include "ronin_audio.h"
#include "ronin_debug.h"

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
        extern void free_legacy_font();
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
        World *_world;
        World *preload_world;
        World *last_switched_world;
        std::set<World *> pinnedWorlds;
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
        extern void internal_input_init();
        extern void internal_input_update(const SDL_Event &e);
        extern void internal_input_update_before();
        extern void internal_input_update_after();
        extern void internal_input_release();
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

    RoninEnvironment gscope;

    void internal_apply_settings()
    {
        if(gscope.simConfig.conf == CONF_RENDER_NOCONF)
            return;

        if(gscope.simConfig.conf & CONF_RENDER_CHANGED)
        {
            if(gscope.simConfig.conf & CONF_RENDER_SOFTWARE == CONF_RENDER_SOFTWARE)
                gscope.simConfig.renderBackend = RenderDriverInfo::RenderBackend::CPU;
            else
                gscope.simConfig.renderBackend = RenderDriverInfo::RenderBackend::GPU;
        }

        if(gscope.simConfig.conf & CONF_RELOAD_WORLD)
        {
            last_switched_world = _world; // Switched world first unload after load
            preload_world = _world;       // Switched world as Newer is preload

            gscope.internalWorldLoaded = false;
        }

        gscope.simConfig.conf = CONF_RENDER_NOCONF;
    }

    void internal_init_timer()
    {
        internal_game_time = 0;
        internal_time_scale = 1;
        internal_start_engine_time = 0;
        internal_start_engine_time = Time::startUpTime();
    }

    void internal_draw_debug()
    {
        constexpr int debugFontSize = 11;

        static struct __INFOLABEL__
        {
            int labelLen;

            std::uint32_t value;
            std::string format;
            Color format_color;

            __INFOLABEL__(const char *label, std::uint32_t value) : format(label)
            {
                this->labelLen = this->format.length();
                this->value = value;
            }
        } debugLabels[] = {{"FPS: ", 0}, {" > GUI: ", 0}, {" > Scripts: ", 0}, {" > Render: ", 0}, {" > Gizmos: ", 0}, {" > Memory: ", 0}};

        static std::uint32_t max_elements = sizeof(debugLabels) / sizeof(debugLabels[0]);
        static std::uint32_t max;
        static std::uint32_t averrage;

        int x;
        char buffer[32];
        Vec2 g_size {138, static_cast<float>(debugFontSize * (max_elements + 2))};
        Vec2Int screen_point = Vec2::RoundToInt({g_size.x, static_cast<float>(gscope.activeResolution.height)});
        Vec2 g_pos = Camera::ScreenToWorldPoint({screen_point.x / 2.f, screen_point.y - g_size.y / 2});

        if(Time::frame() % 10 == 0)
        {
            // Update data
            debugLabels[0].value = gscope.lastWatcher.delayFrameRate;
            debugLabels[1].value = gscope.lastWatcher.delayRenderGUI;
            debugLabels[2].value = gscope.lastWatcher.delayExecScripts + gscope.lastWatcher.delayExecWorld;
            debugLabels[3].value = gscope.lastWatcher.delayRenderWorld;
            debugLabels[4].value = gscope.lastWatcher.delayRenderGizmos;
            debugLabels[5].value = Perfomances::GetMemoryUsed() / 1024 / 1024; // convert Bytes to MB

            // calculate averrage and max
            max = 10;
            averrage = 0;
            for(x = 1; x < max_elements - 1; ++x)
            {
                max = std::max(debugLabels[x].value, max);
                averrage += debugLabels[x].value;
            }
            averrage /= std::max(1, x);

            // Calculate FPS and timing
            debugLabels[0].format.resize(debugLabels[0].labelLen);
            snprintf(buffer, sizeof(buffer), "%d (%d ms)", static_cast<int>(Math::Max<int>(0, 1 / Time::deltaTime())), debugLabels[0].value);
            debugLabels[0].format += buffer;

            // format text
            for(x = 1; x < max_elements; ++x)
            {
                debugLabels[x].format.resize(debugLabels[x].labelLen);
                snprintf(buffer, sizeof(buffer), "%d ", debugLabels[x].value);
                debugLabels[x].format += buffer;

                // format color
                if(max && debugLabels[x].value == max)
                    debugLabels[x].format_color = Color::red;
                else if(averrage && debugLabels[x].value >= averrage)
                    debugLabels[x].format_color = Color::yellow;
                else
                    debugLabels[x].format_color = Color::white;
            }

            for(x = 1; x < max_elements - 1; ++x)
                debugLabels[x].format += "ms";
            debugLabels[x].format += "MiB";
            debugLabels[x].format_color = Color::white;
        }

        // Set background color
        RenderUtility::SetColor(Color(0, 0, 0, 100));

        // Draw box
        RenderUtility::DrawFillRect(g_pos, g_size.x / _world->irs->metricPixelsPerPoint.x, g_size.y / _world->irs->metricPixelsPerPoint.y);

        screen_point.x = 10;
        screen_point.y -= static_cast<int>(g_size.y) - debugFontSize / 2;
        RenderUtility::SetColor(Color::white);
        RenderUtility::DrawTextToScreen(screen_point, debugLabels[0].format, debugFontSize);
        for(x = 1; x < max_elements; ++x)
        {
            RenderUtility::SetColor(debugLabels[x].format_color);

            screen_point.y += debugFontSize + 1;
            RenderUtility::DrawTextToScreen(screen_point, debugLabels[x].format, debugFontSize);
        }
    }

    void RoninSimulator::Init(InitializeFlags flags)
    {
        std::uint32_t current_inits;

        if(SDL_WasInit(SDL_INIT_VIDEO))
            return;

        current_inits = SDL_INIT_EVENTS | SDL_INIT_TIMER;
        if(SDL_Init(current_inits) == -1)
        {
            ronin_err_d("Fail init SDL2 main system.");
            return;
        }

        // init graphics
        if(flags & InitializeFlags::Graphics)
        {
            current_inits = SDL_INIT_VIDEO;

            if(SDL_InitSubSystem(current_inits) == -1)
                ronin_err_d("Fail init Video system.");

            current_inits = IMG_InitFlags::IMG_INIT_PNG | IMG_InitFlags::IMG_INIT_JPG;
            if(IMG_Init(current_inits) != current_inits)
                ronin_err_d("Fail init Image library.");

            if(TTF_Init() == -1)
                ronin_err_d("Fail init Font library.");
        }

        // init Audio system
        if(flags & InitializeFlags::Audio)
        {
            if(Runtime::RoninAudio::Init() == -1)
                ronin_err_d("Fail init audio.");
        }

        if(flags & InitializeFlags::Joystick)
        {
            if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
                ronin_err_d("Fail init Joystick system.");
        }

        // init input system
        internal_input_init();

        // setup
        _world = nullptr;

        // Init legacy fonts
        UI::font2d_init(1);
    }

    void RoninSimulator::Finalize()
    {
        if(gscope.activeWindow == nullptr)
            return;

        // unload existence worlds
        for(auto &pinned : Runtime::pinnedWorlds)
        {
            Runtime::internal_unload_world(pinned);
        }

        Runtime::pinnedWorlds.clear();

        SDL_DestroyWindow(gscope.activeWindow);
        gscope.activeWindow = nullptr;

        UI::free_legacy_font();

        // NOTE: Free Global Resources
        if(external_global_resources)
        {
            gid_resources_free(external_global_resources);
            RoninMemory::free(external_global_resources);
            external_global_resources = nullptr;
        }
        Runtime::internal_free_loaded_assets();
        internal_input_release();
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
        if(gscope.activeWindow)
            return;

        std::uint32_t __flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;

        if(fullscreen)
            __flags |= SDL_WINDOW_FULLSCREEN;

        gscope.activeWindow = SDL_CreateWindow("Ronin Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resolution.width, resolution.height, __flags);

        if(gscope.activeWindow == nullptr)
            ShowMessageFail(SDL_GetError());

        // Get valid resolution size
        SDL_DisplayMode mode;
        SDL_GetWindowDisplayMode(gscope.activeWindow, &mode);
        // SDL_SetWindowSize(active_window, mode.w, mode.h);

        SDL_ShowWindow(gscope.activeWindow);

        // get activated resolution
        gscope.activeResolution = GetCurrentResolution();
    }

    std::pair<bool, Resolution> RoninSimulator::ShowSplashScreen(bool applyScreen)
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
            if(applyScreen)
                Show(__templs[btId - 1], false);
            resultIsOK = true;
        }

        return {resultIsOK, __templs[btId]};
    }

    void RoninSimulator::RequestQuit()
    {
        if(_world)
            _world->RequestUnload();
    }

    bool RoninSimulator::LoadWorld(World *world, bool unloadPrevious)
    {
        const std::function<bool(void)> checks_queue[] {
            [=]() -> bool const
            {
                bool hasError = gscope.activeWindow == nullptr;
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
                bool hasError = _world == world || last_switched_world != nullptr || preload_world == world;
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

        if(unloadPrevious && _world)
        {
            last_switched_world = _world;
            last_switched_world->RequestUnload();
        }

        // cancelation of reload
        CancelReload();

        // switching as main
        if(_world == nullptr)
            _world = world;

        // preload world
        preload_world = world;

        gscope.internalWorldCanStart = false;
        gscope.internalWorldLoaded = false;

        return true;
    }

    bool RoninSimulator::ReloadWorld()
    {
        if(_world == nullptr || preload_world != nullptr)
        {
            Log("Active world not loaded");
            return false;
        }

        // set config for reload
        gscope.simConfig.conf |= CONF_RELOAD_WORLD;
        return true;
    }

    World *RoninSimulator::GetWorld()
    {
        return _world;
    }

    bool RoninSimulator::CancelReload()
    {
        int last_flag = (gscope.simConfig.conf & CONF_RELOAD_WORLD);
        // clear reload flag
        gscope.simConfig.conf &= ~CONF_RELOAD_WORLD;
        return last_flag != 0;
    }

    Resolution RoninSimulator::GetCurrentResolution()
    {
        if(gscope.activeWindow == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode displayMode;

        int dpIndex = SDL_GetWindowDisplayIndex(gscope.activeWindow);
        int mdIndex = SDL_GetNumDisplayModes(dpIndex);

        if(SDL_GetDisplayMode(dpIndex, 0, &displayMode) == -1)
        {
            RoninSimulator::ShowMessageFail(SDL_GetError());
        }

        SDL_GetWindowSize(gscope.activeWindow, &displayMode.w, &displayMode.h);

        return {displayMode.w, displayMode.h, displayMode.refresh_rate};
    }

    Resolution RoninSimulator::GetCurrentDisplayResolution()
    {
        if(gscope.activeWindow == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode display_mode;

        if(SDL_GetWindowDisplayMode(gscope.activeWindow, &display_mode) == -1)
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

        if(gscope.activeWindow == nullptr)
            active_display = SDL_GetNumVideoDisplays() > 0 ? 0 : -1;
        else
            active_display = SDL_GetWindowDisplayIndex(gscope.activeWindow);

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
        if(gscope.activeWindow == nullptr)
        {
            throw ronin_init_error();
        }

        SDL_DisplayMode mode;

        mode.w = new_resolution.width;
        mode.h = new_resolution.height;
        mode.refresh_rate = new_resolution.hz;
        bool result = SDL_SetWindowDisplayMode(gscope.activeWindow, &mode) == 0;
        if(result)
        {
            gscope.activeResolution = new_resolution;
        }
        else
        {
            RoninSimulator::ShowMessage(SDL_GetError());
        }
        return result;
    }

    bool RoninSimulator::SetFullscreenMode(FullscreenMode mode)
    {
        if(gscope.activeWindow == nullptr)
        {
            return false;
        }

        std::uint32_t flags = SDL_GetWindowFlags(gscope.activeWindow);
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

        return SDL_SetWindowFullscreen(gscope.activeWindow, flags) == 0;
    }

    TimingWatcher RoninSimulator::GetTimingWatches()
    {
        return gscope.lastWatcher;
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
        gscope.debugMode = state;
    }

    void RoninSimulator::Simulate()
    {
        SDL_Event event;
        SDL_DisplayMode displayMode;

        bool isQuitting = false;
        char title[96];
        float fps = 0;
        float secPerFrame, game_time_score;
        std::uint64_t delayed = 0;

        if(gscope.activeWindow == nullptr)
        {
            ShowMessage("Engine not inited");
            return;
        }

        if(_world == nullptr)
        {
            ShowMessage("World not loaded");
            return;
        }

        Resolution current_resolution = RoninSimulator::GetCurrentResolution();
        gscope.queueWatcher = {};

        secPerFrame = 1000.f / (current_resolution.hz / 1); // refresh screen from Monitor Settings
        game_time_score = secPerFrame / 1000;
        while(!isQuitting)
        {
            // TODO: m_level->request_unloading use as WNILE block (list proc)
            delayed = Time::millis();

            // Timing for FrameTime
            Time::BeginWatch();
            // Timing for System delay
            Time::BeginWatch();

            while(SDL_PollEvent(&event))
            {
                internal_input_update(event);
                switch(event.type)
                {
                    case SDL_QUIT:
                    {
                        isQuitting = true;
                        _world->RequestUnload();
                        last_switched_world = _world;
                        gscope.simConfig.conf = CONF_RENDER_NOCONF;
                        gscope.internalWorldLoaded = false;
                        _world = nullptr;
                        break;
                    }
                }
            }

            // Update Before Input (Axis)
            internal_input_update_before();

            // Apply changed settings
            internal_apply_settings();

            if(!gscope.internalWorldLoaded)
            {
                // Unload old World
                if(last_switched_world)
                {
                    internal_unload_world(last_switched_world);
                    last_switched_world = nullptr;
                }

                if(gscope.renderer != nullptr)
                {
                    // Destroy last renderer
                    SDL_DestroyRenderer(gscope.renderer);
                    gscope.renderer = nullptr;
                }

                if(_world == nullptr)
                {
                    gscope.queueWatcher.delaySystem = Time::EndWatch();
                    goto end_simulate;
                }
                else
                {
                    // On Runtime Loading world

                    std::uint32_t renderFlags = SDL_RENDERER_TARGETTEXTURE;

                    if(gscope.simConfig.renderBackend == RenderDriverInfo::RenderBackend::CPU)
                    {
                        renderFlags |= SDL_RENDERER_SOFTWARE;
                    }
                    else
                    {
                        renderFlags |= SDL_RENDERER_ACCELERATED;
                    }

                    // on first load level
                    gscope.renderer = SDL_CreateRenderer(gscope.activeWindow, -1, renderFlags);

                    if(gscope.renderer == nullptr)
                        ShowMessageFail(SDL_GetError());

                    // Set blendmode
                    SDL_SetRenderDrawBlendMode(gscope.renderer, SDL_BLENDMODE_BLEND);

                    // on swtiched and init resources, even require
                    Runtime::internal_load_world(preload_world);

                    _world = preload_world;

                    // Init Internal World Timer IIWT
                    internal_init_timer();

                    if(!_world->isHierarchy())
                    {
                        // init main object
                        _world->irs->mainObject = create_empty_gameobject();
                        _world->irs->mainObject->name("Main Object");
                        _world->irs->mainObject->transform()->name("Root");
                        // pickup from renders
                        Matrix::matrix_remove(_world->irs->mainObject->transform());
                    }

                    // Set Metric as default
                    _world->irs->metricPixelsPerPoint = Vec2::one * defaultPixelsPerPoint;

                    gscope.internalWorldCanStart = false;

                    _world->OnAwake();

                    preload_world = nullptr;
                    gscope.internalWorldLoaded = true;
                }
            }

            gscope.queueWatcher.delaySystem = Time::EndWatch();
            // end watcher

            // update level
            if(!_world->irs->requestUnloading)
            {
                // begin watcher
                Time::BeginWatch();

                if(!gscope.internalWorldCanStart)
                {
                    _world->OnStart();
                    gscope.internalWorldCanStart = true;
                }

                if(!_world->irs->requestUnloading)
                {
                    _world->OnUpdate();
                }
                // end watcher
                gscope.queueWatcher.delayExecWorld = Time::EndWatch();

                if(_world->irs->requestUnloading)
                {
                    goto end_simulate; // break on unload state
                }
            }
            else
                goto end_simulate; // break on unload state

            level_render_world();

            // Run Collector
            Time::BeginWatch();
            if(!_world->irs->requestUnloading)
                SepukuRun();
            // end watcher
            gscope.queueWatcher.delayHarakiring = Time::EndWatch();

            if(_world->irs->requestUnloading)
                goto end_simulate; // break on unload state

            // Set scale to default
            SDL_RenderSetScale(gscope.renderer, 1.f, 1.f);

            // begin watcher
            Time::BeginWatch();
            UI::native_draw_render(_world->irs->gui);
            gscope.queueWatcher.delayRenderGUI = Time::EndWatch();
            // end watcher

            if(_world->irs->requestUnloading)
                goto end_simulate; // break on unload state

            if(gscope.debugMode)
                internal_draw_debug();

            if(!last_switched_world)
            {
                // begin watcher
                Time::BeginWatch();
                SDL_RenderPresent(gscope.renderer);
                gscope.queueWatcher.delayRenderWorld += Time::EndWatch();
                // end watcher
            }

        end_simulate:

            delayed = Time::millis() - delayed;

            gscope.queueWatcher.delayFrameRate = Time::EndWatch();

            // update watcher
            gscope.lastWatcher = gscope.queueWatcher;

            ++internal_frames;

            if(!_watcher_time.empty())
                throw ronin_watcher_error();

            internal_delta_time = Math::Min<float>(1.f, Math::Max<float>(delayed / 1000.f, game_time_score));
            internal_game_time += internal_delta_time;

            if(gscope.debugMode)
            {
                if(Time::startUpTime() > fps)
                {
                    // SDL_METHOD:
                    // fps = internal_frames / (TimeEngine::startUpTime());
                    fps = 1 / internal_delta_time;
                    std::sprintf(title, "FPS:%.1f Memory:%sMiB, Ronin Objects:%s, Internal Objects:%s, Frames:%s", fps, Math::NumBeautify(Perfomances::GetMemoryUsed() / 1024 / 1024).c_str(), Math::NumBeautify(RoninMemory::total_allocated()).c_str(), Math::NumBeautify(SDL_GetNumAllocations()).c_str(), Math::NumBeautify(internal_frames).c_str());
                    SDL_SetWindowTitle(gscope.activeWindow, title);
                    fps = Time::startUpTime() + .5f; // updater per N seconds
                }
            }

            // delay elapsed
            delayed = Math::Max(0, static_cast<int>(Math::Ceil(secPerFrame) - delayed));

            // update events
            internal_input_update_after();

            if(_world == nullptr || _world->irs->requestUnloading && preload_world == nullptr)
                break; // break on unload state

            // delaying
            SDL_Delay(delayed);
        }

        // unload world
        if(_world)
        {
            Runtime::internal_unload_world(_world);
            _world = nullptr;
        }

        if(gscope.renderer)
        {
            SDL_DestroyRenderer(gscope.renderer);
            gscope.renderer = nullptr;
        }
    }

    void RoninSimulator::ShowMessage(const std::string &message)
    {
        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE, "%s", message.c_str());
        printf("%s\n", message.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, nullptr, message.c_str(), gscope.activeWindow);
    }

    void RoninSimulator::ShowMessageFail(const std::string &message)
    {
        // fprintf(stderr, "%s", message.data());
        SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_CRITICAL, "%s", message.c_str());
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Ronin Engine: failed", message.c_str(), gscope.activeWindow);
        Kill();
    }

    void RoninSimulator::Log(const char *str)
    {
        SDL_Log("%s", str);
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

            drivers.emplace_back(rdi.name, RenderDriverInfo::RendererFlags(rdi.flags ^ SDL_RENDERER_TARGETTEXTURE), (rdi.flags & SDL_RENDERER_SOFTWARE ? RenderDriverInfo::RenderBackend::CPU : RenderDriverInfo::RenderBackend::GPU), rdi.max_texture_width, rdi.max_texture_height);
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
        if(gscope.activeWindow == nullptr)
        {
            return;
        }

        SDL_RendererInfo info;
        if(gscope.renderer != nullptr && SDL_GetRendererInfo(gscope.renderer, &info) == 0)
        {
            settings->selectRenderBackend = (info.flags & SDL_RENDERER_SOFTWARE) ? RenderDriverInfo::RenderBackend::CPU : RenderDriverInfo::RenderBackend::GPU;
        }

        settings->brightness = SDL_GetWindowBrightness(gscope.activeWindow);

        if(SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY) != nullptr && !SDL_strcmp(SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY), "1"))
        {
            settings->selectTextureQuality = RoninSettings::RenderTextureScaleQuality::Linear;
        }
        else
        {
            settings->selectTextureQuality = RoninSettings::RenderTextureScaleQuality::Nearest;
        }

        SDL_GetWindowOpacity(gscope.activeWindow, &settings->windowOpacity);
    }

    bool RoninSimulator::SetSettings(const RoninSettings *settings)
    {
        if(!SDL_WasInit(SDL_INIT_VIDEO))
        {
            return false;
        }

        RoninSettings refSettings;
        GetSettings(&refSettings);

        std::function<bool(void)> applies[] {

            [&]() // Apply Render Backend
            {
                bool state = false;
                if(refSettings.selectRenderBackend != settings->selectRenderBackend)
                {
                    switch(settings->selectRenderBackend)
                    {
                        case RenderDriverInfo::RenderBackend::CPU:
                            gscope.simConfig.conf |= CONF_RENDER_SOFTWARE;
                            break;
                        case RenderDriverInfo::RenderBackend::GPU:
                            gscope.simConfig.conf |= CONF_RENDER_CHANGED; // set as HARDWARE
                            break;
                    }
                    state = true;
                }

                return state;
            },
            [&]() // Apply Render Texture Scale Quality
            { return refSettings.selectTextureQuality != settings->selectTextureQuality && (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, (settings->selectTextureQuality ? "1" : "0")) == SDL_TRUE); },
            // Apply Video Driver
            [&]() { return false; },
            // Apply Render Driver
            [&]() { return false; },
            // Apply Brightness
            [&]() { return (refSettings.brightness != settings->brightness && SDL_SetWindowBrightness(gscope.activeWindow, settings->brightness) == 0); },
            // Apply Window Opacity
            [&]() { return (refSettings.windowOpacity != settings->windowOpacity && SDL_SetWindowOpacity(gscope.activeWindow, settings->windowOpacity) == 0); }};
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
