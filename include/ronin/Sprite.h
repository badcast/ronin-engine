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
        friend class GC;

    protected:
        Vec2 m_center;
        Rect m_rect;

    public:
        SDL_Surface* source;
        Sprite(const Sprite&) = default;

        const Rect rect();
        void rect(Runtime::Rect rect);

        const Vec2 Center();
        void Center(Vec2 center);

        const bool valid();

        int width();
        int height();

        Rect realityRect(float&& opaque = 1.f);

        static Sprite* spriteEmpty();
        static Sprite* spriteBlack();
        static Sprite* spriteWhite();
    };

} // namespace RoninEngine::Runtime
