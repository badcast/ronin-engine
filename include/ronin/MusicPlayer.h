#pragma once

#include "begin.h"

namespace RoninEngine::Runtime::MusicPlayer
{
    // Get audio clip
    MusicClip* clip();
    // Set audio clip
    void clip(MusicClip* clip);
    // Get volume at range 0.0 ... 1.0
    float volume();
    // Set volume at range 0.0 ... 1.0
    void volume(float value);
    // Set Clip position to begin
    void rewind();
    // Set play audio source
    void play(bool loop = true);
    // Set pause state
    void pause();
    // Set stoping state
    void stop();
    // Is played now?
    bool is_playing();
    // Is paused now?
    bool is_paused();
    // Get length music
    double duration();
    // Set music position
    void set_position(double value);
    // Get music position
    double get_position();
} // namespace RoninEngine::Runtime
