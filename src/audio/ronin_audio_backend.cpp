#include "ronin_audio.h"

namespace RoninEngine::Runtime
{

    static int lastInitResult = 0;

    constexpr int CHUNK_SIZE = 2048;

    int RoninAudio::Init()
    {
        int currentInits = MIX_InitFlags::MIX_INIT_OGG;
        if((lastInitResult = (Mix_Init(currentInits))) < 1)
            return -1;

        std::vector<const char *> cc;
        int n = SDL_GetNumAudioDevices(0);
        int x;
        for(x = 0; x < n; ++x)
        {
            cc.emplace_back(SDL_GetAudioDeviceName(x,0));
        }
        x = 4;
        if(Mix_OpenAudioDevice(
               MIX_DEFAULT_FREQUENCY,
               MIX_DEFAULT_FORMAT,
               MIX_DEFAULT_CHANNELS,
               CHUNK_SIZE,
               NULL,
               SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE) != 0)
        {
            lastInitResult = 0;
            Mix_Quit();
            return -1;
        }

        lastInitResult = 1;

        return 0;
    }

    void RoninAudio::Finalize()
    {
        if(lastInitResult == 0)
            return;
        Mix_CloseAudio();
        Mix_Quit();
    }

    bool RoninAudio::HasInit()
    {
        return lastInitResult > 0;
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

    int RoninAudio::openChannel()
    {
        if(!HasInit())
            return -1;

        auto IRS = World::self()->irs;

        int channel = -1;
        int lastChannel;

        // if the first state
        if(IRS->audioChannels.empty())
        {
            // Reserve with default channels length
            IRS->audioChannels.resize(Mix_AllocateChannels(-1), false);
        }

        // find is not allocated channel in reserve
        for(lastChannel = 0; lastChannel < IRS->audioChannels.size(); ++lastChannel)
        {
            // is not played channel, then grub it
            if(!IRS->audioChannels[lastChannel])
            {
                // Allocated
                IRS->audioChannels[lastChannel] = true;
                channel = lastChannel;
                break;
            }
        }

        // if channel is not enough, request get new allocates
        if(channel == -1)
        {
            int newReserve = Mix_AllocateChannels(Mix_AllocateChannels(-1) * 2);
            IRS->audioChannels.resize(newReserve, false);
            channel = lastChannel; // it's n-1 ( size is ) +1
        }

        return channel;
    }

    bool RoninAudio::closeChannel(int channel)
    {
        auto &channels = World::self()->irs->audioChannels;
        if(HasInit() && channel < channels.size() && channel > -1)
        {
            // Request Close
            channels[channel] = false;
            return true;
        }

        return false;
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
