#pragma once

#include "Renderer.h"
#include "SpriteRenderer.h"

namespace RoninEngine::Runtime
{
    class Atlas;

    class RONIN_API Sprite : public RoninPointer
    {
    protected:
        Vec2 m_center;
        Rect m_rect;
        Image *surface;

    public:
        std::string name;

        Sprite();
        Sprite(Image *src, const Rect &rect);
        Sprite(const Sprite &) = default;

        void setSurface(Image *surface);
        Image *getImage() const;

        const Rect rect() const;
        void rect(const Rect &rect);

        const Vec2 center() const;
        void center(Vec2 center);

        const bool valid() const;

        int width() const;
        int height() const;

        const Vec2 size() const;

        Rect realityRect(float opaque = 1.f);

        static SpriteRef CreateEmptySprite();
        static SpriteRef CreateBlackSprite();
        static SpriteRef CreateWhiteSprite();
    };

} // namespace RoninEngine::Runtime
