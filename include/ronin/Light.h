#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Light : public Component
    {
    protected:
        virtual void get_light() = 0;

    public:
        Light();
        Light(const std::string &name);
        virtual ~Light();
        Light* get_light_component() override { return this; }
        void get_light_source(Rendering *render_info);
    };

    class RONIN_API SimpleLight : public Light
    {
    protected:
        void get_light() override;
    public:
        SimpleLight();
        SimpleLight(const std::string &name);
    };

    class RONIN_API DirectLight : public Light
    {
    protected:
        void get_light() override;
    public:
        DirectLight();
        DirectLight(const std::string &name);
    };
} // namespace RoninEngine::Runtime
