#pragma once

#include "dependency.h"

namespace RoninEngine::Runtime {
enum class SpriteRenderPresentTiles : char
{
    //Fixed type
    Fixed,
    //Placement type
    Place
};
enum class SpriteRenderType : char {
    //Simple drawing tiles with self properties
    Simple,
    //Tile (Tilling) drawing tiles, with self properties tilling
    Tile
};

class SpriteRenderer : public Renderer {
    Texture* texture;
    Sprite* sprite;

   public:
    virtual ~SpriteRenderer();

    SpriteRenderType renderType;
    SpriteRenderPresentTiles renderTilePresent;
    Color color;
    Vec2 size;
    Vec2 flip;
    Vec2 offset;

    SpriteRenderer();
    SpriteRenderer(const std::string& name);
    SpriteRenderer(const SpriteRenderer&);

    Vec2 GetSize();

    void setSprite(Sprite* sprite);
    void setSpriteFromTextureToGC(Texture* texture);
    Sprite* getSprite();

    void offsetFromLocalPosition(Vec2 localPosition);
    void offsetFromWorldPosition(Vec2 worldPosition);

    void Render(Render_info* render_info);
};

}  // namespace RoninEngine::Runtime
