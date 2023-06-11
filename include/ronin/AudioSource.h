#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API AudioSource : public Component
    {
    protected:
        struct AudioSourceData* data;

    public:
        AudioSource();
        AudioSource(const std::string& name);
        ~AudioSource();
        // Get audio clip
        AudioClip* clip() const;
        // Set audio clip
        void clip(AudioClip* clip);

        // Get volume at range 0.0 ... 1.0
        float volume() const;
        // Set volume at range 0.0 ... 1.0
        void volume(const float value);
        // Set volume at range 0.0 ... 1.0
        void volume(float&& value);
        // Set Clip position to begin
        void rewind();
        // Set play audio source
        void play();
        // Set pause state
        void pause();
        // Set stoping state
        void stop();
        // Is played now?
        bool is_playing();
        // Is paused now?
        bool is_paused();
    };

} // namespace RoninEngine::Runtime
