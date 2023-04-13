#pragma once
#include "Object.h"

namespace RoninEngine
{
    struct Resolution {
        int width;
        int height;
    };

    class RONIN_API Application
    {
    public:
        static void init(const std::uint32_t& width, const std::uint32_t& height, bool initResources = true);
        static void Quit();

        static void loadLevel(Level* level);
        static bool simulate();
        static void getScreen(const char* filename);
        static SDL_Surface* getScreen();
        static SDL_DisplayMode getDisplayMode();
        static void requestQuit();
        static SDL_Window* getWindow();
        static SDL_Renderer* getRenderer();
        static Resolution getResolution();

        static void back_fail(void);
        static void show_message(const std::string& message);
        static void fail(const std::string& message);
        static void fail_OutOfMemory();
    };

} // namespace RoninEngine
