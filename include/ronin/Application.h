#pragma once
#include "Object.h"

namespace RoninEngine
{
    struct Resolution {
        int width;
        int height;
        // int hz;
    };

    class SHARK Application
    {
        static void LoadGame();
        static void LoadedLevel();

    public:
        static void Init(const std::uint32_t& width, const std::uint32_t& height);
        static void Quit();

        static void LoadLevel(Level* level);
        static bool Simulate();
        static void ScreenShot(const char* filename);
        static SDL_Surface* ScreenShot();
        static SDL_DisplayMode getDisplayMode();
        static void RequestQuit();
        static SDL_Window* GetWindow();
        static SDL_Renderer* GetRenderer();
        static Resolution getResolution();

        static void back_fail(void);
        static void show_message(const std::string& message);
        static void fail(const std::string& message);
        static void fail_OutOfMemory();
    };

} // namespace RoninEngine
