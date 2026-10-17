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
        if(it != cache.end() && it->second.second != nullptr)
            return it->second.second;
        SDL_Texture *tex = render_make_texture(surf);
        if(tex)
        {
            if(it != cache.end())
            {
                it->second.first = std::max(1, it->second.first);
                it->second.second = tex;
            }
            else
            {
                cache.insert({surf, {1, tex}});
            }
            currentWorld->irs->renderCacheRefs[tex] = surf;
        }
        return tex;
    }

    static SDL_Surface *create_spotlight_surface(int size = 128, float coneAngleDeg = 60.0f)
    {
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
        if(!surf)
            return nullptr;
        SDL_LockSurface(surf);
        float cx = (size - 1) * 0.5f;
        float cy = (size - 1) * 0.5f;
        float maxRadius = cx;
        float halfConeRad = std::max(0.001f, ( std::clamp(coneAngleDeg, 1.0f, 360.0f) * 0.5f) * Math::deg2rad);
        for(int y = 0; y < size; ++y)
        {
            Uint32 *row = reinterpret_cast<Uint32 *>(static_cast<Uint8 *>(surf->pixels) + y * surf->pitch);
            for(int x = 0; x < size; ++x)
            {
                float dx = x - cx;
                float dy = cy - y; // upward is positive Y
                float dist = std::hypot(dx, dy) / maxRadius;
                Uint8 a = 0;
                if(dist <= 1.0f && (dx != 0.0f || dy != 0.0f))
                {
                    float pxAngle = std::atan2(std::abs(dx), dy);
                    if(pxAngle <= halfConeRad)
                    {
                        float radFalloff = (1.0f - dist) * (1.0f - dist);
                        float angFalloff = std::cos((pxAngle / halfConeRad) * (Math::Pi * 0.5f));
                        a = static_cast<Uint8>(std::clamp(radFalloff * angFalloff * 255.0f, 0.0f, 255.0f));
                    }
                }
                row[x] = SDL_MapRGBA(surf->format, 255, 255, 255, a);
            }
        }
        SDL_UnlockSurface(surf);
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

    static std::map<int, SDL_Surface *> s_spotlightSurfCache;
    struct SpotlightCacheCleanup
    {
        ~SpotlightCacheCleanup()
        {
            for(auto &pair : s_spotlightSurfCache)
            {
                if(pair.second)
                    SDL_FreeSurface(pair.second);
            }
            s_spotlightSurfCache.clear();
        }
    };
    static SpotlightCacheCleanup s_spotlightCleaner;

    void Spotlight::get_light(Rendering *render_info)
    {
        if(!render_info)
            return;
        int angleKey = std::clamp(static_cast<int>(std::round(angle)), 1, 360);
        SDL_Surface *surf = nullptr;
        auto it = s_spotlightSurfCache.find(angleKey);
        if(it != s_spotlightSurfCache.end())
        {
            surf = it->second;
        }
        else
        {
            surf = create_spotlight_surface(128, static_cast<float>(angleKey));
            if(surf)
                s_spotlightSurfCache[angleKey] = surf;
        }
        if(!surf)
            return;
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
