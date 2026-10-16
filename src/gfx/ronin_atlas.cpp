#include "ronin.h"
#include <algorithm>

namespace RoninEngine::Runtime
{
    Atlas::Atlas() : src(nullptr), _sprites {}, _nameCache {}, _cacheDirty(true)
    {
    }

    Atlas::Atlas(Image *source) : src(source), _sprites {}, _nameCache {}, _cacheDirty(true)
    {
    }

    Atlas::~Atlas()
    {
        Clear();
    }

    void Atlas::rebuildCache() const
    {
        _nameCache.clear();
        for(const auto &sprite : _sprites)
        {
            if(sprite && !sprite->name.empty())
            {
                _nameCache[sprite->name] = sprite;
            }
        }
        _cacheDirty = false;
    }

    Image *Atlas::GetSource() const
    {
        return src;
    }

    void Atlas::SetSource(Image *source)
    {
        src = source;
        for(auto &spr : _sprites)
        {
            if(spr && !spr->surface)
            {
                spr->surface = src;
            }
        }
    }

    int Atlas::GetCount() const
    {
        return static_cast<int>(_sprites.size());
    }

    bool Atlas::IsEmpty() const
    {
        return _sprites.empty();
    }

    Vec2Int Atlas::GetSize() const
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
        if(value >= 0 && value < static_cast<int>(_sprites.size()))
        {
            return _sprites[value];
        }
        return nullptr;
    }

    SpriteRef Atlas::GetSpriteFromPoint(const Vec2Int &value)
    {
        for(const auto &sprite : _sprites)
        {
            if(sprite && value.x >= sprite->m_rect.x && value.x < sprite->m_rect.x + sprite->m_rect.w &&
               value.y >= sprite->m_rect.y && value.y < sprite->m_rect.y + sprite->m_rect.h)
            {
                return sprite;
            }
        }
        return nullptr;
    }

    SpriteRef Atlas::GetSpriteFromName(const std::string &value)
    {
        if(_cacheDirty || (_nameCache.empty() && !_sprites.empty()))
        {
            rebuildCache();
        }
        auto it = _nameCache.find(value);
        if(it != _nameCache.end())
            return it->second;
        for(const auto &sprite : _sprites)
        {
            if(sprite && sprite->name == value)
                return sprite;
        }
        return nullptr;
    }

    SpriteRef Atlas::operator[](int index)
    {
        return GetSpriteFromIndex(index);
    }

    SpriteRef Atlas::operator[](const std::string &name)
    {
        return GetSpriteFromName(name);
    }

    std::vector<SpriteRef> Atlas::GetSprites()
    {
        return _sprites;
    }

    const std::vector<SpriteRef> &Atlas::GetSprites() const
    {
        return _sprites;
    }

    void Atlas::AddSprite(SpriteRef sprite)
    {
        if(!sprite)
            return;
        if(!sprite->surface && src)
        {
            sprite->surface = src;
        }
        _sprites.push_back(sprite);
        _cacheDirty = true;
    }

    SpriteRef Atlas::AddSprite(const std::string &name, const Rect &rect)
    {
        SpriteRef spr = Primitive::CreateEmptySprite();
        spr->name = name;
        spr->m_rect = rect;
        spr->surface = src;
        _sprites.push_back(spr);
        _cacheDirty = true;
        return spr;
    }

    bool Atlas::RemoveSprite(int index)
    {
        if(index >= 0 && index < static_cast<int>(_sprites.size()))
        {
            _sprites.erase(_sprites.begin() + index);
            _cacheDirty = true;
            return true;
        }
        return false;
    }

    bool Atlas::RemoveSprite(const std::string &name)
    {
        for(auto it = _sprites.begin(); it != _sprites.end(); ++it)
        {
            if(*it && (*it)->name == name)
            {
                _sprites.erase(it);
                _cacheDirty = true;
                return true;
            }
        }
        return false;
    }

    void Atlas::Clear()
    {
        _sprites.clear();
        _nameCache.clear();
        _cacheDirty = false;
    }

    std::vector<SpriteRef> Atlas::SliceGrid(int tileWidth, int tileHeight, const std::string &namePrefix)
    {
        if(tileWidth <= 0 || tileHeight <= 0)
            return {};

        int totalW = src ? src->w : ((!_sprites.empty() && _sprites.front()) ? _sprites.front()->m_rect.w : 0);
        int totalH = src ? src->h : ((!_sprites.empty() && _sprites.front()) ? _sprites.front()->m_rect.h : 0);

        if(totalW <= 0 || totalH <= 0)
            return {};

        int cols = totalW / tileWidth;
        int rows = totalH / tileHeight;
        return SliceGrid(tileWidth, tileHeight, rows, cols, namePrefix);
    }

    std::vector<SpriteRef> Atlas::SliceGrid(int tileWidth, int tileHeight, int rows, int cols, const std::string &namePrefix)
    {
        std::vector<SpriteRef> newSprites;
        if(tileWidth <= 0 || tileHeight <= 0 || rows <= 0 || cols <= 0)
            return newSprites;

        int index = 0;
        for(int y = 0; y < rows; ++y)
        {
            for(int x = 0; x < cols; ++x)
            {
                Rect r {x * tileWidth, y * tileHeight, tileWidth, tileHeight};
                std::string name = namePrefix.empty() ? ("sprite_" + std::to_string(index)) : (namePrefix + "_" + std::to_string(index));
                SpriteRef spr = AddSprite(name, r);
                newSprites.push_back(spr);
                ++index;
            }
        }
        return newSprites;
    }

} // namespace RoninEngine::Runtime
