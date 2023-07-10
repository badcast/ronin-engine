#include "ronin.h"

namespace RoninEngine::Runtime
{
    Light::Light()
        : Light(DESCRIBE_AS_MAIN_OFF(Light))
    {
    }
    Light::Light(const std::string& name)
        : Component(DESCRIBE_AS_ONLY_NAME(Light))
    {
        DESCRIBE_AS_MAIN(Light);
        fieldFogTexture = nullptr;
    }

    Light::~Light()
    {
    }

    void Light::get_light_source(Rendering* render)
    {
        auto res = RoninSimulator::get_current_resolution();
        Color c;
        SDL_Texture *target, *lastTarget;
        // TODO: not a work light sources
        return;
        if (!fieldFogTexture)
        {
            // Resources::gc_alloc_texture(&fieldFogTexture, res.width, res.height, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING);
            fieldFogTexture->blendMode(SDL_BlendMode::SDL_BLENDMODE_BLEND);

            c = fieldFogTexture->color();
            c.r = 0;
            c.g = 0;
            c.b = 0;
            c.a = 255;

            fieldFogTexture->color(c);
        }

        lastTarget = SDL_GetRenderTarget(renderer);

        target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET, res.width, res.height);

        SDL_SetRenderTarget(renderer, target);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 55);

        // Get light
        this->get_light(renderer);

        SDL_SetRenderTarget(renderer, lastTarget);
        SDL_RenderCopy(renderer, target, nullptr, nullptr); // copy
        SDL_DestroyTexture(target);
    }
} // namespace RoninEngine::Runtime
