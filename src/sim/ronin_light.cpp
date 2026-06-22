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
        get_light();
    }

    SimpleLight::SimpleLight() : SimpleLight(DESCRIBE_AS_MAIN_OFF(SimpleLight))
    {
    }
    SimpleLight::SimpleLight(const std::string &name) : Light(DESCRIBE_AS_ONLY_NAME(SimpleLight))
    {
        DESCRIBE_AS_MAIN(SimpleLight);
    }
    void SimpleLight::get_light()
    {
        // Simple light logic
    }

    DirectLight::DirectLight() : DirectLight(DESCRIBE_AS_MAIN_OFF(DirectLight))
    {
    }
    DirectLight::DirectLight(const std::string &name) : Light(DESCRIBE_AS_ONLY_NAME(DirectLight))
    {
        DESCRIBE_AS_MAIN(DirectLight);
    }
    void DirectLight::get_light()
    {
        // Direct light logic
    }
} // namespace RoninEngine::Runtime
