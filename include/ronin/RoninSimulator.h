#pragma once

#include "begin.h"

namespace RoninEngine
{
    enum InitializeFlags
    {
        All = -1,
        Graphics = 1,
        Audio = 2
    };

    enum RendererFlags
    {
        /** The renderer is a software fallback */
        RendererSoftware = 0x1,
        /** The renderer uses hardware acceleration */
        RendererAccelerated = 0x2,
        /** Present is synchronized with the refresh rate */
        RendererPresentVSync = 0x4,
    };

    enum RenderBackend
    {
        GPU,
        CPU
    };

    struct RenderDriverInfo
    {
        std::string name;
        RendererFlags rendererFlags;
        RenderBackend rendererBackend;
        int maxTextureWidth;
        int maxTextureHeight;

        RenderDriverInfo(
            const std::string &name, RendererFlags rendererFlags, RenderBackend rendererBackend, int maxTextureWidth, int maxTextureHeight)
            : name(name),
              rendererFlags(rendererFlags),
              rendererBackend(rendererBackend),
              maxTextureWidth(maxTextureWidth),
              maxTextureHeight(maxTextureHeight)
        {
        }
    };

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
        Version SDL_GFX_Version;
    };

    struct RoninSettings
    {
        RenderBackend selectRenderBackend;
        std::uint8_t selectRenderDriver;
        std::uint8_t selectVideoDriver;
        float brightness;
        float windowOpacity;
    };

    struct RONIN_API Resolution
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

        // Get maximum resolution of the display
        static Resolution GetMaxResolution();

        // Get minimum resolution of the display
        static Resolution GetMinResolution();

        // Get middle resolution of the display
        static Resolution GetMidResolution();
    };

    enum class FullscreenMode
    {
        Desktop,
        Display
    };

    struct TimingWatcher
    {
        std::uint32_t delayExecWorld;
        std::uint32_t delayExecScripts;
        std::uint32_t delayRenderWorld;
        std::uint32_t delayRenderGUI;
        std::uint32_t delayRenderGizmos;
        std::uint32_t delayHarakiring;
        std::uint32_t delaySystem;
        std::uint32_t delayFrameRate;
    };

    class RONIN_API RoninSimulator
    {
    public:
        /**
         * @brief Initializes the RoninEngine library.
         * @param flags The InitializeFlags for init switch.
         */
        static void Init(InitializeFlags flags = InitializeFlags::All);

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
         * @return result of loaded
         */
        static bool LoadWorld(Runtime::World *world, bool unloadPrevious = true);

        /**
         * @brief Reload current world
         * Unload and Load
         *
         * @return result of reloaded
         * @see CancelReload
         */
        static bool ReloadWorld();

        /**
         * @brief Cancelation a reloading current world
         *
         * @return result of cancelled
         * @see ReloadWorld
         */
        static bool CancelReload();

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
         * @brief Retrieves the current display resolution.
         *
         * @return The current display resolution.
         */
        static Resolution GetCurrentDisplayResolution();

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
        static bool SetWindowResolution(const Resolution &new_resolution);

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
         * @brief Write log
         * @param str The message string
         */
        static void Log(const char *str);

        /**
         * @brief Handles an out-of-memory failure by terminating the application.
         */
        static void Kill();

        /**
         * @brief Enumerate the active render drivers
         * @return list active render drivers
         * @see SetSettings, GetSettings
         */
        static std::vector<RenderDriverInfo> EnumerateRenderDrivers();

        /**
         * @brief Enumerate the active video drivers
         * @return List video drivers
         */
        static std::vector<std::string> EnumerateVideoDrivers();

        /**
         * @brief Get installed settings
         * @param settings
         * @see SetSettings, GetRenderDrivers
         */
        static void GetSettings(RoninSettings *settings);

        /**
         * @brief Set the settings as new
         * @param settings for set
         * @return Result of the set
         * @see GetSettings, GetRenderDrivers
         */
        static bool SetSettings(const RoninSettings *settings);
    };

} // namespace RoninEngine
