#pragma once
#include "Object.h"

namespace RoninEngine
{
    struct Version
    {
        std::uint8_t major;
        std::uint8_t minor;
        std::uint8_t patch;
    };
    struct VersionInfo
    {
        Version Engine_Version;
        Version SDL_Version;
        Version SDL_TTF_Version;
        Version SDL_IMG_Version;
        Version SDL_Mix_Version;
    };

    struct Resolution
    {
        // Display width
        int width;
        // Display height
        int height;
        // Display refresh rate
        int hz;

        Resolution(int Width, int Height, int HZ = 0) : width(Width), height(Height), hz(HZ)
        {
        }
    };

    enum class FullscreenMode
    {
        Desktop,
        Display
    };

    struct ScoreWatcher
    {
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

    class RONIN_API RoninSimulator
    {
    public:
        // First initing RoninEngine Library
        static void init();
        // Close and Utilize
        static void utilize();
        // Show main frame for simulation
        static void show(const Resolution &resolution, bool fullscreen = false);
        // Load world for simmulate
        static void load_world(Runtime::World *world, bool unloadPrevious = true);
        // Simulate world
        static void simulate();
        // Request sent Quiting (Destroy current world)
        static void request_quit();
        // Get resolution of screen
        static Resolution get_current_resolution();
        // Get resolution of active display
        static std::list<Resolution> get_display_resolutions();
        // Set resolution of active display
        static bool set_display_resolution(const Resolution &new_resolution);
        // Set active window to fullscreen mode
        static bool set_display_fullscreen(FullscreenMode mode);
        // Get timming watches
        static ScoreWatcher get_watches();
        // Get linked version
        static VersionInfo get_version();

        // Message handler

        static void back_fail(void);
        static void show_message(const std::string &message);
        static void fail(const std::string &message);
        static void fail_oom_kill();
    };

} // namespace RoninEngine
