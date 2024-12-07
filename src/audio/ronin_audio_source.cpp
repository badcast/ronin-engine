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
    RoninMemory::free(data);
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
