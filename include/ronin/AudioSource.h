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
        void volume(float value);
        // Set volume at range 0.0 ... 1.0
        void volume(float&& value);
        // Set play audio source
        void play();
        // Set pause state
        void pause();
        // Set stoping state
        void stop();
        // Is played now?
        bool is_playing();
    };

} // namespace RoninEngine::Runtime
