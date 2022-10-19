#include "framework.h"

namespace RoninEngine {
float Time::timeScale, Time::_lastTime, Time::m_time, Time::m_deltaTime;
uint32_t Time::m_frames;
float _startedTime = 0;

void Time::Init_TimeEngine() {
    m_time = 0;
    _lastTime = 0;
    timeScale = 1;
    _startedTime = 0;
    _startedTime = startUpTime();
}

float Time::time() { return m_time; }

float Time::startUpTime() { return static_cast<float>(tickMillis() - _startedTime)/1000; }

float Time::deltaTime() { return m_deltaTime; }

bool Time::is_paused() { return timeScale == 0; }

float Time::get_time_scale() { return Time::timeScale; }

void Time::set_time_scale(float scale) { Time::timeScale = Math::min(Math::max(scale, 0.F), 1.f); }

std::uint32_t Time::frame() { return m_frames; }

std::uint32_t Time::tickMillis() { return SDL_GetTicks(); }

}  // namespace RoninEngine
