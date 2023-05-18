#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    enum class SpriteRenderPresentMode : char {
        // Fixed type
        Fixed,
        // Placement type
        Place
    };

    enum class SpriteRenderOut : char {
        // Draw on
        Origin,
        // Draw an padding
        Centering
    };

    enum class SpriteRenderType : char {
        // Simple drawing tiles with self properties
        Simple,
        // Tile (Tilling) drawing tiles, with self properties tilling
        Tile
    };

    class RONIN_API SpriteRenderer : public Renderer
    {
        //Texture* textureCache;
        Sprite* sprite;

    public:
        SpriteRenderType renderType;
        SpriteRenderOut renderOut;
        SpriteRenderPresentMode renderPresentMode;

        Color color;
        Vec2 size;
        Vec2 flip;
        Vec2 offset;

        SpriteRenderer();
        SpriteRenderer(const std::string& name);
        SpriteRenderer(const SpriteRenderer&);
        virtual ~SpriteRenderer();

        Vec2 getSize();
        Vec2 getOffset();
        Rect getFactical();

        void setSprite(Sprite* sprite);
        void setSpriteFromTextureToGC(SDL_Surface* src);
        Sprite* getSprite();

        void offsetFromLocalPosition(Vec2 localPosition);
        void offsetFromWorldPosition(Vec2 worldPosition);

        void Render(Render_info* render_info);
    };

} // namespace RoninEngine::Runtime
