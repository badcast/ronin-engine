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
    private:
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

        Vec2 get_size();
        Vec2 get_offset();
        Rect get_relative_size();

        void set_sprite(Sprite* sprite);
        Sprite* get_sprite();

        void offset_from_local_position(Vec2 localPosition);
        void offset_from_world_position(Vec2 worldPosition);

        void render(Rendering* rendering);
    };

} // namespace RoninEngine::Runtime
