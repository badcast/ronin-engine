#pragma once

#include "Renderer.h"
#include "SpriteRenderer.h"

namespace RoninEngine::Runtime
{
    class Atlas;

    class RONIN_API Sprite
    {
        friend class Scene;
        friend class Camera2D;
        friend class Renderer;
        friend class SpriteRenderer;
        friend class Atlas;
        friend class Resources;

    protected:
        Vec2 m_center;
        Rect m_rect;

    public:
        SDL_Surface* surface;

        Sprite() = default;
        Sprite(SDL_Surface* src, const Rect& rect);
        Sprite(const Sprite&) = default;

        const Rect rect() const;
        void rect(const Rect& rect);

        const Vec2 center() const;
        void center(Vec2 center);

        const bool valid() const;

        int width() const;
        int height() const;

        Rect realityRect(float&& opaque = 1.f);

        static Sprite* spriteEmpty();
        static Sprite* spriteBlack();
        static Sprite* spriteWhite();
    };

} // namespace RoninEngine::Runtime
