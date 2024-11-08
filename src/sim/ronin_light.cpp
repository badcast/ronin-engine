#include "ronin.h"

namespace RoninEngine::Runtime
{
    Light::Light() : Light(DESCRIBE_AS_MAIN_OFF(Light))
    {
    }
    Light::Light(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Light))
    {
        DESCRIBE_AS_MAIN(Light);
    }

    Light::~Light()
    {
    }

    void Light::get_light_source(Rendering *render)
    {
    }
} // namespace RoninEngine::Runtime
