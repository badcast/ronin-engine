#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Light : public Component
    {
    protected:
        Texture* fieldFogTexture;
        virtual void get_light(SDL_Renderer* renderer) = 0;

    public:
        Light();
        Light(const std::string& name);
        virtual ~Light();
        void get_light_source(Rendering* render_info);
    };
}
