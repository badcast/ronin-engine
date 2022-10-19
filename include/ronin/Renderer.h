#pragma once

#include "dependency.h"
#include "Component.h"

namespace RoninEngine::Runtime {
struct Render_info {
    SDL_Renderer* renderer;
    Rect src;
    Rectf_t dst;
    Texture* texture;
};

class Renderer : public Component {
   public:
    Renderer();
    Renderer(const std::string& name);
    Renderer(const Renderer&) = delete;

    virtual Vec2 getSize() = 0;
    virtual Vec2 getOffset() = 0;

    virtual void Render(Render_info* render_info) = 0;
};
}  // namespace RoninEngine::Runtime
