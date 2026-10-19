#include "ronin.h"
#include "ronin_audio.h"
#include <SDL2/SDL_mixer.h>

namespace RoninEngine::Runtime
{
    AudioClip::AudioClip()
        : mix_chunk(nullptr)
#ifndef NDEBUG
          , used(0)
#endif
          , name("")
          , guid("")
          , assetId(0)
    {
    }

    AudioClip::AudioClip(Mix_Chunk *chunk)
        : mix_chunk(chunk)
#ifndef NDEBUG
          , used(0)
#endif
          , name("")
          , guid("")
          , assetId(0)
    {
    }

    AudioClip::AudioClip(const std::string &name, Mix_Chunk *chunk)
        : mix_chunk(chunk)
#ifndef NDEBUG
          , used(0)
#endif
          , name(name)
          , guid("")
          , assetId(0)
    {
    }

    AudioClip::~AudioClip()
    {
    }

    Mix_Chunk *AudioClip::getMixChunk() const
    {
        return mix_chunk;
    }

    void AudioClip::setMixChunk(Mix_Chunk *chunk)
    {
        mix_chunk = chunk;
    }

    double AudioClip::getDuration() const
    {
        if (mix_chunk == nullptr)
            return -1.0;

        int frequency = 0, channels = 0;
        Uint16 format = 0;
        if (Mix_QuerySpec(&frequency, &format, &channels) != 0)
        {
            int bytes_per_sample = (format & 0xFF) / 8;
            if (frequency > 0 && channels > 0 && bytes_per_sample > 0)
            {
                return static_cast<double>(mix_chunk->alen) / (frequency * channels * bytes_per_sample);
            }
        }
        return -1.0;
    }

    int AudioClip::getChannels() const
    {
        int frequency = 0, channels = 0;
        Uint16 format = 0;
        if (Mix_QuerySpec(&frequency, &format, &channels) != 0)
        {
            return channels;
        }
        return 0;
    }

    int AudioClip::getFrequency() const
    {
        int frequency = 0, channels = 0;
        Uint16 format = 0;
        if (Mix_QuerySpec(&frequency, &format, &channels) != 0)
        {
            return frequency;
        }
        return 0;
    }

    uint32_t AudioClip::getLengthInBytes() const
    {
        return mix_chunk ? mix_chunk->alen : 0;
    }

    bool AudioClip::valid() const
    {
        return mix_chunk != nullptr;
    }

    AudioClipRef AudioClip::Create(Mix_Chunk *chunk)
    {
        if (!chunk)
            return nullptr;
        return RoninMemory::alloc<AudioClip>(chunk)->GetRef<AudioClip>();
    }

    AudioClipRef AudioClip::CreateFromFile(const std::string &path)
    {
        ResId id = Resources::LoadAudioClip(path, false);
        if (id == RES_INVALID)
            return nullptr;
        AudioClip *clip = Resources::GetAudioClipSource(id);
        if (!clip)
            return nullptr;
        return clip->GetRef<AudioClip>();
    }

    AudioClipRef AudioClip::CreateFromStream(std::istream &stream)
    {
        ResId id = Resources::LoadAudioClipFromStream(stream, false);
        if (id == RES_INVALID)
            return nullptr;
        AudioClip *clip = Resources::GetAudioClipSource(id);
        if (!clip)
            return nullptr;
        return clip->GetRef<AudioClip>();
    }

} // namespace RoninEngine::Runtime
