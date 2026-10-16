#pragma once

#include "begin.h"
#include <unordered_map>
#include <vector>

namespace RoninEngine::Runtime
{
    class RONIN_API Atlas : public RoninPointer
    {
    protected:
        Image *src;
        std::vector<SpriteRef> _sprites;
        mutable std::unordered_map<std::string, SpriteRef> _nameCache;
        mutable bool _cacheDirty;

        void rebuildCache() const;

    public:
        Atlas();
        explicit Atlas(Image *source);
        Atlas(const Atlas &) = delete;
        virtual ~Atlas();

        Image *GetSource() const;
        void SetSource(Image *source);

        int GetCount() const;
        Vec2Int GetSize() const;
        bool IsEmpty() const;

        SpriteRef GetSpriteFromIndex(int value);
        SpriteRef GetSpriteFromPoint(const Vec2Int &value);
        SpriteRef GetSpriteFromName(const std::string &value);

        SpriteRef operator[](int index);
        SpriteRef operator[](const std::string &name);

        std::vector<SpriteRef> GetSprites();
        const std::vector<SpriteRef> &GetSprites() const;

        void AddSprite(SpriteRef sprite);
        SpriteRef AddSprite(const std::string &name, const Rect &rect);
        bool RemoveSprite(int index);
        bool RemoveSprite(const std::string &name);
        void Clear();

        std::vector<SpriteRef> SliceGrid(int tileWidth, int tileHeight, const std::string &namePrefix = "tile");
        std::vector<SpriteRef> SliceGrid(int tileWidth, int tileHeight, int rows, int cols, const std::string &namePrefix = "tile");
    };

} // namespace RoninEngine::Runtime
