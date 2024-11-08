#pragma once
#include "Component.h"

namespace RoninEngine::Runtime
{

    struct Rendering;

    class RONIN_API Renderer : public Component
    {
    private:
        int _class;

    public:
        Renderer();
        Renderer(const std::string &name);
        Renderer(const Renderer &) = delete;

        virtual const Vec2 getSize() const = 0;

        virtual void setSize(const Vec2 &value) = 0;

        virtual Vec2 get_offset() = 0;

        virtual Rect get_relative_size() = 0;

        virtual void free_render_cache() = 0;
    };
} // namespace RoninEngine::Runtime
