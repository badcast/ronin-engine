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

    struct TimingWatcher
    {
        std::uint32_t ms_wait_exec_world;
        std::uint32_t ms_wait_exec_scripts;
        std::uint32_t ms_wait_render_world;
        std::uint32_t ms_wait_render_gui;
        std::uint32_t ms_wait_render_gizmos;
        std::uint32_t ms_wait_destructions;
        std::uint32_t ms_wait_internal_instructions;
        std::uint32_t ms_wait_frame;
    };

    class RONIN_API RoninSimulator
    {
    public:
        /**
         * @brief Initializes the RoninEngine library.
         */
        static void Init();

        /**
         * @brief Closes the library and performs necessary cleanup.
         */
        static void Finalize();

        /**
         * @brief Displays the main simulation frame.
         *
         * @param resolution The desired resolution for the frame.
         * @param fullscreen Whether to display in fullscreen mode.
         */
        static void Show(const Resolution &resolution, bool fullscreen = false);

        /**
         * @brief Loads a simulation world for simulation.
         *
         * @param world The world to load.
         * @param unloadPrevious Whether to unload the previous world.
         */
        static void LoadWorld(Runtime::World *world, bool unloadPrevious = true);

        /**
         * @brief Simulates the loaded world.
         */
        static void Simulate();

        /**
         * @brief Requests to quit the simulation and destroys the current world.
         */
        static void RequestQuit();

        /**
         * @brief Retrieves the current screen resolution.
         *
         * @return The current screen resolution.
         */
        static Resolution GetCurrentResolution();

        /**
         * @brief Enumerates the available screen resolutions of the active display.
         *
         * @return A list of available screen resolutions.
         */
        static std::list<Resolution> EnumerateResolutions();

        /**
         * @brief Sets the display resolution of the active display.
         *
         * @param new_resolution The new resolution to set.
         * @return True if the resolution was set successfully, false otherwise.
         */
        static bool SetDisplayResolution(const Resolution &new_resolution);

        /**
         * @brief Sets the active window to fullscreen mode.
         *
         * @param mode The desired fullscreen mode.
         * @return True if fullscreen mode was set successfully, false otherwise.
         */
        static bool SetFullscreenMode(FullscreenMode mode);

        /**
         * @brief Retrieves timing information from the simulation.
         *
         * @return TimingWatcher containing timing information.
         */
        static TimingWatcher GetTimingWatches();

        /**
         * @brief Retrieves version information of the linked library.
         *
         * @return VersionInfo containing version details.
         */
        static VersionInfo GetVersion();

        /**
         * @brief Retrieves the linked targets system (e.g., ui, ai, res, etc.).
         *
         * @return Linked targets information.
         */
        static const char *GetLinkedTargets();

        /**
         * @brief Toggles debug mode to display FPS, frametime, render time, gizmos, etc.
         *
         * @param state The state of debug mode (on/off).
         */
        static void SetDebugMode(bool state);

        /**
         * @brief Handles a failure during a back operation.
         */
        static void BreakSimulate(void);

        /**
         * @brief Displays a message.
         *
         * @param message The message to display.
         */
        static void ShowMessage(const std::string &message);

        /**
         * @brief Handles a failure by displaying an error message.
         *
         * @param message The error message.
         */
        static void ShowMessageFail(const std::string &message);

        /**
         * @brief Handles an out-of-memory failure by terminating the application.
         */
        static void Kill();
    };

} // namespace RoninEngine
