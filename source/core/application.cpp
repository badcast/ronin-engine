#include "ronin.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine
{
    namespace Runtime
    {
        float timeScale, _lastTime, internal_time, intenal_delta_time;

        std::uint32_t _startedTime;
        std::uint32_t intenal_frames; // framecounter

        extern GameObject* create_empty_gameobject();

        extern void manager_init();
    }

    static bool m_inited = false;
    static bool m_levelLoaded = false;
    static bool m_levelAccept = false;
    static Runtime::Level* destroyableLevel = nullptr;
    static SDL_Renderer* renderer = nullptr;
    static SDL_Window* windowOwner = nullptr;
    static Runtime::Level* m_level = nullptr;
    static bool isQuiting;

    Runtime::Vec2Int m_mousePoint;
    std::uint8_t mouseState;
    std::uint8_t lastMouseState;
    std::uint8_t mouseWheels;
    Runtime::Vec2 m_axis;
    bool text_inputState;

    void internal_init_TimeEngine()
    {
        internal_time = 0;
        _lastTime = 0;
        timeScale = 1;
        _startedTime = 0;
        _startedTime = TimeEngine::startUpTime();
    }

    void input_reset()
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
        if (!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG))
            fail("Fail init imageformats. (libPNG, libJPG) not defined");

        // init Audio system
        if (!Mix_Init(MIX_InitFlags::MIX_INIT_OGG | MIX_InitFlags::MIX_INIT_MP3))
            fail("Fail init audio.");

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024))
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

        // Brightness - Яркость
        SDL_SetWindowBrightness(windowOwner, 1);
    }

    void Application::close_window()
    {
        if (windowOwner) {
            request_quit();
            SDL_DestroyWindow(windowOwner);
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
            windowOwner = nullptr;
        }
    }

    void Application::request_quit() { isQuiting = true; }

    void Application::quit()
    {
        if (!m_inited)
            return;

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(windowOwner);

        UI::ui_free_controls();
        // sdl quit
        IMG_Quit();
        Mix_Quit();
        SDL_Quit();

        m_inited = false;
    }

    void Application::load_level(Level* level)
    {
        if (!level || m_level == level || windowOwner == nullptr)
            throw std::bad_cast();

        if (m_level) {
            destroyableLevel = m_level;
            m_level->unload();
        }

        if (!level->is_hierarchy()) {
            // init main object
            level->main_object = create_empty_gameobject();
            level->main_object->name("Main Object");
            level->main_object->transform()->name("Root");
            //pickup from renders
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

    bool Application::simulate()
    {
        char windowTitle[128];
        float fps;
        int delayed;
        SDL_Event event;
        SDL_WindowFlags wndFlags;
        float secPerFrame;
        SDL_DisplayMode displayMode;

        if (m_level == nullptr) {
            show_message("Level not loaded");
            return false;
        }

        if (windowOwner == nullptr) {
            show_message("Window not inited");
            return false;
        }

        internal_init_TimeEngine();

        displayMode = Application::get_display_mode();
        secPerFrame = 1000.f / displayMode.refresh_rate; // refresh screen from Monitor Settings
        while (!isQuiting) {
            // update events
            input_reset();
            delayed = TimeEngine::tickMillis();
            wndFlags = static_cast<SDL_WindowFlags>(SDL_GetWindowFlags(Application::get_window()));
            while (SDL_PollEvent(&event)) {
                input::Update_Input(&event);
                if (event.type == SDL_QUIT)
                    isQuiting = true;
            }

            if ((wndFlags & SDL_WindowFlags::SDL_WINDOW_MINIMIZED) != SDL_WindowFlags::SDL_WINDOW_MINIMIZED) {
                // set default color
                SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, SDL_ALPHA_OPAQUE); // back color for clear

                // Clearing
                SDL_RenderClear(renderer);

                input::movement_update();

                if (!m_levelLoaded) {
                    // free cache
                    if (destroyableLevel) {
                        destroyableLevel->onUnloading();

                        RoninMemory::free(destroyableLevel);

                        destroyableLevel = nullptr;
                    }

                    m_levelLoaded = false;
                    // Start first init
                    m_level->awake();
                    m_levelLoaded = true;
                } else {
                    // update level
                    if (!m_levelAccept) {
                        SDL_RenderFlush(renderer); // flush renderer before first render
                        m_level->start();
                        m_levelAccept = true;
                    } else {
                        m_level->update();
                    }

                    m_level->level_render_world(renderer);
                    if (Camera::main_camera())
                        m_level->onDrawGizmos(); // Draw gizmos

                    // Set scale as default
                    SDL_RenderSetScale(renderer, 1, 1);

                    m_level->level_render_ui(renderer);

                    if (!destroyableLevel) {
                        SDL_RenderPresent(renderer);
                        m_level->lateUpdate();
                        m_level->level_render_world_late(renderer);
                    }
                }
            }

            delayed = TimeEngine::tickMillis() - delayed;

            // if (Time::startUpTime() > fpsRound)
            {
                fps = intenal_frames / (TimeEngine::startUpTime());
                if (fps > 2000000) {
                    fps = 0;
                }

                std::sprintf(
                    windowTitle,
                    "FPS:%d Memory:%luMiB, "
                    "Ronin_Allocated:%lu, SDL_Allocated:%d",
                    static_cast<int>(fps), get_process_sizeMemory() / 1024 / 1024, RoninMemory::total_allocated(), SDL_GetNumAllocations());
                SDL_SetWindowTitle(Application::get_window(), windowTitle);
                // fpsRound = Time::startUpTime() + 1;  // updater per 1 seconds
            }

            intenal_delta_time = delayed;
            if (intenal_delta_time > 1) {
                intenal_delta_time -= 1;
                intenal_delta_time = Math::Clamp01(intenal_delta_time);
            }
            internal_time += 0.001f * secPerFrame;
            // delay elapsed
            SDL_Delay(Math::max(0, static_cast<int>(secPerFrame - delayed)));
            ++intenal_frames;
        }

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
