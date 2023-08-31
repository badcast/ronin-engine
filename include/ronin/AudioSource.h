#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API AudioSource : public Component
    {
    protected:
        struct AudioSourceData *data;

    public:
        AudioSource();
        AudioSource(const std::string &name);
        AudioSource(const AudioSource &) = delete;
        ~AudioSource();

        // Get audio clip
        AudioClip *getClip() const;
        // Set audio clip
        void setClip(AudioClip *clip);
        // Get volume at range 0.0 ... 1.0
        float getVolume() const;
        // Set volume at range 0.0 ... 1.0
        void setVolume(float value);
        // Set Clip position to begin
        bool Rewind();
        // Set play audio source
        bool Play(bool loop = false);
        // Set pause state
        void Pause();
        // Set stoping state
        void Stop();
        // Is played now?
        bool isPlaying() const;
        // Is paused now?
        bool isPaused() const;
    };

} // namespace RoninEngine::Runtime
