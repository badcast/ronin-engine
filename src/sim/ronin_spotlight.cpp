#include "ronin.h"
#include "Spotlight.h"
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

    static SDL_Surface *create_spotlight_surface(int size = 128, float coneAngleDeg = 60.0f)
    {
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
        if(!surf)
            return nullptr;
        Uint32 *pixels = static_cast<Uint32 *>(surf->pixels);
        float cx = (size - 1) * 0.5f;
        float cy = (size - 1) * 0.5f;
        float maxRadius = cx;
        float halfConeRad = (coneAngleDeg * 0.5f) * Math::deg2rad;
        for(int y = 0; y < size; ++y)
        {
            for(int x = 0; x < size; ++x)
            {
                float dx = x - cx;
                float dy = cy - y; // upward is positive Y
                float dist = std::hypot(dx, dy) / maxRadius;
                Uint8 a = 0;
                if(dist <= 1.0f && dy > 0.0f)
                {
                    float angle = std::atan2(std::abs(dx), dy);
                    if(angle <= halfConeRad)
                    {
                        float radFalloff = (1.0f - dist) * (1.0f - dist);
                        float angFalloff = std::cos((angle / halfConeRad) * (Math::Pi * 0.5f));
                        a = static_cast<Uint8>(std::clamp(radFalloff * angFalloff * 255.0f, 0.0f, 255.0f));
                    }
                }
                pixels[y * size + x] = SDL_MapRGBA(surf->format, 255, 255, 255, a);
            }
        }
        return surf;
    }

    Spotlight::Spotlight() : Spotlight(DESCRIBE_AS_MAIN_OFF(Spotlight))
    {
    }

    Spotlight::Spotlight(const std::string &name) : Light(DESCRIBE_AS_ONLY_NAME(Spotlight)), angle(60.0f)
    {
        DESCRIBE_AS_MAIN(Spotlight);
        range = 1.f;
        intensity = 1.f;
    }

    Spotlight::Spotlight(const Spotlight &other) : Light(other), angle(other.angle)
    {
    }

    Spotlight::~Spotlight()
    {
    }

    void Spotlight::get_light()
    {
    }

    void Spotlight::get_light(Rendering *render_info)
    {
        if(!render_info)
            return;
        static SDL_Surface *surf = create_spotlight_surface();
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
