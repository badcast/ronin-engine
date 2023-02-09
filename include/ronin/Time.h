#ifndef _TIME_H_
#define _TIME_H_

#include "begin.h"

namespace RoninEngine
{
    class SHARK Time
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
        static std::uint32_t tickMillis();
    };
} // namespace RoninEngine
#endif
