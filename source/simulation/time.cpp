#include "ronin.h"

namespace RoninEngine
{

    extern float timeScale, _lastTime, internal_time, intenal_delta_time;
    extern std::uint32_t intenal_frames; // framecounter
    extern std::uint32_t _startedTime;

    float Time::time() { return internal_time; }

    float Time::startUpTime() { return static_cast<float>(tickMillis() - _startedTime) / 1000; }

    float Time::deltaTime() { return intenal_delta_time; }

    bool Time::is_paused() { return timeScale == 0; }

    bool Time::is_playing() { return timeScale != 0; }

    float Time::get_time_scale() { return timeScale; }

    void Time::set_time_scale(float scale) { timeScale = Math::min(Math::max(scale, 0.F), 1.f); }

    std::uint32_t Time::frame() { return intenal_frames; }

    std::uint32_t Time::tickMillis() { return SDL_GetTicks(); }

} // namespace RoninEngine
