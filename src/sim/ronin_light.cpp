#include "ronin.h"
#include "ronin_render_cache.h"
#include <cmath>
#include <algorithm>

namespace RoninEngine::Runtime
{
    static SDL_Texture *get_light_texture(SDL_Surface *surf)
    {
        if(!surf || !currentWorld || !currentWorld->irs)
            return nullptr;
        auto &cache = currentWorld->irs->renderCache;
        auto it = cache.find(surf);
        if(it != cache.end())
            return it->second.second;
        SDL_Texture *tex = render_make_texture(surf);
        if(tex)
        {
            cache.insert({surf, {1, tex}});
            currentWorld->irs->renderCacheRefs.insert({tex, surf});
        }
        return tex;
    }

    static SDL_Surface *create_radial_surface(int size = 128)
    {
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
        if(!surf)
            return nullptr;
        SDL_LockSurface(surf);
        float center = (size - 1) * 0.5f;
        float maxRadius = center;
        for(int y = 0; y < size; ++y)
        {
            Uint32 *row = reinterpret_cast<Uint32 *>(static_cast<Uint8 *>(surf->pixels) + y * surf->pitch);
            for(int x = 0; x < size; ++x)
            {
                float dx = (x - center) / maxRadius;
                float dy = (y - center) / maxRadius;
                float dist = std::sqrt(dx * dx + dy * dy);
                Uint8 a = 0;
                if(dist < 1.0f)
                {
                    float falloff = (1.0f - dist);
                    a = static_cast<Uint8>(std::clamp(falloff * falloff * 255.0f, 0.0f, 255.0f));
                }
                row[x] = SDL_MapRGBA(surf->format, 255, 255, 255, a);
            }
        }
        SDL_UnlockSurface(surf);
        return surf;
    }

    static SDL_Surface *create_direct_surface(int size = 128)
    {
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
        if(!surf)
            return nullptr;
        SDL_LockSurface(surf);
        float center = (size - 1) * 0.5f;
        float maxRadius = center;
        for(int y = 0; y < size; ++y)
        {
            Uint32 *row = reinterpret_cast<Uint32 *>(static_cast<Uint8 *>(surf->pixels) + y * surf->pitch);
            float fy = 1.0f - static_cast<float>(y) / (size - 1);
            for(int x = 0; x < size; ++x)
            {
                float dx = std::abs(x - center) / maxRadius;
                float fx = std::max(0.0f, 1.0f - dx);
                float a_val = fx * fx * fy * 255.0f;
                Uint8 a = static_cast<Uint8>(std::clamp(a_val, 0.0f, 255.0f));
                row[x] = SDL_MapRGBA(surf->format, 255, 255, 255, a);
            }
        }
        SDL_UnlockSurface(surf);
        return surf;
    }

    Light::Light() : Light(DESCRIBE_AS_MAIN_OFF(Light))
    {
    }
    Light::Light(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Light)), range(1.0f), intensity(1.0f), color(Color::white)
    {
        DESCRIBE_AS_MAIN(Light);
    }
    Light::Light(const Light &other) : Component(other.m_name), range(other.range), intensity(other.intensity), color(other.color)
    {
    }

    Light::~Light()
    {
    }

    void Light::get_light()
    {
    }

    void Light::get_light(Rendering *render_info)
    {
    }

    void Light::get_light_source(Rendering *render)
    {
        if(render)
        {
            get_light(render);
        }
        get_light();
    }

    SimpleLight::SimpleLight() : SimpleLight(DESCRIBE_AS_MAIN_OFF(SimpleLight))
    {
    }
    SimpleLight::SimpleLight(const std::string &name) : Light(DESCRIBE_AS_ONLY_NAME(SimpleLight))
    {
        DESCRIBE_AS_MAIN(SimpleLight);
    }
    SimpleLight::SimpleLight(const SimpleLight &other) : Light(other)
    {
    }
    void SimpleLight::get_light()
    {
    }
    void SimpleLight::get_light(Rendering *render_info)
    {
        if(!render_info)
            return;
        static SDL_Surface *surf = create_radial_surface();
        SDL_Texture *tex = get_light_texture(surf);
        if(!tex)
            return;
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
        SDL_SetTextureColorMod(tex, color.r, color.g, color.b);
        SDL_SetTextureAlphaMod(tex, static_cast<Uint8>(std::clamp(intensity, 0.f, 1.f) * color.a));

        render_info->texture = tex;
        render_info->src = {0, 0, surf->w, surf->h};
        float dia = std::max(0.0f, range * 2.0f);
        render_info->dst = {0, 0, dia, dia};
    }

    DirectLight::DirectLight() : DirectLight(DESCRIBE_AS_MAIN_OFF(DirectLight))
    {
    }
    DirectLight::DirectLight(const std::string &name) : Light(DESCRIBE_AS_ONLY_NAME(DirectLight))
    {
        DESCRIBE_AS_MAIN(DirectLight);
    }
    DirectLight::DirectLight(const DirectLight &other) : Light(other)
    {
    }
    void DirectLight::get_light()
    {
    }
    void DirectLight::get_light(Rendering *render_info)
    {
        if(!render_info)
            return;
        static SDL_Surface *surf = create_direct_surface();
        SDL_Texture *tex = get_light_texture(surf);
        if(!tex)
            return;
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
        SDL_SetTextureColorMod(tex, color.r, color.g, color.b);
        SDL_SetTextureAlphaMod(tex, static_cast<Uint8>(std::clamp(intensity, 0.f, 1.f) * color.a));

        render_info->texture = tex;
        render_info->src = {0, 0, surf->w, surf->h};
        float dia = std::max(0.0f, range * 2.0f);
        render_info->dst = {0, 0, dia, dia};
    }
} // namespace RoninEngine::Runtime
