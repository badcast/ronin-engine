#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    // TODO: Make atlas object (Collection sprite)
    class RONIN_API Atlas : public RoninPointer
    {
    protected:
        Image *src;
        std::vector<SpriteRef> _sprites;

    public:
        Atlas();
        Atlas(const Atlas &) = delete;

        Image *GetSource();

        int GetCount();

        Vec2Int GetSize();

        SpriteRef GetSpriteFromIndex(int value);
        SpriteRef GetSpriteFromPoint(const Vec2Int &value);
        SpriteRef GetSpriteFromName(const std::string &value);

        std::vector<SpriteRef> GetSprites();
    };

} // namespace RoninEngine::Runtime
