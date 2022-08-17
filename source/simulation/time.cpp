#include "ronin.h"

namespace RoninEngine::Runtime
{

    extern float timeScale, _lastTime, internal_time, intenal_delta_time;
    extern std::uint32_t intenal_frames; // framecounter
    extern std::uint32_t _startedTime;

    float TimeEngine::time() { return internal_time; }

    float TimeEngine::startUpTime() { return static_cast<float>(tickMillis() - _startedTime) / 1000; }

    float TimeEngine::deltaTime() { return intenal_delta_time; }

    bool TimeEngine::is_paused() { return timeScale == 0; }

    bool TimeEngine::is_playing() { return timeScale != 0; }

    float TimeEngine::get_time_scale() { return timeScale; }

    void TimeEngine::set_time_scale(float scale) { timeScale = Math::min(Math::max(scale, 0.F), 1.f); }

    std::uint32_t TimeEngine::frame() { return intenal_frames; }

    std::uint32_t TimeEngine::tickMillis() { return SDL_GetTicks(); }

} // namespace RoninEngine
