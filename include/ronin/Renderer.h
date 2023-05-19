#pragma once
#include "Component.h"

namespace RoninEngine::Runtime {

struct Rendering;

class RONIN_API Renderer : public Component {
   public:
    Renderer();
    Renderer(const std::string& name);
    Renderer(const Renderer&) = delete;

    virtual Vec2 get_size() = 0;
    virtual Vec2 get_offset() = 0;

    virtual Rect get_relative_size() = 0;

    virtual void render(Rendering* rendering) = 0;
};
}  // namespace RoninEngine::Runtime
