#pragma once

#include "dependency.h"
#include "Renderer.h"
#include "SpriteRenderer.h"

namespace RoninEngine::Runtime {
class Atlas;

class Sprite {
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
    Texture* texture;

    Sprite(const Sprite&) = default;

    const Rect rect();
    void rect(Runtime::Rect rect);

    const Vec2 Center();
    void Center(Vec2 center);

    const bool valid();

    int width();
    int height();

    static Sprite* empty();
};

}  // namespace RoninEngine::Runtime
