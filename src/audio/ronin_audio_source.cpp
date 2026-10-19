#include "ronin.h"
#include "ronin_audio.h"

using namespace RoninEngine::Runtime;

class AudioSourceAutoDestr : public Behaviour
{
public:
    AudioSourceRef _audio;
    void OnUpdate()
    {
        if(Time::frame() % 10 == 0 && !_audio->isPlaying())
        {
            Destroy(gameObject());
        }
    }
};

AudioSource::AudioSource() : AudioSource(DESCRIBE_AS_MAIN_OFF(AudioSource))
{
}

AudioSource::AudioSource(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(AudioSource))
{
    DESCRIBE_AS_MAIN(AudioSource);

    RoninMemory::alloc_self(data);
    data->targetChannel = RoninAudio::openChannel();
}

AudioSource::~AudioSource()
{
    if(data)
    {
        Stop();
        if(data->targetChannel != -1)
        {
            RoninAudio::closeChannel(data->targetChannel);
            data->targetChannel = -1;
        }
        setClip(nullptr);
        RoninMemory::free(data);
        data = nullptr;
    }
}

AudioClip *AudioSource::getClip() const
{
    return data->m_clip;
}

void AudioSource::setClip(AudioClip *clip)
{
    Stop();
#ifndef NDEBUG
    if(data->m_clip)
        data->m_clip->used--;
    if(clip)
        clip->used++;
#endif
    data->m_clip = clip;
}

float AudioSource::getVolume() const
{
    return RoninAudio::getChannelVolume(data->targetChannel);
}

void AudioSource::setVolume(float value)
{
    RoninAudio::setChannelVolume(data->targetChannel, value);
}

float AudioSource::getPitch() const
{
    return RoninAudio::getChannelPitch(data->targetChannel);
}

void AudioSource::setPitch(float value)
{
    RoninAudio::setChannelPitch(data->targetChannel, value);
}

float AudioSource::getPan() const
{
    return RoninAudio::getChannelPan(data->targetChannel);
}

void AudioSource::setPan(float value)
{
    RoninAudio::setChannelPan(data->targetChannel, value);
}

bool AudioSource::getLoop() const
{
    return RoninAudio::isChannelLooping(data->targetChannel);
}

bool AudioSource::setLoop(bool loop)
{
    return RoninAudio::setChannelLooping(data->targetChannel, loop);
}

int AudioSource::getTimeSamples() const
{
    uint64_t frame = 0;
    if(!RoninAudio::getChannelCursor(data->targetChannel, &frame))
        return -1;
    return static_cast<int>(frame);
}

bool AudioSource::setTimeSamples(int frame)
{
    if(frame < 0)
        return false;
    return RoninAudio::setChannelCursor(data->targetChannel, static_cast<uint64_t>(frame));
}

float AudioSource::getTime() const
{
    uint64_t frame = 0;
    if(data->m_clip == nullptr || data->m_clip->getFrequency() <= 0 || !RoninAudio::getChannelCursor(data->targetChannel, &frame))
        return -1.0f;
    return static_cast<float>(static_cast<double>(frame) / data->m_clip->getFrequency());
}

bool AudioSource::setTime(float seconds)
{
    if(seconds < 0.0f || data->m_clip == nullptr || data->m_clip->getFrequency() <= 0)
        return false;
    return RoninAudio::setChannelCursor(data->targetChannel, static_cast<uint64_t>(static_cast<double>(seconds) * data->m_clip->getFrequency()));
}

// Copy `count` frames of one channel ending at the playback cursor (zero padded before the clip start).
static bool read_pcm_block(AudioSourceData *data, int channel, int count, float *out)
{
    std::fill(out, out + count, 0.0f);

    AudioClip *clip = data->m_clip;
    if(clip == nullptr || clip->getData() == nullptr)
        return false;

    const AudioClipData *cd = clip->getData();
    if(cd->format != ma_format_f32 || cd->channels == 0 || channel < 0 || channel >= static_cast<int>(cd->channels))
        return false;

    uint64_t cursor = 0;
    if(!RoninAudio::getChannelCursor(data->targetChannel, &cursor))
        return false;

    const float *frames = reinterpret_cast<const float *>(cd->pcmData.data());
    const int64_t total = static_cast<int64_t>(cd->totalFrames);
    for(int i = 0; i < count; ++i)
    {
        const int64_t idx = static_cast<int64_t>(cursor) - count + i;
        if(idx >= 0 && idx < total)
            out[i] = frames[idx * cd->channels + channel];
    }
    return true;
}

bool AudioSource::GetOutputData(float *samples, int numSamples, int channel)
{
    if(samples == nullptr || numSamples <= 0 || numSamples > 65536)
        return false;

    if(!read_pcm_block(data, channel, numSamples, samples))
        return false;

    const float volume = getVolume();
    for(int i = 0; i < numSamples; ++i)
        samples[i] *= volume;
    return true;
}

bool AudioSource::GetSpectrumData(float *spectrum, int numSamples, int channel, FFTWindow window)
{
    if(spectrum == nullptr || numSamples < 64 || numSamples > 8192 || (numSamples & (numSamples - 1)) != 0)
        return false;

    std::fill(spectrum, spectrum + numSamples, 0.0f);

    std::vector<float> block(static_cast<size_t>(numSamples) * 2);
    if(!read_pcm_block(data, channel, static_cast<int>(block.size()), block.data()))
        return false;

    return ronin_compute_spectrum(block.data(), static_cast<int>(block.size()), window, spectrum);
}

bool AudioSource::Rewind()
{
    bool last_state = isPlaying();
    Stop();
    if(last_state)
        last_state = Play();
    return last_state;
}

bool AudioSource::Play(bool loop)
{
    bool result;
    if(isPaused())
    {
        result = RoninAudio::resumeChannel(data->targetChannel);
    }
    else
    {
        Stop();
        result = RoninAudio::setChannelState(data->targetChannel, data->m_clip, AudioState::Play, loop);
    }
    return result;
}

void AudioSource::Pause()
{
    RoninAudio::setChannelState(data->targetChannel, data->m_clip, AudioState::Pause, false);
}

void AudioSource::Stop()
{
    RoninAudio::setChannelState(data->targetChannel, data->m_clip, AudioState::Stop, false);
}

bool AudioSource::isPlaying() const
{
    return RoninAudio::getChannelState(data->targetChannel) == AudioState::Play;
}

bool AudioSource::isPaused() const
{
    return RoninAudio::getChannelState(data->targetChannel) == AudioState::Pause;
}

float AudioSource::getDuration()
{
    return static_cast<float>(RoninAudio::getAudioClipDuration(this->data->m_clip));
}

AudioSourceRef AudioSource::PlayClipAtPoint(AudioClip *clip, Vec2 position, float volume)
{
    if(clip == nullptr)
        return nullptr;

    AudioSourceRef audio = Primitive::CreateEmptyGameObject(position)->AddComponent<AudioSource>();
    audio->setClip(clip);
    audio->setVolume(volume);
    audio->Play(false);

    audio->gameObject()->AddComponent<AudioSourceAutoDestr>()->_audio = audio;

    return audio;
}
