#include "ronin.h"

namespace RoninEngine::Runtime
{

    extern float internal_time_scale, internal_game_time, internal_delta_time;
    extern std::uint32_t internal_frames; // framecounter
    extern std::vector<std::uint32_t> _watcher_time;

    float Time::time()
    {
        return internal_game_time;
    }

    float Time::startUpTime()
    {
        return static_cast<float>(millis()) / 1000;
    }

    float Time::deltaTime()
    {
        return internal_delta_time;
    }

    bool Time::isPaused()
    {
        return internal_time_scale == 0;
    }

    bool Time::isPlaying()
    {
        return internal_time_scale != 0;
    }

    float Time::GetTimeScale()
    {
        return internal_time_scale;
    }

    void Time::SetTimeScale(float scale)
    {
        internal_time_scale = Math::Clamp(scale, 0.f, 2.f);
    }

    std::uint32_t Time::frame()
    {
        return internal_frames;
    }

    std::uint64_t Time::millis()
    {
        return SDL_GetTicks64();
    }

    // TODO: Make Securilly container for Watcher

    void Time::BeginWatch()
    {
        _watcher_time.push_back(millis());
    }

    std::uint32_t Time::EndWatch()
    {
        std::uint32_t time;

        if(_watcher_time.empty())
            throw std::runtime_error("begin_watch() method is not runned");

        time = Math::Max<std::uint32_t>(0, millis() - _watcher_time.back());

        _watcher_time.pop_back();

        return time;
    }

} // namespace RoninEngine::Runtime
