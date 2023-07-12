#pragma once

#include "Renderer.h"
#include "SpriteRenderer.h"

namespace RoninEngine::Runtime
{
    class Atlas;

    class RONIN_API Sprite
    {
    protected:
        Vec2 m_center;
        Rect m_rect;
        native_surface_t *surface;

    public:
        Sprite();
        Sprite(native_surface_t *src, const Rect &rect);
        Sprite(const Sprite &) = default;

        void set_surface(native_surface_t *surface);
        const native_surface_t *get_surface() const;

        const Rect rect() const;
        void rect(const Rect &rect);

        const Vec2 center() const;
        void center(Vec2 center);

        const bool valid() const;

        int width() const;
        int height() const;

        const Vec2 size() const;

        Rect realityRect(float opaque = 1.f);

        static Sprite *spriteEmpty();
        static Sprite *spriteBlack();
        static Sprite *spriteWhite();
    };

} // namespace RoninEngine::Runtime
