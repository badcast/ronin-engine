#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    enum class SpriteRenderPresentMode : char
    {
        // Fixed type
        Fixed,
        // Placement type
        Place
    };

    enum class SpriteRenderOut : char
    {
        // Draw on
        Origin,
        // Draw an padding
        Centering
    };

    enum class SpriteRenderType : char
    {
        // Simple drawing tiles with self properties
        Simple,
        // Tile (Tilling) drawing tiles, with self properties tilling
        Tile
    };

    class RONIN_API SpriteRenderer : public Renderer
    {
    private:
        Sprite *sprite;
        native_texture_t *save_texture;
        Rect save_src;
        Rectf save_dst;

    public:
        SpriteRenderType renderType;
        SpriteRenderOut renderOut;
        SpriteRenderPresentMode renderPresentMode;

        Color color;
        Vec2 size;
        Vec2 flip;
        Vec2 offset;

        SpriteRenderer();
        SpriteRenderer(const std::string &name);
        SpriteRenderer(const SpriteRenderer &);
        ~SpriteRenderer();

        Vec2 get_size();
        Vec2 get_offset();
        Rect get_relative_size();

        void set_sprite(Sprite *sprite);
        Sprite *get_sprite();

        void free_render_cache();
        void render(Rendering *rendering);
    };

} // namespace RoninEngine::Runtime
