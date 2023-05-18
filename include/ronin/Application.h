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
        static void init();

        static void quit();

        static void create_window(const int& width, const int& height, bool fullscreen = false);
        static void close_window();
        static void load_level(Runtime::Level* level);
        static bool simulate();
        static void get_screen(const char* filename);
        static SDL_Surface* get_screen();
        static SDL_DisplayMode get_display_mode();
        static void request_quit();
        static SDL_Window* get_window();
        static SDL_Renderer* get_renderer();
        static Resolution get_resolution();

        static void back_fail(void);
        static void show_message(const std::string& message);
        static void fail(const std::string& message);
        static void fail_oom_kill() throw();
    };

} // namespace RoninEngine
