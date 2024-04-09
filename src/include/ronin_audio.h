/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef _RONIN_AUDIO_H_
#define _RONIN_AUDIO_H_

#include "ronin.h"

namespace RoninEngine::Runtime
{
    class RoninAudio
    {
    public:
        static int Init();
        static void Finalize();
        static bool HasInit();

        static AudioState getChannelState(int channel);
        static int openChannel();
        static bool closeChannel(int channel);
        static bool resumeChannel(int channel);
        static bool setChannelState(int channel, AudioClip *clip, AudioState state, bool loop);
        static float getChannelVolume(int channel);
        static void setChannelVolume(int channel, float value);
        static double getAudioClipDuration(AudioClip *clip);

        static AudioState getMusicState();
        static bool setMusicState(MusicClip *clip, AudioState state, bool loop);
        static float getMusicVolume(MusicClip *clip);
        static void setMusicVolume(float value);
        static double getMusicDuration(MusicClip *clip);
        static bool setMusicPosition(double position);
        static double getMusicPosition(MusicClip *clip);
    };
} // namespace RoninEngine::Runtime

#endif
