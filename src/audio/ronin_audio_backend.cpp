#include "ronin_audio.h"

namespace RoninEngine::Runtime
{
    static int last_inited_result = 0;

    constexpr int CHUNK_SIZE = 2048;

    int RoninAudio::Init()
    {
        int current_inits = MIX_InitFlags::MIX_INIT_OGG;
        if((last_inited_result = (Mix_Init(current_inits))) < 1)
            return -1;

        if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, CHUNK_SIZE))
        {
            last_inited_result = 0;
            Mix_Quit();
            return -1;
        }

        last_inited_result = 1;

        return 0;
    }

    void RoninAudio::Finalize()
    {
        if(last_inited_result == 0)
            return;
        Mix_CloseAudio();
        Mix_Quit();
    }

    bool RoninAudio::HasInit()
    {
        return last_inited_result > 0;
    }

    AudioState RoninAudio::getChannelState(int channel)
    {
        AudioState state;
        if(HasInit())
        {
            if(Mix_Playing(channel) == 1)
                state = AudioState::Play;
            else if(Mix_Paused(channel) == 1)
                state = AudioState::Pause;
            else
                state = AudioState::Stop;
        }
        else
        {
            state = AudioState::Undefined;
        }
        return state;
    }

    int RoninAudio::pushChannel()
    {
        if(!HasInit())
            return -1;

        WorldResources *__world_resources = World::self()->irs;
        int channel = __world_resources->audio_channels++;

        if(__world_resources->audio_channels > __world_resources->audio_reserved_channels)
            Mix_AllocateChannels(__world_resources->audio_reserved_channels *= 2);

        return channel;
    }

    bool RoninAudio::resumeChannel(int channel)
    {
        bool result;
        if(result = (channel > -1 && HasInit()))
        {
            Mix_Resume(channel);
        }
        return result;
    }

    bool RoninAudio::setChannelState(int channel, AudioClip *clip, AudioState state, bool loop)
    {
        bool result;
        if(result = HasInit())
            switch(state)
            {
                case AudioState::Play:
                    if(clip != nullptr && clip->mix_chunk != nullptr)
                    {
                        if(Mix_Paused(channel))
                            Mix_Resume(channel);
                        else
                            result = (Mix_PlayChannel(channel, clip->mix_chunk, loop ? -1 : 0) != -1);
                    }
                    break;
                case AudioState::Pause:
                    Mix_Pause(channel);
                    break;
                case AudioState::Stop:
                    result = Mix_HaltChannel(channel) == 0;
                    break;
                default:
                    result = false;
                    break;
            }

        return result;
    }

    void RoninAudio::setChannelVolume(int channel, float value)
    {
        // value = Math::clamp01(value); // Mix_Volume automatically clamp volume
        (channel > -1) && HasInit() && Mix_Volume(channel, Math::Map<float, std::uint8_t>(value, 0.f, 1.f, 0, MIX_MAX_VOLUME));
    }

    double RoninAudio::getAudioClipDuration(AudioClip *clip)
    {
        if(clip == nullptr || clip->mix_chunk == nullptr)
            return -1;
        //        double duration = static_cast<double>(clip->mix_chunk->alen) / (MIX_DEFAULT_FORMAT);
        //        return duration;

        // double duration = (static_cast<double>(clip->mix_chunk->alen) / MIX_DEFAULT_FREQUENCY);

        throw RoninEngine::Exception::ronin_implementation_error();

        return -1;
    }

    float RoninAudio::getChannelVolume(int channel)
    {
        float volume;
        Mix_Chunk *ch;
        if(HasInit() && (ch = Mix_GetChunk(channel)))
            volume = Math::Map<std::uint8_t, float>(ch->volume, 0, MIX_MAX_VOLUME, 0.0, 1.f);
        else
            volume = -1;
        return volume;
    }

    AudioState RoninAudio::getMusicState()
    {
        AudioState state;
        if(HasInit())
        {
            if(Mix_PlayingMusic())
                state = AudioState::Play;
            else if(Mix_PausedMusic())
                state = AudioState::Pause;
            else
                state = AudioState::Stop;
        }
        else
        {
            state = AudioState::Undefined;
        }
        return state;
    }

    bool RoninAudio::setMusicState(MusicClip *clip, AudioState state, bool loop)
    {
        bool result;
        if(result = HasInit())
            switch(state)
            {
                case AudioState::Play:
                    if(clip != nullptr && clip->mix_music != nullptr)
                        result = Mix_PlayMusic(clip->mix_music, loop == true ? -1 : 0) == 0;
                    break;
                case AudioState::Pause:
                    Mix_PauseMusic();
                    break;
                case AudioState::Stop:
                    result = Mix_HaltMusic() == 0;
                    break;
                default:
                    result = false;
                    break;
            }
        return result;
    }

    void RoninAudio::setMusicVolume(float value)
    {
        // set volume (clamp)
        HasInit() && Mix_VolumeMusic(Math::Map<float, int>(value, 0.f, 1.f, 0, MIX_MAX_VOLUME));
    }

    float RoninAudio::getMusicVolume(MusicClip *clip)
    {
        float volume;
        if(HasInit() && clip && clip->mix_music)
            volume = Math::Map<int, float>(Mix_GetMusicVolume(clip->mix_music), 0, MIX_MAX_VOLUME, 0.f, 1.f);
        else
            volume = -1;
        return volume;
    }

    double RoninAudio::getMusicDuration(MusicClip *clip)
    {
        if(HasInit() && clip && clip->mix_music)
        {
            return Mix_MusicDuration(clip->mix_music);
        }
        return -1.0;
    }

    double RoninAudio::getMusicPosition(MusicClip *clip)
    {
        if(HasInit() && clip && clip->mix_music)
        {
            return Mix_GetMusicPosition(clip->mix_music);
        }
        return -1.0;
    }

    bool RoninAudio::setMusicPosition(double position)
    {
        return HasInit() && (Mix_SetMusicPosition(position) == 0);
    }
} // namespace RoninEngine::Runtime
