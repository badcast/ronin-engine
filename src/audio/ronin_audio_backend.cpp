#include <stdlib.h>
#include <vector>
#include <mutex>
#include <algorithm>

#include "ronin.h"
#include "ronin_std.h"
#include "ronin_audio.h"
#include "miniaudio/miniaudio.h"

namespace RoninEngine::Runtime
{
    struct ChannelSlot
    {
        ma_sound sound;
        ma_audio_buffer_ref bufferRef;
        bool initialized = false;
        AudioClip *currentClip = nullptr;
        float volume = 1.0f;
        float pitch = 1.0f;
        float pan = 0.0f;
    };

    static ma_engine g_engine;
    static bool g_engineInitialized = false;
    static std::vector<ChannelSlot> g_channels;
    static std::mutex g_audioMutex;

    // Music
    static ma_sound g_musicSound;
    static ma_audio_buffer_ref g_musicBufferRef;
    static bool g_musicInitialized = false;
    static MusicClip *g_currentMusicClip = nullptr;
    static float g_musicVolume = 1.0f;

    int RoninAudio::Init()
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (g_engineInitialized)
            return 0;

        ma_engine_config config = ma_engine_config_init();
        ma_result result = ma_engine_init(&config, &g_engine);
        if (result != MA_SUCCESS)
        {
            g_engineInitialized = false;
            return -1;
        }

        g_engineInitialized = true;
        g_channels.resize(16);
        return 0;
    }

    void RoninAudio::Finalize()
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (!g_engineInitialized)
            return;

        if (g_musicInitialized)
        {
            ma_sound_stop(&g_musicSound);
            ma_sound_uninit(&g_musicSound);
            ma_audio_buffer_ref_uninit(&g_musicBufferRef);
            g_musicInitialized = false;
            g_currentMusicClip = nullptr;
        }

        for (auto &slot : g_channels)
        {
            if (slot.initialized)
            {
                ma_sound_stop(&slot.sound);
                ma_sound_uninit(&slot.sound);
                ma_audio_buffer_ref_uninit(&slot.bufferRef);
                slot.initialized = false;
                slot.currentClip = nullptr;
            }
        }
        g_channels.clear();

        ma_engine_uninit(&g_engine);
        g_engineInitialized = false;
    }

    bool RoninAudio::HasInit()
    {
        return g_engineInitialized;
    }

    ma_engine *RoninAudio::GetEngine()
    {
        return g_engineInitialized ? &g_engine : nullptr;
    }

    AudioState RoninAudio::getChannelState(int channel)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (!g_engineInitialized || channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return AudioState::Undefined;

        auto &slot = g_channels[channel];
        if (!slot.initialized)
            return AudioState::Stop;

        if (ma_sound_is_playing(&slot.sound))
            return AudioState::Play;
        
        if (ma_sound_at_end(&slot.sound))
            return AudioState::Stop;

        return AudioState::Pause;
    }

    int RoninAudio::openChannel()
    {
        if (!HasInit())
        {
            if (Init() != 0)
                return -1;
        }

        World *world = World::GetCurrentWorld();
        if (!world || !world->irs)
            return -1;

        std::lock_guard<std::mutex> lock(g_audioMutex);
        auto &channels = world->irs->audioChannels;
        if (channels.empty())
        {
            channels.resize(16, false);
            if (g_channels.size() < 16)
                g_channels.resize(16);
        }

        int channel = -1;
        for (int i = 0; i < static_cast<int>(channels.size()); ++i)
        {
            if (!channels[i])
            {
                channel = i;
                break;
            }
        }

        if (channel == -1)
        {
            channel = static_cast<int>(channels.size());
            channels.resize(channel * 2, false);
            g_channels.resize(channels.size());
        }

        channels[channel] = true;
        return channel;
    }

    bool RoninAudio::closeChannel(int channel)
    {
        World *world = World::GetCurrentWorld();
        if (!world || !world->irs)
            return false;

        std::lock_guard<std::mutex> lock(g_audioMutex);
        auto &channels = world->irs->audioChannels;
        if (channel >= 0 && channel < static_cast<int>(channels.size()))
        {
            if (channel < static_cast<int>(g_channels.size()))
            {
                auto &slot = g_channels[channel];
                if (slot.initialized)
                {
                    ma_sound_stop(&slot.sound);
                    ma_sound_uninit(&slot.sound);
                    ma_audio_buffer_ref_uninit(&slot.bufferRef);
                    slot.initialized = false;
                    slot.currentClip = nullptr;
                }
            }
            channels[channel] = false;
            return true;
        }
        return false;
    }

    bool RoninAudio::resumeChannel(int channel)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (!g_engineInitialized || channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return false;

        auto &slot = g_channels[channel];
        if (slot.initialized && !ma_sound_is_playing(&slot.sound))
        {
            ma_sound_start(&slot.sound);
            return true;
        }
        return false;
    }

    bool RoninAudio::setChannelState(int channel, AudioClip *clip, AudioState state, bool loop)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (!g_engineInitialized || channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return false;

        auto &slot = g_channels[channel];

        switch (state)
        {
        case AudioState::Play:
        {
            if (!clip || !clip->getData() || clip->getData()->pcmData.empty())
                return false;

            AudioClipData *data = clip->getData();

            // If already initialized for this clip and was paused, resume
            if (slot.initialized && slot.currentClip == clip)
            {
                if (ma_sound_at_end(&slot.sound))
                {
                    ma_sound_seek_to_pcm_frame(&slot.sound, 0);
                }
                ma_sound_set_looping(&slot.sound, loop ? MA_TRUE : MA_FALSE);
                ma_sound_start(&slot.sound);
                return true;
            }

            // Otherwise re-initialize sound from data source
            if (slot.initialized)
            {
                ma_sound_stop(&slot.sound);
                ma_sound_uninit(&slot.sound);
                ma_audio_buffer_ref_uninit(&slot.bufferRef);
                slot.initialized = false;
                slot.currentClip = nullptr;
            }

            ma_result res = ma_audio_buffer_ref_init(
                static_cast<ma_format>(data->format),
                data->channels,
                data->pcmData.data(),
                data->totalFrames,
                &slot.bufferRef);
            if (res != MA_SUCCESS)
                return false;

            res = ma_sound_init_from_data_source(
                &g_engine,
                &slot.bufferRef,
                0,
                NULL,
                &slot.sound);
            if (res != MA_SUCCESS)
            {
                ma_audio_buffer_ref_uninit(&slot.bufferRef);
                return false;
            }

            slot.initialized = true;
            slot.currentClip = clip;
            ma_sound_set_volume(&slot.sound, slot.volume);
            ma_sound_set_pitch(&slot.sound, slot.pitch);
            ma_sound_set_pan(&slot.sound, slot.pan);
            ma_sound_set_looping(&slot.sound, loop ? MA_TRUE : MA_FALSE);
            ma_sound_start(&slot.sound);
            return true;
        }
        case AudioState::Pause:
        {
            if (slot.initialized)
            {
                ma_sound_stop(&slot.sound);
                return true;
            }
            return false;
        }
        case AudioState::Stop:
        {
            if (slot.initialized)
            {
                ma_sound_stop(&slot.sound);
                ma_sound_seek_to_pcm_frame(&slot.sound, 0);
                return true;
            }
            return false;
        }
        default:
            return false;
        }
    }

    void RoninAudio::setChannelVolume(int channel, float value)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return;

        auto &slot = g_channels[channel];
        slot.volume = std::clamp(value, 0.0f, 1.0f);
        if (slot.initialized)
        {
            ma_sound_set_volume(&slot.sound, slot.volume);
        }
    }

    float RoninAudio::getChannelVolume(int channel)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return -1.0f;

        return g_channels[channel].volume;
    }

    void RoninAudio::setChannelPitch(int channel, float value)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return;

        auto &slot = g_channels[channel];
        slot.pitch = std::clamp(value, 0.1f, 8.0f);
        if (slot.initialized)
        {
            ma_sound_set_pitch(&slot.sound, slot.pitch);
        }
    }

    float RoninAudio::getChannelPitch(int channel)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return -1.0f;

        return g_channels[channel].pitch;
    }

    void RoninAudio::setChannelPan(int channel, float value)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return;

        auto &slot = g_channels[channel];
        slot.pan = std::clamp(value, -1.0f, 1.0f);
        if (slot.initialized)
        {
            ma_sound_set_pan(&slot.sound, slot.pan);
        }
    }

    float RoninAudio::getChannelPan(int channel)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()))
            return 0.0f;

        return g_channels[channel].pan;
    }

    bool RoninAudio::setChannelLooping(int channel, bool loop)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()) || !g_channels[channel].initialized)
            return false;

        ma_sound_set_looping(&g_channels[channel].sound, loop ? MA_TRUE : MA_FALSE);
        return true;
    }

    bool RoninAudio::isChannelLooping(int channel)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()) || !g_channels[channel].initialized)
            return false;

        return ma_sound_is_looping(&g_channels[channel].sound) == MA_TRUE;
    }

    bool RoninAudio::getChannelCursor(int channel, uint64_t *frame)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (!frame || channel < 0 || channel >= static_cast<int>(g_channels.size()) || !g_channels[channel].initialized)
            return false;

        ma_uint64 cursor = 0;
        if (ma_sound_get_cursor_in_pcm_frames(&g_channels[channel].sound, &cursor) != MA_SUCCESS)
            return false;

        *frame = cursor;
        return true;
    }

    bool RoninAudio::setChannelCursor(int channel, uint64_t frame)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (channel < 0 || channel >= static_cast<int>(g_channels.size()) || !g_channels[channel].initialized)
            return false;

        return ma_sound_seek_to_pcm_frame(&g_channels[channel].sound, frame) == MA_SUCCESS;
    }

    double RoninAudio::getAudioClipDuration(AudioClip *clip)
    {
        if (!clip)
            return -1.0;
        return clip->getDuration();
    }

    AudioState RoninAudio::getMusicState()
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (!g_engineInitialized || !g_musicInitialized)
            return AudioState::Stop;

        if (ma_sound_is_playing(&g_musicSound))
            return AudioState::Play;

        if (ma_sound_at_end(&g_musicSound))
            return AudioState::Stop;

        return AudioState::Pause;
    }

    bool RoninAudio::setMusicState(MusicClip *clip, AudioState state, bool loop)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (!g_engineInitialized)
            return false;

        switch (state)
        {
        case AudioState::Play:
        {
            if (!clip || clip->pcmData.empty())
                return false;

            if (g_musicInitialized && g_currentMusicClip == clip)
            {
                if (ma_sound_at_end(&g_musicSound))
                    ma_sound_seek_to_pcm_frame(&g_musicSound, 0);
                ma_sound_set_looping(&g_musicSound, loop ? MA_TRUE : MA_FALSE);
                ma_sound_start(&g_musicSound);
                return true;
            }

            if (g_musicInitialized)
            {
                ma_sound_stop(&g_musicSound);
                ma_sound_uninit(&g_musicSound);
                ma_audio_buffer_ref_uninit(&g_musicBufferRef);
                g_musicInitialized = false;
                g_currentMusicClip = nullptr;
            }

            ma_result res = ma_audio_buffer_ref_init(
                static_cast<ma_format>(clip->format),
                clip->channels,
                clip->pcmData.data(),
                clip->totalFrames,
                &g_musicBufferRef);
            if (res != MA_SUCCESS)
                return false;

            res = ma_sound_init_from_data_source(&g_engine, &g_musicBufferRef, 0, NULL, &g_musicSound);
            if (res != MA_SUCCESS)
            {
                ma_audio_buffer_ref_uninit(&g_musicBufferRef);
                return false;
            }

            g_musicInitialized = true;
            g_currentMusicClip = clip;
            ma_sound_set_volume(&g_musicSound, g_musicVolume);
            ma_sound_set_looping(&g_musicSound, loop ? MA_TRUE : MA_FALSE);
            ma_sound_start(&g_musicSound);
            return true;
        }
        case AudioState::Pause:
        {
            if (g_musicInitialized)
            {
                ma_sound_stop(&g_musicSound);
                return true;
            }
            return false;
        }
        case AudioState::Stop:
        {
            if (g_musicInitialized)
            {
                ma_sound_stop(&g_musicSound);
                ma_sound_seek_to_pcm_frame(&g_musicSound, 0);
                return true;
            }
            return false;
        }
        default:
            return false;
        }
    }

    void RoninAudio::setMusicVolume(float value)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        g_musicVolume = std::clamp(value, 0.0f, 1.0f);
        if (g_musicInitialized)
        {
            ma_sound_set_volume(&g_musicSound, g_musicVolume);
        }
    }

    float RoninAudio::getMusicVolume(MusicClip *clip)
    {
        (void)clip;
        std::lock_guard<std::mutex> lock(g_audioMutex);
        return g_musicVolume;
    }

    double RoninAudio::getMusicDuration(MusicClip *clip)
    {
        if (clip)
            return clip->duration;
        return -1.0;
    }

    double RoninAudio::getMusicPosition(MusicClip *clip)
    {
        (void)clip;
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (g_musicInitialized)
        {
            float cursorSeconds = 0.0f;
            if (ma_sound_get_cursor_in_seconds(&g_musicSound, &cursorSeconds) == MA_SUCCESS)
                return static_cast<double>(cursorSeconds);
        }
        return -1.0;
    }

    bool RoninAudio::setMusicPosition(double position)
    {
        std::lock_guard<std::mutex> lock(g_audioMutex);
        if (g_musicInitialized && g_currentMusicClip && g_currentMusicClip->sampleRate > 0)
        {
            ma_uint64 frameIndex = static_cast<ma_uint64>(position * g_currentMusicClip->sampleRate);
            return ma_sound_seek_to_pcm_frame(&g_musicSound, frameIndex) == MA_SUCCESS;
        }
        return false;
    }

} // namespace RoninEngine::Runtime
