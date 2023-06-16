#pragma once
#include "Object.h"

namespace RoninEngine
{
    struct Resolution {
        int width;
        int height;
    };

    struct ScoreWatcher {
        std::uint32_t ms_wait_exec_level;
        std::uint32_t ms_wait_exec_scripts;
        std::uint32_t ms_wait_render_collect;
        std::uint32_t ms_wait_render_level;
        std::uint32_t ms_wait_render_ui;
        std::uint32_t ms_wait_render_gizmos;
        std::uint32_t ms_wait_destructions;
        std::uint32_t ms_wait_internal_instructions;
        std::uint32_t ms_wait_frame;
    };

    class RONIN_API Application
    {
    public:
        // First initing RoninEngine Library
        static void init();
        // Close and Utilize
        static void utilize();
        // Create main window for renderer
        static void create_window(const int width, const int height, bool fullscreen = false);
        // Load world for simmulate
        static void load_world(Runtime::World* world, bool unloadPrevious = true);
        // Simulate world
        static bool simulate();
        // Get display mode
        static SDL_DisplayMode get_display_mode();
        // Request sent Quiting (Destroy current world)
        static void request_quit();
        // Get SDL Window
        static SDL_Window* get_window();
        // Get SDL Renderer
        static SDL_Renderer* get_renderer();
        // Get resolution of screen
        static Resolution get_resolution();
        // Get timming watches
        static ScoreWatcher get_watches();

        // Message handler

        static void back_fail(void);
        static void show_message(const std::string& message);
        static void fail(const std::string& message) throw();
        static void fail_oom_kill() throw();
    };

} // namespace RoninEngine
