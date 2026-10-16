#pragma once

#include "Component.h"
#include "Color.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Light : public Component
    {
    protected:
        virtual void get_light();
        virtual void get_light(Rendering *render_info);

    public:
        // Range of the light (world space)
        float range;
        // Intensity of the light [0..1]
        float intensity;
        // Color tint of the light
        Color color;

        Light();
        Light(const std::string &name);
        Light(const Light &other);
        virtual ~Light();
        Light* get_light_component() override { return this; }
        void get_light_source(Rendering *render_info);
    };

    class RONIN_API SimpleLight : public Light
    {
    protected:
        void get_light() override;
        void get_light(Rendering *render_info) override;
    public:
        SimpleLight();
        SimpleLight(const std::string &name);
        SimpleLight(const SimpleLight &other);
    };

    class RONIN_API DirectLight : public Light
    {
    protected:
        void get_light() override;
        void get_light(Rendering *render_info) override;
    public:
        DirectLight();
        DirectLight(const std::string &name);
        DirectLight(const DirectLight &other);
    };
} // namespace RoninEngine::Runtime
