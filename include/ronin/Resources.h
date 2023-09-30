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

            static resource_id LoadImageFromStream(std::istream &stream, bool local);

            static resource_id LoadAudioClip(const std::string &path, bool local);

            static resource_id LoadAudioClipFromStream(std::istream &stream, bool local);

            static resource_id LoadMusicClip(const std::string &path, bool local);

            static resource_id LoadMusicClipFromStream(std::istream &stream, bool local);

            static Image *GetImageSource(resource_id resource);

            static AudioClip *GetAudioClipSource(resource_id resource);

            static MusicClip *GetMusicClipSource(resource_id resource);
        };

        class RONIN_API Asset
        {
        private:
            struct AssetRef *__ref;

        public:
            Image *GetImage(const std::string &key);
        };

        class RONIN_API AssetManager
        {
        public:
            static bool LoadAsset(const std::string &assetFile, Asset **asset);
        };
    } // namespace Runtime
} // namespace RoninEngine
