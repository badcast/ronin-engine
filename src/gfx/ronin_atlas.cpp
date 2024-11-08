#include "ronin.h"

namespace RoninEngine::Runtime
{
    Atlas::Atlas() : src(nullptr), _sprites {}
    {
    }

    Image *Atlas::GetSource()
    {
        return src;
    }

    int Atlas::GetCount()
    {
        return static_cast<int>(_sprites.size());
    }

    Vec2Int Atlas::GetSize()
    {
        Vec2Int sz;

        if(src != nullptr)
        {
            sz.x = src->w;
            sz.y = src->h;
        }

        return sz;
    }

    SpriteRef Atlas::GetSpriteFromIndex(int value)
    {
        SpriteRef result = nullptr;

        if(value >= 0 && value < GetCount())
        {
            result = _sprites[value];
        }
        else
        {
            result = nullptr;
        }

        return result;
    }

    SpriteRef Atlas::GetSpriteFromPoint(const Vec2Int &value)
    {
        SpriteRef result = nullptr;

        for(const auto &sprite : _sprites)
        {
            if(value.x >= sprite->m_rect.x && value.x <= sprite->m_rect.x + sprite->m_rect.w && value.y >= sprite->m_rect.y && value.y <= sprite->m_rect.y + sprite->m_rect.h)
            {
                result = sprite;
                break;
            }
        }

        return result;
    }

    SpriteRef Atlas::GetSpriteFromName(const std::string &value)
    {
        SpriteRef result = nullptr;

        // TODO: Optimize here (hash_name use)
        for(const auto &sprite : _sprites)
        {
            if(sprite->name == value)
            {
                result = sprite;
                break;
            }
        }

        return result;
    }

    std::vector<SpriteRef> Atlas::GetSprites()
    {
        return _sprites;
    }

} // namespace RoninEngine::Runtime
