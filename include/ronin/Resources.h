#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        typedef std::uint32_t resource_id;
        typedef std::pair<std::string, resource_id> resource_key;
        class RONIN_API Resources
        {
        public:
            static resource_id LoadImage(const std::string &path, bool local);

            static resource_id LoadImageFromStream(std::istream &stream, bool local);

            static resource_id LoadAudioClip(const std::string &path, bool local);

            static resource_id LoadAudioClipFromStream(std::istream &stream, bool local);

            static resource_id LoadMusicClip(const std::string &path, bool local);

            static resource_id LoadMusicClipFromStream(std::istream &stream, bool local);

            static native_surface_t *GetImageSource(resource_id resource);

            static AudioClip *GetAudioClipSource(resource_id resource);

            static MusicClip *GetMusicClipSource(resource_id resource);
        };

        class ResourceConverter
        {
        public:
            static std::vector<resource_key> ReadFromCollection(const std::string &jsonfile, bool allLocal);
        };
    } // namespace Runtime
} // namespace RoninEngine
