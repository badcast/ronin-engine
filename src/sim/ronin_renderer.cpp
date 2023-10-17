#include "ronin.h"
namespace RoninEngine::Runtime
{

    Renderer::Renderer() : Renderer(DESCRIBE_AS_MAIN_OFF(Renderer))
    {
    }

    Renderer::Renderer(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Renderer))
    {
        DESCRIBE_AS_MAIN(Renderer);
    }
} // namespace RoninEngine::Runtime
