#include "ronin.h"
#include "ronin_audio.h"

using namespace RoninEngine::Runtime;

AudioSource::AudioSource() : AudioSource(DESCRIBE_AS_MAIN_OFF(AudioSource))
{
}

AudioSource::AudioSource(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(AudioSource))
{
    DESCRIBE_AS_MAIN(AudioSource);

    RoninMemory::alloc_self(data);
    data->target_channel = RoninAudio::pushChannel();
}

AudioSource::~AudioSource()
{
    // TODO: Make Free Channel in
    // World::self()->internal_resources->__world_resources->audio_channels--

    setClip(nullptr);
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
    return RoninAudio::getChannelVolume(data->target_channel);
}

void AudioSource::setVolume(float value)
{
    RoninAudio::setChannelVolume(data->target_channel, value);
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
        result = RoninAudio::resumeChannel(data->target_channel);
    }
    else
    {
        Stop();
        result = RoninAudio::setChannelState(data->target_channel, data->m_clip, AudioState::Play, loop);
    }
    return result;
}

void AudioSource::Pause()
{
    RoninAudio::setChannelState(data->target_channel, data->m_clip, AudioState::Pause, false);
}

void AudioSource::Stop()
{
    RoninAudio::setChannelState(data->target_channel, data->m_clip, AudioState::Stop, false);
}

bool AudioSource::isPlaying() const
{
    return RoninAudio::getChannelState(data->target_channel) == AudioState::Play;
}

bool AudioSource::isPaused() const
{
    return RoninAudio::getChannelState(data->target_channel) == AudioState::Pause;
}
