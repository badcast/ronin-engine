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
            static resource_id load_surface(const std::string &path, bool local);

            static resource_id load_audio_clip(const std::string &path, bool local);

            static resource_id load_music_clip(const std::string &path, bool local);

            static native_surface_t *get_surface(resource_id resource);

            static AudioClip *get_audio_clip(resource_id resource);

            static MusicClip *get_music_clip(resource_id resource);
        };
    } // namespace Runtime
} // namespace RoninEngine
