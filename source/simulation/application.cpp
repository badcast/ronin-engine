
#include <thread>

#include "ronin.h"

using namespace UI;

namespace RoninEngine {
static Level* destroyableLevel = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Window* window = nullptr;
static bool m_inited = false;
static bool m_levelAccept = false;
static Level* m_level = nullptr;
static bool m_levelLoaded = false;
static bool isQuiting;

void Application::Init(const std::uint32_t& width, const std::uint32_t& height) {
    char errorStr[128];
    if (m_inited) return;

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO)) fail("Fail init main system.");

    // init graphics
    if (!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG)) fail("Fail init imageformats. (libPNG, libJPG) not defined");

    // init Audio system
    if (!Mix_Init(MIX_InitFlags::MIX_INIT_OGG | MIX_InitFlags::MIX_INIT_MP3)) fail("Fail init audio.");

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)) fail("Fail open audio.");

    window = SDL_CreateWindow("Ronin Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

    if (!window) fail(SDL_GetError());

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
    if (!renderer) fail(SDL_GetError());

    // Brightness - Яркость
    SDL_SetWindowBrightness(window, 1);

    LoadGame();
    m_inited = true;
    isQuiting = false;
}

void Application::RequestQuit() { isQuiting = true; }

void Application::Quit() {
    if (!m_inited) return;

    GC::gc_free();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Free_Controls();
    // sdl quit
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();

    m_inited = false;
}

void Application::LoadGame() {
    GC::gc_lock();

    GC::CheckResources();

    // initialize GC
    GC::gc_init();

    std::string path = getDataFrom(FolderKind::LOADER);
    std::string temp = path + "graphics.conf";
    GC::LoadImages(temp.c_str());

    // load textures
    path = getDataFrom(FolderKind::LOADER);
    temp = path + "textures.conf";
    GC::LoadImages(temp.c_str());

    //Загрузк шрифта и оптимизация дэффектов
    UI::Initialize_Fonts(true);

    //Инициализация инструментов
    InitalizeControls();

    // Set cursor
    // SDL_SetCursor(GC::GetCursor("cursor", {1, 1}));

    Levels::Level_Init();
}

void Application::LoadedLevel() {
    if (destroyableLevel) {
        destroyableLevel->onUnloading();

        GC::gc_unalloc(destroyableLevel);

        destroyableLevel = nullptr;
//BUG: DANGER ZONE
         GC::UnloadUnused();
        //GC::gc_free_source();
    }

    m_levelLoaded = true;

    // capture memory as GC
    GC::gc_unlock();

    m_level->awake();
}

void Application::LoadLevel(Level* level) {
    if (!level || m_level == level) throw std::bad_cast();

    if (m_level) {
        destroyableLevel = m_level;
        m_level->Unload();
    }

    if (!level->is_hierarchy()) {
        // init main object
        level->main_object = create_empty_gameobject();
        level->main_object->name("Main Object");
        level->main_object->transform()->name("Root");
    }

    m_level = level;
    m_levelAccept = false;
    m_levelLoaded = false;
}

SDL_Surface* Application::ScreenShot() {
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

void Application::ScreenShot(const char* filename) {
    SDL_Surface* surf = ScreenShot();
    IMG_SavePNG(surf, filename);
    SDL_FreeSurface(surf);
}

SDL_DisplayMode Application::getDisplayMode() {
    SDL_DisplayMode display;
    SDL_GetWindowDisplayMode(window, &display);
    return display;
}

RoninEngine::Resolution RoninEngine::Application::getResolution() {
    Resolution res;
    SDL_RendererInfo rf;
    SDL_GetRendererInfo(renderer, &rf);

    SDL_GetWindowSize(window, &res.width, &res.height);
    return res;
}

bool Application::Simulate() {
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

    Time::Init_TimeEngine();

    displayMode = Application::getDisplayMode();
    secPerFrame = 1000.f / displayMode.refresh_rate;  // refresh screen from Monitor Settings
    while (!isQuiting) {
        // update events
        input::Reset();
        delayed = Time::tickMillis();
        wndFlags = static_cast<SDL_WindowFlags>(SDL_GetWindowFlags(Application::GetWindow()));
        while (SDL_PollEvent(&event)) {
            input::Update_Input(&event);
            if (event.type == SDL_QUIT) isQuiting = true;
        }

        if ((wndFlags & SDL_WindowFlags::SDL_WINDOW_MINIMIZED) != SDL_WindowFlags::SDL_WINDOW_MINIMIZED) {
            // set default color
            SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, SDL_ALPHA_OPAQUE);  // back color for clear

            // Clearing
            SDL_RenderClear(renderer);

            input::movement_update();

            if (!m_levelLoaded) {
                // free cache
                LoadedLevel();
                m_levelLoaded = true;
            } else {
                // update level
                if (!m_levelAccept) {
                    SDL_RenderFlush(renderer);  // flush renderer before first render
                    m_level->start();
                    m_levelAccept = true;
                } else {
                    m_level->update();
                }

                m_level->RenderLevel(renderer);
                if (Camera::mainCamera()) m_level->onDrawGizmos();  // Draw gizmos

                // Set scale as default
                SDL_RenderSetScale(renderer, 1, 1);

                m_level->RenderUI(renderer);

                if (!destroyableLevel) {
                    SDL_RenderPresent(renderer);
                    m_level->lateUpdate();
                    m_level->RenderSceneLate(renderer);
                }
            }
        }

        delayed = Time::tickMillis() - delayed;

        ++Time::m_frames;  // framecounter
        if (Time::m_frames == 0) Time::m_frames = 1;

        // if (Time::startUpTime() > fpsRound)
        {
            fps = (Time::m_frames) / Time::startUpTime();
            std::sprintf(windowTitle,
                         "Ronin Engine (Debug) FPS:%d Memory:%luMiB, "
                         "GC_Allocated:%lu, SDL_Allocated:%d",
                         static_cast<int>(fps), get_process_sizeMemory() / 1024 / 1024, GC::gc_total_allocated(), SDL_GetNumAllocations());
            SDL_SetWindowTitle(Application::GetWindow(), windowTitle);
            // fpsRound = Time::startUpTime() + 1;  // updater per 1 seconds
        }

        Time::m_deltaTime = delayed / secPerFrame;  // get deltas
        Time::m_deltaTime = Math::Clamp01(Time::m_deltaTime);

        Time::m_time += 0.001f * Math::ceil(secPerFrame);

        // delay elapsed
        SDL_Delay(Math::max(0, static_cast<int>(secPerFrame - delayed)));
    }

    return isQuiting;
}

SDL_Window* Application::GetWindow() { return window; }

SDL_Renderer* Application::GetRenderer() { return renderer; }

void Application::back_fail(void) { exit(EXIT_FAILURE); }

void Application::show_message(const std::string& message) {
    SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE, "%s", message.c_str());
    printf("%s\n", message.c_str());
    SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, nullptr, message.c_str(), window);
}

void Application::fail(const std::string& message) {
    std::string _template = message;
    char _temp[32]{0};
    tm* ltime;
    time_t tt;
    time(&tt);

    ltime = localtime(&tt);
    strftime(_temp, sizeof(_temp), "%d.%m.%y %H:%M:%S", ltime);
    _template += "\n\n\t";
    _template += _temp;

    fprintf(stderr, "%s", _template.data());

    SDL_LogMessage(SDL_LogCategory::SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_CRITICAL, "%s", _template.c_str());
    SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, "Ronin Engine: failed", _template.c_str(), window);
    back_fail();
}

void Application::fail_OutOfMemory() { fail("Out of memory!"); }
}  // namespace RoninEngine
