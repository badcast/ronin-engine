#include "ronin.h"

namespace RoninEngine::Runtime
{

    extern float internal_time_scale, internal_game_time, internal_delta_time;
    extern std::uint32_t internal_frames; // framecounter
    extern std::uint32_t internal_start_engine_time;
    extern std::vector<std::uint32_t> _watcher_time;

    float TimeEngine::time()
    {
        return internal_game_time;
    }

    float TimeEngine::startUpTime()
    {
        return static_cast<float>(millis()) / 1000;
    }

    float TimeEngine::deltaTime()
    {
        return internal_delta_time;
    }

    bool TimeEngine::IsPaused()
    {
        return internal_time_scale == 0;
    }

    bool TimeEngine::IsPlaying()
    {
        return internal_time_scale != 0;
    }

    float TimeEngine::GetTimeScale()
    {
        return internal_time_scale;
    }

    void TimeEngine::SetTimeScale(float scale)
    {
        internal_time_scale = Math::Clamp(scale, 0.f, 2.f);
    }

    std::uint32_t TimeEngine::frame()
    {
        return internal_frames;
    }

    std::uint64_t TimeEngine::millis()
    {
        return SDL_GetTicks64();
    }

    void TimeEngine::BeginWatch()
    {
        _watcher_time.push_back(millis());
    }

    std::uint32_t TimeEngine::EndWatch()
    {
        std::uint32_t time;
        if(_watcher_time.empty())
            throw std::runtime_error("begin_watch() method is not runned");
        time = millis() - _watcher_time.back();
        _watcher_time.pop_back();
        return time;
    }

} // namespace RoninEngine::Runtime
