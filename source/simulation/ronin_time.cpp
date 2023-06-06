#include "ronin.h"

namespace RoninEngine::Runtime
{

    extern float internal_time_scale, internal_game_time, internal_delta_time;
    extern std::uint32_t internal_frames; // framecounter
    extern std::uint32_t internal_start_engine_time;
    extern std::vector<std::uint32_t> _watcher_time;

    float TimeEngine::time() { return internal_game_time; }

    float TimeEngine::startUpTime() { return static_cast<float>(tick_millis()) / 1000; }

    float TimeEngine::deltaTime() { return internal_delta_time; }

    bool TimeEngine::is_paused() { return internal_time_scale == 0; }

    bool TimeEngine::is_playing() { return internal_time_scale != 0; }

    float TimeEngine::get_time_scale() { return internal_time_scale; }

    void TimeEngine::set_time_scale(float scale) { internal_time_scale = Math::clamp01(scale); }

    std::uint32_t TimeEngine::frame() { return internal_frames; }

    std::uint32_t TimeEngine::tick_millis() { return SDL_GetTicks(); }

    void TimeEngine::begin_watch() { _watcher_time.push_back(tick_millis()); }

    std::uint32_t TimeEngine::end_watch()
    {
        std::uint32_t time;
        if (_watcher_time.empty())
            throw std::runtime_error("begin_watch() method is not runned");
        time = tick_millis() - _watcher_time.back();
        _watcher_time.pop_back();
        return time;
    }

} // namespace RoninEngine
