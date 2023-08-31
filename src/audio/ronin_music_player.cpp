#include "ronin.h"
#include "ronin_audio.h"

namespace RoninEngine::Runtime
{
    using namespace RoninEngine::Runtime;

    MusicPlayerData data;

    MusicClip *clip()
    {
        return data.m_clip;
    }

    void MusicPlayer::setClip(MusicClip *clip)
    {
        Stop();
#ifndef NDEBUG
        if(data.m_clip)
            data.m_clip->used--;
        if(clip)
            clip->used++;
#endif
        data.m_clip = clip;
    }

    float MusicPlayer::getVolume()
    {
        return RoninAudio::getMusicVolume(data.m_clip);
    }

    void MusicPlayer::setVolume(float value)
    {
        RoninAudio::setMusicVolume(value);
    }

    void MusicPlayer::Rewind()
    {
        Mix_RewindMusic();
    }

    void MusicPlayer::Play(bool loop)
    {
        Stop();
        RoninAudio::setMusicState(data.m_clip, AudioState::Play, loop);
    }

    void MusicPlayer::Pause()
    {
        RoninAudio::setMusicState(data.m_clip, AudioState::Pause, false);
    }

    void MusicPlayer::Stop()
    {
        RoninAudio::setMusicState(data.m_clip, AudioState::Stop, false);
    }

    bool MusicPlayer::isPlaying()
    {
        return RoninAudio::getMusicState() == AudioState::Play;
    }

    bool MusicPlayer::isPaused()
    {
        return RoninAudio::getMusicState() == AudioState::Pause;
    }

    double MusicPlayer::getDuration()
    {
        return RoninAudio::getMusicDuration(data.m_clip);
    }

    double MusicPlayer::getPosition()
    {
        return RoninAudio::getMusicPosition(data.m_clip);
    }

    AudioState MusicPlayer::getState()
    {
        return RoninAudio::getMusicState();
    }

    void MusicPlayer::setPosition(double value)
    {
        RoninAudio::setMusicPosition(value);
    }
} // namespace RoninEngine::Runtime
