#pragma once

#include "begin.h"

namespace RoninEngine::Runtime::MusicPlayer
{
    // Get audio clip
    RONIN_API MusicClip *clip();
    // Set audio clip
    RONIN_API void clip(MusicClip *clip);
    // Get volume at range 0.0 ... 1.0
    RONIN_API float volume();
    // Set volume at range 0.0 ... 1.0
    RONIN_API void volume(float value);
    // Set Clip position to begin
    RONIN_API void rewind();
    // Set play audio source
    RONIN_API void play(bool loop = true);
    // Set pause state
    RONIN_API void pause();
    // Set stoping state
    RONIN_API void stop();
    // Is played now?
    RONIN_API bool is_playing();
    // Is paused now?
    RONIN_API bool is_paused();
    // Get length music
    RONIN_API double duration();
    // Set music position
    RONIN_API void set_position(double value);
    // Get music position
    RONIN_API double get_position();
} // namespace RoninEngine::Runtime::MusicPlayer
