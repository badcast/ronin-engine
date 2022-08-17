#pragma once

#include "dependency.h"

namespace RoninEngine::Runtime {
enum class SpriteRenderPresentTiles : char { Fixed, Place };
enum class SpriteRenderType : char {
    ///Обычное отрисовка спрайта с учетом его параметров.
    Simple,
    ///Отрисовка спрайта с учетом его параметров и черепиция от size
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
