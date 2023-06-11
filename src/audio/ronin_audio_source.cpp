#include <ronin.h>

using namespace RoninEngine::Runtime;

AudioSource::AudioSource()
    : AudioSource(DESCRIBE_AS_MAIN_OFF(AudioSource))
{
}

AudioSource::AudioSource(const std::string& name)
    : Component(DESCRIBE_AS_ONLY_NAME(AudioSource))
{
    DESCRIBE_AS_MAIN(AudioSource);

    RoninMemory::alloc_self(data);
}

AudioSource::~AudioSource() { RoninMemory::free(data); }

AudioClip* AudioSource::clip() const { return data->m_clip; }

void AudioSource::clip(AudioClip* clip)
{
    stop();
    data->m_clip = clip;
}

float AudioSource::volume() const { return data->m_volume; }

void AudioSource::volume(float value) { data->m_volume = Math::clamp01(value); }

void AudioSource::volume(float&& value) { data->m_volume = Math::clamp01(value); }

void AudioSource::play()
{
    stop();

    Mix_PlayChannel(-1, data->m_clip->mix_chunk, 0);
}

void AudioSource::pause() { throw std::bad_alloc(); }

void AudioSource::stop() { }

bool AudioSource::is_playing() { }
