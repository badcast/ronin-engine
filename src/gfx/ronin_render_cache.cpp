#include "ronin_render_cache.h"
#include "ronin_debug.h"
#include <algorithm>

namespace RoninEngine::Runtime
{
    SDL_Texture *render_make_texture(SDL_Surface *surface)
    {
        if(gscope.renderer == nullptr || surface == nullptr)
            return nullptr;

        SDL_Texture *texture;

// Use by SDL_CreateTextureFromSurface ...
#if 1
        texture = SDL_CreateTextureFromSurface(gscope.renderer, surface);
        if(texture != nullptr)
        {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureUserData(texture, surface);
        }
        else
        {
            Debug::Log(SDL_GetError());
        }
#else
        SDL_Rect rect;

        for(;;)
        {
            texture = SDL_CreateTexture(gscope.renderer, defaultPixelFormat, SDL_TEXTUREACCESS_STREAMING, surface->clip_rect.w, surface->clip_rect.h);
            if(texture == nullptr)
            {
                Debug::Log(SDL_GetError());
                break;
            }

            SDL_Surface *blitDst;
            void *pixels;
            int pitch;
            std::uint8_t r, g, b, a;

            blitDst = SDL_CreateRGBSurfaceWithFormat(0, surface->clip_rect.w, surface->clip_rect.h, 32, defaultPixelFormat);
            if(blitDst == nullptr)
            {
                Debug::Log(SDL_GetError());
                SDL_DestroyTexture(texture);
                break;
            }

            SDL_GetSurfaceColorMod(surface, &r, &g, &b);
            SDL_GetSurfaceAlphaMod(surface, &a);

            SDL_SetSurfaceColorMod(blitDst, r, g, b);
            SDL_SetSurfaceAlphaMod(blitDst, a);
            SDL_SetTextureColorMod(texture, r, g, b);
            SDL_SetTextureAlphaMod(texture, a);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

            // COPY PIXELS
            SDL_LockTexture(texture, &surface->clip_rect, &pixels, &pitch);
            SDL_BlitSurface(surface, &rect, blitDst, nullptr);
            memcpy(pixels, blitDst->pixels, blitDst->pitch * blitDst->h);

            if(SDL_MUSTLOCK(surface))
                SDL_LockSurface(surface);

            SDL_UpdateTexture(texture, &surface->clip_rect, surface->pixels, surface->pitch);

            if(SDL_MUSTLOCK(surface))
                SDL_UnlockSurface(surface);

            // Apply changes
            SDL_UnlockTexture(texture);
            SDL_FreeSurface(blitDst);
            break;
        }
#endif
        return texture;
    }

    SDL_Texture *render_cache_texture(Sprite *sprite)
    {
        SDL_Texture *texture = nullptr;
        std::map<SDL_Surface *, std::pair<int, SDL_Texture *>> *_cache;

        if(sprite == nullptr || sprite->surface == nullptr || currentWorld == nullptr || currentWorld->irs == nullptr)
            return nullptr;

        for(;;)
        {
            _cache = &currentWorld->irs->renderCache;

            const auto iter = _cache->find(sprite->surface);
            if(iter == std::end(*_cache))
            {
                texture = render_make_texture(sprite->surface);

                if(texture != nullptr)
                {
                    _cache->insert({sprite->surface, {1, texture}});
                    currentWorld->irs->renderCacheRefs.insert({texture, sprite->surface});
                }
            }
            else if(iter->second.second == nullptr)
            {
                texture = render_make_texture(sprite->surface);
                if(texture != nullptr)
                {
                    iter->second.first = std::max(1, iter->second.first + 1);
                    iter->second.second = texture;
                    currentWorld->irs->renderCacheRefs[texture] = sprite->surface;
                }
                else
                {
                    _cache->erase(iter);
                }
            }
            else
            {
                ++iter->second.first; // Add ref
                texture = iter->second.second;
            }

            break;
        }
        return texture;
    }

    void render_texture_extension(SDL_Texture *texture, const Vec2Int *baseSize, const SDL_Rect *srcFrom, const SDL_Rect *extent, float angleRad)
    {
        int access = SDL_TEXTUREACCESS_STATIC;
        SDL_Rect dst, src, srcCopy, overrideSrc;
        SDL_Texture *extentTexture = nullptr;
        SDL_Surface *extentSurface = nullptr;

        if(gscope.renderer == nullptr || texture == nullptr || extent == nullptr || extent->w <= 0 || extent->h <= 0 || (baseSize && (baseSize->x < 1 || baseSize->y < 1)))
        {
            return;
        }

        if(srcFrom == nullptr)
        {
            srcFrom = &overrideSrc;
            overrideSrc.x = 0;
            overrideSrc.y = 0;
            SDL_QueryTexture(texture, nullptr, nullptr, &overrideSrc.w, &overrideSrc.h);
        }

        if(srcFrom->w <= 0 || srcFrom->h <= 0)
        {
            return;
        }

        for(;;)
        {
            src.x = 0;
            src.y = 0;
            src.w = srcFrom->w;
            src.h = srcFrom->h;

#define SOFTWARE_SURFACE 0
#if SOFTWARE_SURFACE
            SDL_Surface *surfSrc = nullptr;
            SDL_Texture *destTexture;

            SDL_QueryTexture(texture, nullptr, &access, nullptr, nullptr);
            switch(access)
            {
                case SDL_TEXTUREACCESS_STREAMING:
                    SDL_LockTextureToSurface(texture, nullptr, &surfSrc);
                    break;
                case SDL_TEXTUREACCESS_STATIC:
                    surfSrc = static_cast<SDL_Surface *>(SDL_GetTextureUserData(texture));
                    break;
                default:;
            }

            if(surfSrc == nullptr)
                break;

            extentSurface = SDL_CreateRGBSurfaceWithFormat(0, extent->w, extent->h, 32, defaultPixelFormat);
            if(extentSurface == nullptr)
                break;

            if(baseSize != nullptr)
            {
                src.w = baseSize->x;
                src.h = baseSize->y;
            }

            srcCopy = src;
            SDL_BlitScaled(surfSrc, srcFrom, extentSurface, &srcCopy);

            dst = src;
            dst.x += src.w;
            for(; dst.x < extent->w;)
            {
                SDL_BlitSurface(extentSurface, &src, extentSurface, &dst);
                dst.x += src.w;
                src.w *= 2;
                dst.w = src.w;
            }

            dst.x = 0;
            dst.y = src.h;
            for(; dst.y < extent->h;)
            {
                // Recover width
                dst.w = extent->w;
                SDL_BlitSurface(extentSurface, &src, extentSurface, &dst);
                dst.y += src.h;
                src.h *= 2;
                dst.h = src.h;
            }

            destTexture = SDL_CreateTextureFromSurface(gscope.renderer, extentSurface);
            if(destTexture == nullptr)
                break;

            SDL_SetTextureBlendMode(destTexture, SDL_BLENDMODE_BLEND);
            SDL_RenderCopyEx(gscope.renderer, destTexture, nullptr, extent, Math::rad2deg * angleRad, nullptr, SDL_FLIP_NONE);
            SDL_DestroyTexture(destTexture);
#else
            extentTexture = SDL_CreateTexture(gscope.renderer, defaultPixelFormat, SDL_TEXTUREACCESS_TARGET, extent->w, extent->h);

            if(extentTexture == nullptr)
                break;

            int tileW = (baseSize != nullptr) ? baseSize->x : srcFrom->w;
            int tileH = (baseSize != nullptr) ? baseSize->y : srcFrom->h;
            if(tileW <= 0 || tileH <= 0)
            {
                SDL_DestroyTexture(extentTexture);
                break;
            }

            SDL_Texture *prevTarget = SDL_GetRenderTarget(gscope.renderer);
            SDL_SetTextureBlendMode(extentTexture, SDL_BLENDMODE_BLEND);
            if(SDL_SetRenderTarget(gscope.renderer, extentTexture) != 0)
            {
                SDL_DestroyTexture(extentTexture);
                SDL_RenderCopyEx(gscope.renderer, texture, srcFrom, extent, Math::rad2deg * angleRad, nullptr, SDL_FLIP_NONE);
                break;
            }

            Uint8 prevR = 0, prevG = 0, prevB = 0, prevA = 0;
            SDL_GetRenderDrawColor(gscope.renderer, &prevR, &prevG, &prevB, &prevA);
            SDL_SetRenderDrawColor(gscope.renderer, 0, 0, 0, 0);
            SDL_RenderClear(gscope.renderer);

            SDL_BlendMode prevTexBlend = SDL_BLENDMODE_BLEND;
            SDL_GetTextureBlendMode(texture, &prevTexBlend);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);

            for(int y = 0; y < extent->h; y += tileH)
            {
                int curH = std::min(tileH, extent->h - y);
                for(int x = 0; x < extent->w; x += tileW)
                {
                    int curW = std::min(tileW, extent->w - x);
                    SDL_Rect s = *srcFrom;
                    if(curW < tileW)
                        s.w = std::max(1, (s.w * curW) / tileW);
                    if(curH < tileH)
                        s.h = std::max(1, (s.h * curH) / tileH);
                    SDL_Rect d = {x, y, curW, curH};
                    SDL_RenderCopy(gscope.renderer, texture, &s, &d);
                }
            }

            SDL_SetTextureBlendMode(texture, prevTexBlend);

            // Recover previous target and draw color
            SDL_SetRenderTarget(gscope.renderer, prevTarget);
            SDL_SetRenderDrawColor(gscope.renderer, prevR, prevG, prevB, prevA);
            SDL_RenderCopyEx(gscope.renderer, extentTexture, nullptr, extent, Math::rad2deg * angleRad, nullptr, SDL_FLIP_NONE);

            SDL_DestroyTexture(extentTexture);
#endif
            break;
        }
#if SOFTWARE_SURFACE
        if(access != SDL_TEXTUREACCESS_STATIC)
            SDL_UnlockTexture(texture);

        if(extentSurface != nullptr)
            SDL_FreeSurface(extentSurface);
#endif
    }

    void render_texture_extension2(SDL_Texture *texture, const Vec2Int *baseSize, const SDL_Rect *extent, float angleRad)
    {
        render_texture_extension(texture, baseSize, nullptr, extent, angleRad);
    }

    void render_cache_unref(SDL_Texture *texture)
    {
        if(texture == nullptr)
            return;

        if(currentWorld == nullptr || currentWorld->irs == nullptr)
        {
            SDL_DestroyTexture(texture);
            return;
        }

        auto &_refs = currentWorld->irs->renderCacheRefs;
        auto &_cache = currentWorld->irs->renderCache;
        const auto iter1 = _refs.find(texture);
        if(iter1 != std::end(_refs))
        {
            const auto iter2 = _cache.find(iter1->second);
            if(iter2 != std::end(_cache))
            {
                // Rem ref
                if(--iter2->second.first <= 0)
                {
                    SDL_DestroyTexture(texture);
                    _cache.erase(iter2);
                    _refs.erase(iter1);
                }
            }
            else
            {
                SDL_DestroyTexture(texture);
                _refs.erase(iter1);
            }
        }
        else
        {
            SDL_DestroyTexture(texture);
        }
    }

    void cache_render_flush_all()
    {
        if(currentWorld == nullptr || currentWorld->irs == nullptr)
            return;

        if(!currentWorld->irs->mainObject.isNull())
        {
            std::list<ComponentRef> comps = currentWorld->GetAllComponents();
            for(ComponentRef &comp : comps)
            {
                if(!comp.isNull())
                {
                    if(Renderer *rend = comp->get_renderer())
                    {
                        rend->free_render_cache();
                    }
                }
            }
        }

        for(auto &pair : currentWorld->irs->renderCache)
        {
            if(pair.second.second != nullptr)
            {
                SDL_DestroyTexture(pair.second.second);
            }
        }
        currentWorld->irs->renderCache.clear();
        currentWorld->irs->renderCacheRefs.clear();
    }

} // namespace RoninEngine::Runtime
