#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API TimeEngine
    {
    public:
        static float time();
        static float startUpTime();
        static float deltaTime();
        static bool is_paused();
        static bool is_playing();
        static float get_time_scale();
        static void set_time_scale(float scale);
        static std::uint32_t frame();
        static std::uint64_t tick_millis();
        static void begin_watch();
        static std::uint32_t end_watch();
    };
} // namespace RoninEngine::Runtime
