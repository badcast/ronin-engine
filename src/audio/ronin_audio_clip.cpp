#include "ronin.h"
#include "ronin_audio.h"
#include "miniaudio/miniaudio.h"

namespace RoninEngine::Runtime
{
    AudioClip::AudioClip()
        : m_data(new AudioClipData())
#ifndef NDEBUG
          , used(0)
#endif
          , name("")
          , guid("")
          , assetId(0)
    {
    }

    AudioClip::AudioClip(const std::string &name)
        : m_data(new AudioClipData())
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
        delete m_data;
        m_data = nullptr;
    }

    AudioClipData *AudioClip::getData() const
    {
        return m_data;
    }

    void AudioClip::initDecodedData(const void *pcmFrames, uint64_t frameCount, uint32_t channels, uint32_t sampleRate, int format)
    {
        if (!m_data)
            m_data = new AudioClipData();

        m_data->channels = channels;
        m_data->sampleRate = sampleRate;
        m_data->format = format;
        m_data->totalFrames = frameCount;

        size_t bpf = ma_get_bytes_per_frame(static_cast<ma_format>(format), channels);
        size_t totalBytes = frameCount * bpf;
        m_data->lengthInBytes = static_cast<uint32_t>(totalBytes);
        m_data->duration = (sampleRate > 0) ? (static_cast<double>(frameCount) / sampleRate) : 0.0;

        m_data->pcmData.resize(totalBytes);
        if (pcmFrames && totalBytes > 0)
        {
            std::memcpy(m_data->pcmData.data(), pcmFrames, totalBytes);
        }
    }

    double AudioClip::getDuration() const
    {
        if (!m_data || m_data->pcmData.empty())
            return -1.0;
        return m_data->duration;
    }

    int AudioClip::getChannels() const
    {
        return m_data ? static_cast<int>(m_data->channels) : 0;
    }

    int AudioClip::getFrequency() const
    {
        return m_data ? static_cast<int>(m_data->sampleRate) : 0;
    }

    uint32_t AudioClip::getLengthInBytes() const
    {
        return m_data ? m_data->lengthInBytes : 0;
    }

    uint64_t AudioClip::getTotalFrames() const
    {
        return m_data ? m_data->totalFrames : 0;
    }

    bool AudioClip::valid() const
    {
        return m_data != nullptr && !m_data->pcmData.empty();
    }

    AudioClipRef AudioClip::Create(void *chunk)
    {
        (void)chunk;
        return RoninMemory::alloc<AudioClip>()->GetRef<AudioClip>();
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

    AudioClipRef AudioClip::CreateFromMemory(const void *data, size_t sizeInBytes)
    {
        if (!data || sizeInBytes == 0)
            return nullptr;

        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
        ma_uint64 frameCount = 0;
        void *pFrames = nullptr;
        ma_result res = ma_decode_memory(data, sizeInBytes, &config, &frameCount, &pFrames);
        if (res != MA_SUCCESS || !pFrames)
            return nullptr;

        AudioClip *clip = RoninMemory::alloc<AudioClip>();
        clip->initDecodedData(pFrames, frameCount, config.channels, config.sampleRate, config.format);
        ma_free(pFrames, NULL);
        return clip->GetRef<AudioClip>();
    }

} // namespace RoninEngine::Runtime
