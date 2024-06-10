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
        void get_light_source(Rendering *render_info);
    };
} // namespace RoninEngine::Runtime
