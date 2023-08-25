#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        typedef std::uint32_t resource_id;

        class RONIN_API Resources
        {
        public:
            static resource_id LoadImage(const std::string &path, bool local);

            static resource_id LoadAudioClip(const std::string &path, bool local);

            static resource_id LoadMusicClip(const std::string &path, bool local);

            static native_surface_t *GetImageSource(resource_id resource);

            static AudioClip *GetAudioClipSource(resource_id resource);

            static MusicClip *GetMusicClipSource(resource_id resource);
        };
    } // namespace Runtime
} // namespace RoninEngine
