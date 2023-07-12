#include <ronin.h>

namespace RoninEngine::Runtime
{
    using namespace RoninEngine::Runtime;

    MusicPlayerData data;

    MusicClip *clip()
    {
        return data.m_clip;
    }

    void MusicPlayer::clip(MusicClip *clip)
    {
        stop();
#ifndef NDEBUG
        if(data.m_clip)
            data.m_clip->used--;
        if(clip)
            clip->used++;
#endif
        data.m_clip = clip;
    }

    float MusicPlayer::volume()
    {
        return Math::map<int, float>(Mix_GetMusicVolume(data.m_clip->mix_music), 0, MIX_MAX_VOLUME, 0.f, 1.f);
    }

    void MusicPlayer::volume(float value)
    {
        // set volume (clamping)
        Mix_VolumeMusic(Math::map<float, int>(value, 0.f, 1.f, 0, MIX_MAX_VOLUME));
    }

    void MusicPlayer::rewind()
    {
        Mix_RewindMusic();
    }

    void MusicPlayer::play(bool loop)
    {
        stop();
        Mix_PlayMusic(data.m_clip->mix_music, loop == true ? std::numeric_limits<int>::max() : 0);
    }

    void MusicPlayer::pause()
    {
        Mix_PauseMusic();
    }

    void MusicPlayer::stop()
    {
        Mix_HaltMusic();
    }

    bool MusicPlayer::is_playing()
    {
        return Mix_PlayingMusic() == 1;
    }

    bool MusicPlayer::is_paused()
    {
        return Mix_PausedMusic() == 1;
    }

    double MusicPlayer::duration()
    {
        return Mix_MusicDuration(data.m_clip->mix_music);
    }

    double MusicPlayer::get_position()
    {
        return Mix_GetMusicPosition(data.m_clip->mix_music);
    }

    void MusicPlayer::set_position(double value)
    {
        Mix_SetMusicPosition(value);
    }
} // namespace RoninEngine::Runtime
