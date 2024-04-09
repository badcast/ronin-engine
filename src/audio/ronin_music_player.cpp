#include "ronin.h"
#include "ronin_audio.h"

namespace RoninEngine::Runtime
{
    using namespace RoninEngine::Runtime;

    MusicClip *clip()
    {
        return gscope.musicData.m_clip;
    }

    void MusicPlayer::setClip(MusicClip *clip)
    {
        Stop();
#ifndef NDEBUG
        if(gscope.musicData.m_clip)
            gscope.musicData.m_clip->used--;
        if(clip)
            clip->used++;
#endif
        gscope.musicData.m_clip = clip;
    }

    float MusicPlayer::getVolume()
    {
        return RoninAudio::getMusicVolume(gscope.musicData.m_clip);
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
        RoninAudio::setMusicState(gscope.musicData.m_clip, AudioState::Play, loop);
    }

    void MusicPlayer::Pause()
    {
        RoninAudio::setMusicState(gscope.musicData.m_clip, AudioState::Pause, false);
    }

    void MusicPlayer::Stop()
    {
        RoninAudio::setMusicState(gscope.musicData.m_clip, AudioState::Stop, false);
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
        return RoninAudio::getMusicDuration(gscope.musicData.m_clip);
    }

    double MusicPlayer::getPosition()
    {
        return RoninAudio::getMusicPosition(gscope.musicData.m_clip);
    }

    AudioState MusicPlayer::getState()
    {
        return RoninAudio::getMusicState();
    }

    bool MusicPlayer::isLoop()
    {
        return gscope.musicData.loops == 1;
    }

    void MusicPlayer::setPosition(double value)
    {
        RoninAudio::setMusicPosition(value);
    }
} // namespace RoninEngine::Runtime
