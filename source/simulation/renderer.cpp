#include "ronin.h"
namespace RoninEngine::Runtime
{

    Renderer::Renderer()
        : Renderer(DESCRIBE_TYPE(Renderer, this, &t))
    {
    }

    Renderer::Renderer(const std::string& name)
        : Component(name)
    {
    }
} // namespace RoninEngine::Runtime
