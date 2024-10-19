#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    /**
     * @class Time
     * @brief A class that provides time-related functionalities for the RoninEngine.
     *      * This class offers static methods to retrieve the current time, manage time scaling,
     * and monitor frame rates. It is designed to facilitate time management in games or
     * real-time applications using the RoninEngine.
     */
    class RONIN_API Time
    {
    public:
        /**
         * @brief Gets the current time in seconds since the RoninEngine started.
         * @return The current time in seconds.
         */
        static float time();

        /**
         * @brief Gets the startup time of the RoninEngine.
         * @return The startup realtime in seconds.
         */
        static float startUpTime();

        /**
         * @brief Gets the time elapsed since the last frame.
         * @return The time in seconds since the last frame.
         */
        static float deltaTime();

        /**
         * @brief Checks if the RoninEngine is currently paused.
         * @return True if the RoninEngine is paused, false otherwise.
         */
        static bool isPaused();

        /**
         * @brief Checks if the RoninEngine is currently playing.
         * @return True if the RoninEngine is playing, false otherwise.
         */
        static bool isPlaying();

        /**
         * @brief Gets the current time scale.
         * @return The current time scale factor.
         */
        static float GetTimeScale();

        /**
         * @brief Sets the time scale for the RoninEngine.
         * @param scale The new time scale factor. A value of 1.0 represents normal speed,
         *              while values less than 1.0 slow down time and values greater than 1.0 speed it up.
         */
        static void SetTimeScale(float scale);

        /**
         * @brief Gets the current frame number.
         * @return The current frame number as an unsigned 32-bit integer.
         */
        static std::uint32_t frame();

        /**
         * @brief Gets the current time in milliseconds since the RoninEngine started.
         * @return The current time in milliseconds as an unsigned 64-bit integer.
         */
        static std::uint64_t millis();

        /**
         * @brief Begins a time watch for measuring elapsed time.
         */
        static void BeginWatch();

        /**
         * @brief Ends the time watch and returns the elapsed time in milliseconds.
         * @return The elapsed time in milliseconds since the watch was started.
         */
        static std::uint32_t EndWatch();
    };

} // namespace RoninEngine::Runtime
