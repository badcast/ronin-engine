#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    // TODO: Make atlas object (Collection sprite)
    class RONIN_API Atlas
    {
    protected:
        Image *src;
        std::vector<Sprite *> _sprites;

    public:
        Atlas();
        Atlas(const Atlas &) = delete;

        Image *GetSource();

        int GetCount();

        Vec2Int GetSize();

        Sprite *GetSpriteFromIndex(int value);
        Sprite *GetSpriteFromPoint(const Vec2Int &value);
        Sprite *GetSpriteFromName(const std::string &value);

        std::vector<Sprite*> GetSprites();
    };

} // namespace RoninEngine::Runtime
