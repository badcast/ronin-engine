#ifndef _TIME_H_____
#define _TIME_H_____

#include "dependency.h"

namespace RoninEngine {
class SHARK Time {
    friend Application;

   private:
    static float timeScale, _lastTime, m_time, m_deltaTime;
    static std::uint32_t m_frames;

   public:
    static void Init_TimeEngine();
    static float time();
    static float startUpTime();
    static float deltaTime();
    static bool is_paused();
    static float get_time_scale();
    static void set_time_scale(float scale);
    static std::uint32_t frame();
    static std::uint32_t tickMillis();
};
}  // namespace RoninEngine
#endif
