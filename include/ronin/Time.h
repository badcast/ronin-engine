#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API TimeEngine
    {
    public:
        RONIN_API static float time();
        RONIN_API static float startUpTime();
        RONIN_API static float deltaTime();
        RONIN_API static bool is_paused();
        RONIN_API static bool is_playing();
        RONIN_API static float get_time_scale();
        RONIN_API static void set_time_scale(float scale);
        RONIN_API static std::uint32_t frame();
        RONIN_API static std::uint32_t tickMillis();
    };
} // namespace RoninEngine
