#include "ronin.h"
#include "ronin_std.h"
#include "ronin_matrix.h"
#include "ronin_audio.h"
#include "ronin_debug.h"
#include "ronin_splash_world.h"

#ifdef __linux__
// For malloc_trim
#include <malloc.h>
#endif

using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine
{
    enum ConfigState : int
    {
        CONF_RENDER_NOCONF = 0,

        CONF_RENDER_CHANGED = 1,
        CONF_RELOAD_WORLD = 2,
        CONF_RESERVED2 = 4,
        CONF_RENDER_SOFTWARE = CONF_RENDER_CHANGED | 8,
    };

    ////////////////////////
    /// GLOBAL VARIABLES ///
    ////////////////////////
    namespace UI
    {
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
        World *currentWorld;
        World *preloadWorld;
        World *lastWorld;
        std::set<World *> pinnedWorlds;
        std::set<World *> privateWorlds;
        std::vector<std::uint32_t> _watcher_time;

        std::list<Asset> loaded_assets;
        GidResources *external_global_resources = nullptr;

        float internal_time_scale, internal_game_time, internal_delta_time;

        std::uint32_t internal_frames; // framecounter
        int _matrix_dimensity_ = 1;
        RoninInput internal_input;
        bool text_inputState;

        extern GameObjectRef create_empty_gameobject();
        extern void free_legacy_font();
        extern void ui_reset_controls();
        extern void text_get(std::string &text);
        extern void internal_input_init();
        extern void internal_input_update(const SDL_Event &e);
        extern void internal_input_update_before();
        extern void internal_input_update_after();
        extern void internal_input_release();
        extern void internal_load_world(World *world);
        extern bool internal_unload_world(World *world);
    } // namespace Runtime

    RoninEnvironment gscope;

    struct _TDebugLabel
    {
        int labelLen;

        std::uint32_t value;
        std::string format;
        Color format_color;

        _TDebugLabel(const char *label, std::uint32_t value) : format(label)
        {
            this->labelLen = this->format.length();
            this->value = value;
        }
    } debugLabels[] = {{"FPS: ", 0}, {" > GUI: ", 0}, {" > Scripts: ", 0}, {" > Render: ", 0}, {" > Gizmos: ", 0}, {" > Memory: ", 0}};

    //////////////////////////////////
    /// PRIVATE INTERNAL FUNCTIONS ///
    //////////////////////////////////
    void internal_apply_settings()
    {
        if(!gscope.config.conf)
            return;

        if(gscope.config.conf & CONF_RENDER_CHANGED)
        {
            if((gscope.config.conf & CONF_RENDER_SOFTWARE) == CONF_RENDER_SOFTWARE)
                gscope.config.renderBackend = RenderDriverInfo::RenderBackend::CPU;
            else
                gscope.config.renderBackend = RenderDriverInfo::RenderBackend::GPU;
        }

        if(gscope.config.conf & CONF_RELOAD_WORLD)
        {
            lastWorld = currentWorld;    // Switched world first unload after load
            preloadWorld = currentWorld; // Switched world as Newer is preload

            gscope.internalWorldLoaded = false;
        }

        gscope.config.conf = CONF_RENDER_NOCONF;
    }

    void internal_init_timer()
    {
        internal_game_time = 0;
        internal_time_scale = 1;
    }

    void internal_draw_debug()
    {
        constexpr int debugFontSize = 11;

        static std::uint32_t max_elements = sizeof(debugLabels) / sizeof(debugLabels[0]);
        static std::uint32_t max;
        static std::uint32_t averrage;
        static std::uint32_t updateTick;

        int x;
        char buffer[32];
        float fps;
        Vec2 g_size {138, static_cast<float>(debugFontSize * (max_elements + 2))};
        Vec2Int screen_point = Vec2::RoundToInt({g_size.x, static_cast<float>(gscope.activeResolution.height)});
        Vec2 g_pos = Camera::ScreenToWorldPoint({screen_point.x / 2.f, screen_point.y - g_size.y / 2});

        if(updateTick < Time::millis())
        {
            // Refresh tick
            updateTick = Time::millis() + 200;

            // Update data
            debugLabels[0].value = Math::Max(1u, gscope.lastWatcher.delayFrameRate);
            debugLabels[1].value = gscope.lastWatcher.delayRenderGUI;
            debugLabels[2].value = gscope.lastWatcher.delayExecScripts + gscope.lastWatcher.delayExecWorld;
            debugLabels[3].value = gscope.lastWatcher.delayPresent;
            debugLabels[4].value = gscope.lastWatcher.delayRenderGizmos;
            debugLabels[5].value = Perfomances::GetMemoryUsed() / 1024 / 1024; // convert Bytes to MiB

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

            snprintf(buffer, sizeof(buffer), "%u (%u ms)", gscope.lastWatcher.fps, debugLabels[0].value);
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
        RenderUtility::DrawFillRect(g_pos, g_size.x / currentWorld->irs->metricPixelsPerPoint.x, g_size.y / currentWorld->irs->metricPixelsPerPoint.y);

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

    void RoninSimulator::makePrivate(Runtime::World *world)
    {
        privateWorlds.insert(world);
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

        // Set VSync as default
        const char *vshint;
        vshint = SDL_GetHint(SDL_HINT_RENDER_VSYNC);
        if(vshint == nullptr)
        {
            SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
        }

        // setup
        currentWorld = nullptr;

        // Init legacy fonts
        font2d_init(1);
    }

    void RoninSimulator::Finalize()
    {
        if(gscope.activeWindow == nullptr)
            return;

        // unload existence worlds
        for(World *pinned : Runtime::pinnedWorlds)
        {
            Runtime::internal_unload_world(pinned);
        }
        Runtime::pinnedWorlds.clear();

        for(World *pinned : Runtime::privateWorlds)
        {
            Runtime::internal_unload_world(pinned);
            RoninMemory::free(pinned);
        }
        Runtime::privateWorlds.clear();

        SDL_DestroyWindow(gscope.activeWindow);
        gscope.activeWindow = nullptr;

        Runtime::free_legacy_font();

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

    //////////////////////
    /// SIMULATE WORLD ///
    //////////////////////
    void RoninSimulator::Simulate()
    {
        SDL_Event event;
        SDL_DisplayMode displayMode;

        bool isQuitting = false;
        char title[96];

        if(gscope.activeWindow == nullptr)
        {
            ShowMessage("Engine not inited");
            return;
        }

        if(currentWorld == nullptr)
        {
            ShowMessage("World not loaded");
            return;
        }

        gscope.queueWatcher = {};
        internal_frames = 1;

        float ticksPrev2 = 0;
        std::uint32_t ticksPrev = 0;
        std::uint32_t lastFrames = 0;
        while(!isQuitting)
        {
            gscope.queueWatcher.delayFrameRate = Time::millis();

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
                        currentWorld->RequestUnload();
                        lastWorld = currentWorld;
                        gscope.config.conf = CONF_RENDER_NOCONF;
                        gscope.internalWorldLoaded = false;
                        currentWorld = nullptr;
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
                if(lastWorld)
                {
                    internal_unload_world(lastWorld);

                    // Free Private World
                    auto worldIter = privateWorlds.find(lastWorld);
                    if(worldIter != privateWorlds.end())
                    {
                        RoninMemory::free(lastWorld);
                        privateWorlds.erase(worldIter);
                    }

                    lastWorld = nullptr;
                }

                if(gscope.renderer != nullptr)
                {
                    // Destroy last renderer
                    SDL_DestroyRenderer(gscope.renderer);
                    gscope.renderer = nullptr;
                }

                if(currentWorld == nullptr)
                {
                    gscope.queueWatcher.delaySystem = Time::EndWatch();
                    goto end_simulate;
                }
                else
                {
                    // On Runtime Loading world

                    std::uint32_t renderFlags = SDL_RENDERER_TARGETTEXTURE;

                    if(gscope.config.renderBackend == RenderDriverInfo::RenderBackend::CPU)
                        renderFlags |= SDL_RENDERER_SOFTWARE;
                    else
                        renderFlags |= SDL_RENDERER_ACCELERATED;

                    // on first load level
                    gscope.renderer = SDL_CreateRenderer(gscope.activeWindow, -1, renderFlags);

                    if(gscope.renderer == nullptr)
                        ShowMessageFail(SDL_GetError());

                    // Set blendmode
                    SDL_SetRenderDrawBlendMode(gscope.renderer, SDL_BLENDMODE_BLEND);

                    // on swtiched and init resources, even require
                    Runtime::internal_load_world(preloadWorld);

                    currentWorld = preloadWorld;

                    // Init Internal World Timer IIWT
                    internal_init_timer();

                    if(!currentWorld->isHierarchy())
                    {
                        // init main object
                        currentWorld->irs->mainObject = create_empty_gameobject();
                        currentWorld->irs->mainObject->name("Main Object");
                        currentWorld->irs->mainObject->transform()->name("Root");
                        // pickup from renders
                        Matrix::matrix_remove(currentWorld->irs->mainObject->transform().get());
                    }

                    // Set Metric as default
                    currentWorld->irs->metricPixelsPerPoint = Vec2::one * defaultPixelsPerPoint;

                    gscope.internalWorldCanStart = false;

                    currentWorld->OnAwake();

                    preloadWorld = nullptr;
                    gscope.internalWorldLoaded = true;
                }
            }

            // end watcher
            gscope.queueWatcher.delaySystem = Time::EndWatch();

            // update level
            if(!currentWorld->irs->requestUnloading)
            {
                // begin watcher
                Time::BeginWatch();

                if(!gscope.internalWorldCanStart)
                {
                    currentWorld->OnStart();
                    gscope.internalWorldCanStart = true;
                }

                if(!currentWorld->irs->requestUnloading)
                {
                    currentWorld->OnUpdate();
                }
                // end watcher
                gscope.queueWatcher.delayExecWorld = Time::EndWatch();

                if(currentWorld->irs->requestUnloading)
                {
                    goto end_simulate; // break on unload state
                }
            }
            else
                goto end_simulate; // break on unload state

            // Render World objects
            level_render_world();

            // Run Collector
            Time::BeginWatch();

            if(!currentWorld->irs->requestUnloading)
                sepuku_run();

            // end watcher
            gscope.queueWatcher.delayHarakiring = Time::EndWatch();

            if(currentWorld->irs->requestUnloading)
                goto end_simulate; // break on unload state

            // begin watcher
            Time::BeginWatch();
            UI::native_draw_render(currentWorld->irs->gui);
            // end watcher
            gscope.queueWatcher.delayRenderGUI = Time::EndWatch();

            if(currentWorld->irs->requestUnloading)
                goto end_simulate; // break on unload state

            // begin watcher
            Time::BeginWatch();

            if(gscope.debugMode)
                internal_draw_debug();

            if(!lastWorld)
            {
                SDL_RenderPresent(gscope.renderer);
            }

            // end watcher
            gscope.queueWatcher.delayPresent += Time::EndWatch();

        end_simulate:

            // Capture delayed at last screen time
            gscope.queueWatcher.delayFrameRate = Time::millis() - gscope.queueWatcher.delayFrameRate;

            if(++internal_frames == 0)
                internal_frames = 1;

            std::uint32_t ticksf = Time::millis();
            if(ticksf - ticksPrev >= 1000)
            {
                gscope.queueWatcher.fps = internal_frames - lastFrames;
                lastFrames = internal_frames;
                ticksPrev = ticksf;
            }

            // update watcher
            gscope.lastWatcher = gscope.queueWatcher;

            if(!_watcher_time.empty())
                throw ronin_watcher_error();

            internal_delta_time = Math::Clamp01(gscope.queueWatcher.delayFrameRate / 1000.f) * internal_time_scale;
            internal_game_time += internal_delta_time;

            if(gscope.debugMode)
            {
                if(Time::startUpTime() > ticksPrev2)
                {
                    std::sprintf(
                        title,
                        "FPS:%u Memory:%sMiB, Ronin Objects:%s, Internal Objects:%s, Frames:%s",
                        gscope.queueWatcher.fps,
                        Math::NumBeautify(Perfomances::GetMemoryUsed() / 1024 / 1024).c_str(),
                        Math::NumBeautify(RoninMemory::total_allocated()).c_str(),
                        Math::NumBeautify(SDL_GetNumAllocations()).c_str(),
                        Math::NumBeautify(internal_frames).c_str());
                    SDL_SetWindowTitle(gscope.activeWindow, title);
                    ticksPrev2 = Time::startUpTime() + .5f; // updater per N seconds
                }
            }

            // update events
            internal_input_update_after();

            if(currentWorld == nullptr || currentWorld->irs->requestUnloading && preloadWorld == nullptr)
                break; // break on unload state
        }

        // unload world
        if(currentWorld)
        {
            Runtime::internal_unload_world(currentWorld);
            currentWorld = nullptr;
        }

        if(gscope.renderer)
        {
            SDL_DestroyRenderer(gscope.renderer);
            gscope.renderer = nullptr;
        }
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
            _lstrs[s - 1] += " ";

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
        if(currentWorld)
            currentWorld->RequestUnload();
    }

    bool RoninSimulator::LoadWorld(World *world, bool unloadPrevious)
    {
        const std::function<bool(void)> checks_queue[] {
            [=]() -> bool const
            {
                bool hasError = gscope.activeWindow == nullptr;
                if(hasError)
                    ronin_err("Engine not inited");
                return hasError;
            },
            [=]() -> bool const
            {
                bool hasError = world == nullptr;
                if(hasError)
                    ronin_err("World is not defined");
                return hasError;
            },
            [=]() -> bool const
            {
                bool hasError = currentWorld == world || lastWorld != nullptr || preloadWorld == world;
                if(hasError)
                    ronin_err("Current world is loading state. Failed.");
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

        if(unloadPrevious && currentWorld)
        {
            lastWorld = currentWorld;
            lastWorld->RequestUnload();
        }

        // cancelation of reload
        CancelReload();

        // switching as main
        if(currentWorld == nullptr)
            currentWorld = world;

        // preload world
        preloadWorld = world;

        gscope.internalWorldCanStart = false;
        gscope.internalWorldLoaded = false;

        return true;
    }

    bool RoninSimulator::LoadWorldAfterSplash(Runtime::World *world)
    {
        RoninSplashWorld *splashScreen;

        if(world == nullptr)
            return false;

        RoninMemory::alloc_self(splashScreen);
        splashScreen->nextWorld = world;
        makePrivate(splashScreen);
        return LoadWorld(splashScreen);
    }

    bool RoninSimulator::ReloadWorld()
    {
        if(currentWorld == nullptr || preloadWorld != nullptr)
        {
            ronin_log("Active world not loaded");
            return false;
        }

        // set config for reload
        gscope.config.conf |= CONF_RELOAD_WORLD;
        return true;
    }

    World *RoninSimulator::GetWorld()
    {
        return currentWorld;
    }

    bool RoninSimulator::CancelReload()
    {
        int last_flag = (gscope.config.conf & CONF_RELOAD_WORLD);
        // clear reload flag
        gscope.config.conf &= ~CONF_RELOAD_WORLD;
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
        // int mdIndex = SDL_GetNumDisplayModes(dpIndex);

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
                ronin_log(SDL_GetError());
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
            ronin_log(SDL_GetError());

        for(; num-- > 0;)
            vids.emplace_back(SDL_GetVideoDriver(num));

        return vids;
    }

    RoninSettings RoninSimulator::GetSettings()
    {
        RoninSettings *settings = reinterpret_cast<RoninSettings *>(alloca(sizeof(RoninSettings)));

        memset(settings, 0, sizeof(RoninSettings));

        if(!SDL_WasInit(SDL_INIT_VIDEO))
        {
            return *settings;
        }

        SDL_RendererInfo info;
        if(gscope.renderer != nullptr && SDL_GetRendererInfo(gscope.renderer, &info) == 0)
        {
            settings->renderBackend = (info.flags & SDL_RENDERER_SOFTWARE) ? RenderDriverInfo::RenderBackend::CPU : RenderDriverInfo::RenderBackend::GPU;
        }

        settings->brightness = SDL_GetWindowBrightness(gscope.activeWindow);

        if(SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY) != nullptr && !SDL_strcmp(SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY), "1"))
        {
            settings->textureQuality = RoninSettings::RenderTextureScaleQuality::Linear;
        }
        else
        {
            settings->textureQuality = RoninSettings::RenderTextureScaleQuality::Nearest;
        }

        const char *vsyncVal;
        settings->verticalSync = ((vsyncVal = SDL_GetHint(SDL_HINT_RENDER_VSYNC)) != nullptr && !std::strcmp(vsyncVal, "1"));

        SDL_GetWindowOpacity(gscope.activeWindow, &settings->windowOpacity);

        return *settings;
    }

    bool RoninSimulator::SetSettings(const RoninSettings &settings)
    {
        if(!SDL_WasInit(SDL_INIT_VIDEO))
        {
            return false;
        }

        RoninSettings refSettings = GetSettings();

        std::function<bool(void)> params[] {

            [&]() // Apply Render Backend
            {
                bool state = false;
                if(refSettings.renderBackend != settings.renderBackend)
                {
                    switch(settings.renderBackend)
                    {
                        case RenderDriverInfo::RenderBackend::CPU:
                            gscope.config.conf |= CONF_RENDER_SOFTWARE;
                            break;
                        case RenderDriverInfo::RenderBackend::GPU:
                            gscope.config.conf |= CONF_RENDER_CHANGED; // set as HARDWARE
                            break;
                    }
                    state = true;
                }

                return state;
            },
            [&]() // Apply Render Texture Scale Quality
            { return refSettings.textureQuality != settings.textureQuality && (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, (settings.textureQuality ? "1" : "0")) == SDL_TRUE); },
            // Apply Video Driver
            [&]() { return false; },
            // Apply Render Driver
            [&]() { return false; },
            // Apply Brightness
            [&]() { return (refSettings.brightness != settings.brightness && SDL_SetWindowBrightness(gscope.activeWindow, settings.brightness) == 0); },
            // Apply Window Opacity
            [&]() { return (refSettings.windowOpacity != settings.windowOpacity && SDL_SetWindowOpacity(gscope.activeWindow, settings.windowOpacity) == 0); },
            // Apply VSync
            [&]() { return (refSettings.verticalSync != settings.verticalSync && SDL_SetHint(SDL_HINT_RENDER_VSYNC, ((settings.verticalSync) ? "1" : "0"))); }};

        bool apply_any = false;
        for(auto &apply : params)
        {
            apply() && !apply_any && (apply_any = true);
        }

        if(apply_any)
        {
            ronin_log("Settings applied!");
        }

        return apply_any;
    }
} // namespace RoninEngine
