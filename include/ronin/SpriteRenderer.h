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

    enum class SpriteRenderFlip : char
    {
        FlipNone = 0,
        FlipHorizontal = 1,
        FlipVertical = 2
    };

    class RONIN_API SpriteRenderer : public Renderer
    {
    private:
        SpriteRef sprite;
        native_texture_t *save_texture;
        Rect save_src;
        Rectf save_dst;
        Vec2 m_size;
        Color color;
        Color prevColorTexture;
        SpriteRenderType renderType;
        SpriteRenderOut renderOut;
        SpriteRenderPresentMode renderPresentMode;

        void free_render_cache();

    public:
        SpriteRenderFlip flip;

        SpriteRenderer();
        SpriteRenderer(const std::string &name);
        SpriteRenderer(const SpriteRenderer &);
        ~SpriteRenderer();

        const SpriteRenderType getRenderType() const;
        void setRenderType(SpriteRenderType value);

        const SpriteRenderOut getRenderOut() const;
        void setRenderOut(SpriteRenderOut value);

        const SpriteRenderPresentMode getPresentMode() const;
        void setPresentMode(SpriteRenderPresentMode value);

        void setSprite(SpriteRef sprite);
        const SpriteRef getSprite() const;

        const Color getColor() const;
        void setColor(Color value);

        const Vec2 getSize() const;
        void setSize(const Vec2 &value);

        void setRealSize();
        Vec2 get_offset();
        Rect get_relative_size();
    };
} // namespace RoninEngine::Runtime
