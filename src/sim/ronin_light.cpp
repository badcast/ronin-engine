#include "ronin.h"

namespace RoninEngine::Runtime
{
    Light::Light() : Light(DESCRIBE_AS_MAIN_OFF(Light))
    {
    }
    Light::Light(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Light))
    {
        DESCRIBE_AS_MAIN(Light);
        fieldFogTexture = nullptr;
    }

    Light::~Light()
    {
    }

    void Light::get_light_source(Rendering *render)
    {
        Color c;
        SDL_Texture *target, *lastTarget;
        // TODO: not a work light sources
        return;
        if(!fieldFogTexture)
        {
            fieldFogTexture->blendMode(SDL_BlendMode::SDL_BLENDMODE_BLEND);

            c = fieldFogTexture->color();
            c.r = 0;
            c.g = 0;
            c.b = 0;
            c.a = 255;

            fieldFogTexture->color(c);
        }

        lastTarget = SDL_GetRenderTarget(gscope.renderer);

        target = SDL_CreateTexture(
            gscope.renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET,
            gscope.activeResolution.width,
            gscope.activeResolution.height);

        SDL_SetRenderTarget(gscope.renderer, target);
        SDL_SetRenderDrawColor(gscope.renderer, 0, 0, 0, 55);

        // Get light
        this->get_light();

        SDL_SetRenderTarget(gscope.renderer, lastTarget);
        SDL_RenderCopy(gscope.renderer, target, nullptr, nullptr); // copy
        SDL_DestroyTexture(target);
    }
} // namespace RoninEngine::Runtime
