#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        typedef std::uint32_t ResId;
        class RONIN_API Resources
        {
        public:
            static ResId LoadImage(const std::string &path, bool local);

            static Image *LoadImageRight(const std::string &path, bool local);

            static ResId LoadImageFromStream(std::istream &stream, bool local);

            static Image *LoadImageFromStreamRight(std::istream &stream, bool local);

            static ResId LoadAudioClip(const std::string &path, bool local);

            static ResId LoadAudioClipFromStream(std::istream &stream, bool local);

            static ResId LoadMusicClip(const std::string &path, bool local);

            static ResId LoadMusicClipFromStream(std::istream &stream, bool local);

            static Image *GetImageSource(ResId resource);

            static AudioClip *GetAudioClipSource(ResId resource);

            static MusicClip *GetMusicClipSource(ResId resource);
        };

        class RONIN_API Asset
        {
        private:
            struct AssetRef *ref;

        public:
            Sprite *GetSprite(const std::string &name);
            AudioClip *GetAudioClip(const std::string &name);
            Atlas *GetAtlasObject();

            std::vector<Sprite *> GetSprites(const std::string &name);
            std::vector<AudioClip*> GetAudioClips(const std::string &name);
        };

        class RONIN_API AssetManager
        {
        public:
            static bool LoadAsset(const std::string &filename, Asset **asset);
            static Cursor *ConvertImageToCursor(Image *imageSrc, Vec2Int cursorHotspot);
            static void DeleteAsset(Asset *asset);
        };
    } // namespace Runtime
} // namespace RoninEngine
