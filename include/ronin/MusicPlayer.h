#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API MusicPlayer
    {
    public:
        // Get audio clip
        static MusicClip *getClip();
        // Set audio clip
        static void setClip(MusicClip *clip);
        // Get volume at range 0.0 ... 1.0
        static float getVolume();
        // Set volume at range 0.0 ... 1.0
        static void setVolume(float value);
        // Set Clip position to begin
        static void Rewind();
        // Set play audio source
        static void Play(bool loop = false);
        // Set pause state
        static void Pause();
        // Set stoping state
        static void Stop();
        // Is played now?
        static bool isPlaying();
        // Is paused now?
        static bool isPaused();
        // Get length music
        static double getDuration();
        // Set music position
        static void setPosition(double value);
        // Get music position
        static double getPosition();
        // Get music state
        static AudioState getState();
        // Get music state is loop
        static bool isLoop();
    };
} // namespace RoninEngine::Runtime
