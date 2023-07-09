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
    WorldResources* __world_resources = World::self()->internal_resources;

    RoninMemory::alloc_self(data);
    data->m_volume = MIX_MAX_VOLUME;
    data->target_channel = __world_resources->audio_channels++;

    if (data->target_channel + 1 > __world_resources->audio_reserved_channels)
        Mix_AllocateChannels(__world_resources->audio_reserved_channels *= 2);
}

AudioSource::~AudioSource() { RoninMemory::free(data); }

AudioClip* AudioSource::clip() const { return data->m_clip; }

void AudioSource::clip(AudioClip* clip)
{
    stop();
#ifndef NDEBUG
    if (data->m_clip)
        data->m_clip->used--;
    if (clip)
        clip->used++;
#endif
    data->m_clip = clip;
}

float AudioSource::volume() const { return Math::map<std::uint8_t, float>(data->m_volume, 0, MIX_MAX_VOLUME, 0.0, 1.f); }

void AudioSource::volume(float value)
{
    // set volume (clamping)
    Mix_Volume(data->target_channel, data->m_volume = Math::map<float, std::uint8_t>(value, 0.f, 1.f, 0, MIX_MAX_VOLUME));
}

void AudioSource::rewind()
{
    bool last_state = is_playing();
    stop();
    if (last_state)
        play();
}

void AudioSource::play(bool loop)
{
    if (is_paused())
        Mix_Resume(data->target_channel);
    else {
        stop();

        Mix_PlayChannel(data->target_channel, data->m_clip->mix_chunk, loop ? 1 : 0);
    }
}

void AudioSource::pause() { Mix_Pause(data->target_channel); }

void AudioSource::stop() { Mix_HaltChannel(data->target_channel); }

bool AudioSource::is_playing() const { return Mix_Playing(data->target_channel) == 1; }

bool AudioSource::is_paused() const { return Mix_Paused(data->target_channel) == 1; }
