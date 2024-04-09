#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Time
    {
    public:
        static float time();
        static float startUpTime();
        static float deltaTime();
        static bool IsPaused();
        static bool IsPlaying();
        static float GetTimeScale();
        static void SetTimeScale(float scale);
        static std::uint32_t frame();
        static std::uint64_t millis();
        static void BeginWatch();
        static std::uint32_t EndWatch();
    };
} // namespace RoninEngine::Runtime
