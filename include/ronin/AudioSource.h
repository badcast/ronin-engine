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

        /**
         * @brief Get the audio clip currently associated with the audio source.
         * @return A pointer to the audio clip.
         */
        AudioClip *getClip() const;

        /**
         * @brief Set the audio clip to be played by the audio source.
         * @param clip A pointer to the audio clip to set.
         */
        void setClip(AudioClip *clip);

        /**
         * @brief Get the volume of the audio source within the range [0.0, 1.0].
         * @return The current volume of the audio source.
         */
        float getVolume() const;

        /**
         * @brief Set the volume of the audio source within the range [0.0, 1.0].
         * @param value The volume value to set.
         */
        void setVolume(float value);

        /**
         * @brief Rewind the audio clip to the beginning.
         * @return True if rewinding was successful, false otherwise.
         */
        bool Rewind();

        /**
         * @brief Start playing the audio clip.
         * @param loop Set to true to loop the audio clip, false to play it once.
         * @return True if playing started successfully, false otherwise.
         */
        bool Play(bool loop = false);

        /**
         * @brief Pause the audio source.
         */
        void Pause();

        /**
         * @brief Stop playing the audio source.
         */
        void Stop();

        /**
         * @brief Check if the audio source is currently playing.
         * @return True if the audio source is playing, false otherwise.
         */
        bool isPlaying() const;

        /**
         * @brief Check if the audio source is currently paused.
         * @return True if the audio source is paused, false otherwise.
         */
        bool isPaused() const;

        /**
         * @brief The duration of the AudioClip
         * @return duration of the AudioClip or -1.0 if error.
         */
        float getDuration();

        /**
         * @brief Play an audio clip at a specified position with an optional volume.
         *
         * This static method creates an AudioSource, associates it with the provided
         * audio clip, sets the volume if specified, and starts playing the audio.
         *
         * @param clip A pointer to the audio clip to be played.
         * @param position The 2D position at which to play the audio.
         * @param volume (Optional) The volume for the audio source, within the range [0.0, 1.0]. Default is 1.0.
         * @return A pointer to the created AudioSource, or nullptr if creation failed.
         */
        static AudioSourceRef PlayClipAtPoint(AudioClip *clip, Vec2 position, float volume = 1.F);
    };

} // namespace RoninEngine::Runtime
