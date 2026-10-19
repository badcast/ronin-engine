#pragma once

#include "begin.h"
#include "AudioClip.h"

namespace RoninEngine::Runtime
{
    /**
     * @brief Window functions applied to the PCM block before the FFT in AudioSource::GetSpectrumData.
     */
    enum class FFTWindow : int
    {
        Rectangular,
        Triangle,
        Hamming,
        Hanning,
        Blackman,
        BlackmanHarris
    };

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
         * @brief Get the pitch multiplier of the audio source.
         * @return The pitch (1.0 = original, 2.0 = one octave up).
         */
        float getPitch() const;

        /**
         * @brief Set the pitch multiplier (playback rate) in the range [0.1, 8.0].
         *
         * Changes pitch by resampling, so duration changes with it. A shift of n semitones is 2^(n/12).
         * @param value The pitch multiplier to set.
         */
        void setPitch(float value);

        /**
         * @brief Get the stereo pan in the range [-1.0 (left), 1.0 (right)].
         */
        float getPan() const;

        /**
         * @brief Set the stereo pan in the range [-1.0 (left), 1.0 (right)]. Default is 0.
         */
        void setPan(float value);

        /**
         * @brief Check whether the source is currently playing in loop mode.
         */
        bool getLoop() const;

        /**
         * @brief Change loop mode of the current playback. Takes effect immediately if already playing.
         * @return True if applied to a started sound, false if there is no active sound yet.
         */
        bool setLoop(bool loop);

        /**
         * @brief Get the playback position in seconds (position inside the source clip).
         * @return Position, or -1.0 if there is no active sound.
         */
        float getTime() const;

        /**
         * @brief Seek to a position in seconds. Works after Play() has created the sound.
         * @return True on success.
         */
        bool setTime(float seconds);

        /**
         * @brief Get the playback position in PCM frames, or -1 if there is no active sound.
         */
        int getTimeSamples() const;

        /**
         * @brief Seek to a position in PCM frames. Works after Play() has created the sound.
         * @return True on success.
         */
        bool setTimeSamples(int frame);

        /**
         * @brief Copy the most recent PCM samples (the block ending at the playback position) of one channel.
         *
         * Values are float in [-1, 1], multiplied by the source volume. The block is zero padded before the
         * start of the clip.
         * @param samples Destination buffer with at least numSamples floats.
         * @param numSamples Number of frames to read (1..65536).
         * @param channel Channel of the clip (0 = left/mono).
         * @return True if filled from an active sound, false if nothing is playing (buffer is zero filled).
         */
        bool GetOutputData(float *samples, int numSamples, int channel = 0);

        /**
         * @brief Compute the magnitude spectrum of the block ending at the playback position.
         *
         * Reads 2 * numSamples frames, applies the window, runs an FFT and writes numSamples bins covering
         * 0 Hz .. Nyquist (bin k is k * sampleRate / (2 * numSamples) Hz). A sine of amplitude A yields about A.
         * @param spectrum Destination buffer with at least numSamples floats.
         * @param numSamples Number of bins, a power of two in [64, 8192].
         * @param channel Channel of the clip (0 = left/mono).
         * @param window Window function applied before the FFT.
         * @return True if computed from an active sound, false otherwise (buffer is zero filled).
         */
        bool GetSpectrumData(float *spectrum, int numSamples, int channel = 0, FFTWindow window = FFTWindow::Rectangular);

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
