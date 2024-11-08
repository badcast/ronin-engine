#include "ronin_render_cache.h"
#include "ronin_debug.h"

namespace RoninEngine::Runtime
{
    SDL_Texture *render_make_texture(SDL_Surface *surface)
    {
        SDL_Texture *texture;

// Use by SDL_CreateTextureFromSurface ...
#if 1
        texture = SDL_CreateTextureFromSurface(gscope.renderer, surface);
        if(texture != nullptr)
            SDL_SetTextureUserData(texture, surface);
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

        if(sprite == nullptr || sprite->surface == nullptr)
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
        int access;
        SDL_Rect dst, src, srcCopy, overrideSrc;
        SDL_Texture *extentTexture;
        SDL_Surface *extentSurface;

        if(texture == nullptr || extent == nullptr || extent->w <= 0 || extent->h <= 0 || (baseSize && (baseSize->x < 1 || baseSize->y < 1)))
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

            SDL_Texture *prevTarget = SDL_GetRenderTarget(gscope.renderer);
            SDL_SetTextureBlendMode(extentTexture, SDL_BLENDMODE_BLEND);
            SDL_SetRenderTarget(gscope.renderer, extentTexture);
            SDL_SetRenderDrawColor(gscope.renderer, 0, 0, 255, 0);
            SDL_RenderClear(gscope.renderer);

            if(baseSize != nullptr)
            {
                src.w = baseSize->x;
                src.h = baseSize->y;
            }

            // First Render
            srcCopy = src;
            SDL_RenderCopy(gscope.renderer, texture, srcFrom, &srcCopy);

            dst = src;
            dst.x += src.w;
            for(; dst.x < extent->w;)
            {
                SDL_RenderCopy(gscope.renderer, extentTexture, &src, &dst);
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
                SDL_RenderCopy(gscope.renderer, extentTexture, &src, &dst);
                dst.y += src.h;
                src.h *= 2;
                dst.h = src.h;
            }

            // Recover previuos target
            SDL_SetRenderTarget(gscope.renderer, prevTarget);
            SDL_RenderCopyEx(gscope.renderer, extentTexture, nullptr, extent, Math::rad2deg * angleRad, nullptr, SDL_FLIP_NONE);

            // SDL_DestroyTexture(texture);
            SDL_DestroyTexture(extentTexture);
#endif
            break;
        }
#if SOFTWARE_SURFACE
        if(access != SDL_TEXTUREACCESS_STATIC)
            SDL_UnlockTexture(texture);

        SDL_FreeSurface(extentSurface);
#endif
    }

    void render_texture_extension2(SDL_Texture *texture, const Vec2Int *baseSize, const SDL_Rect *extent, float angleRad)
    {
        SDL_Rect dest, src;
        SDL_Texture *destTexture;
        SDL_Texture *prevTargetTexture;

        if(texture == nullptr || extent == nullptr || extent->w <= 0 || extent->h <= 0)
        {
            return;
        }

        src.x = 0;
        src.y = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &src.w, &src.h);

        destTexture = SDL_CreateTexture(gscope.renderer, defaultPixelFormat, SDL_TEXTUREACCESS_TARGET, extent->w, extent->h);
        if(destTexture == nullptr)
            return;

        if(baseSize == nullptr)
            baseSize = reinterpret_cast<const Vec2Int *>(&src.w);
        else
        {
            src.w = baseSize->x;
            src.h = baseSize->y;
        }

        SDL_SetTextureBlendMode(destTexture, SDL_BLENDMODE_BLEND);

        prevTargetTexture = SDL_GetRenderTarget(gscope.renderer);
        SDL_SetRenderTarget(gscope.renderer, destTexture);
        SDL_SetRenderDrawColor(gscope.renderer, 0, 0, 0, 0);
        SDL_RenderClear(gscope.renderer);

        // Set first draw
        dest = src;

        SDL_RenderCopy(gscope.renderer, texture, nullptr, &dest);
        dest.x += src.w;
        for(; dest.x < extent->w;)
        {
            SDL_RenderCopy(gscope.renderer, destTexture, &src, &dest);
            dest.x += src.w;
            src.w *= 2;
            dest.w = src.w;
        }

        dest.x = 0;
        dest.y = src.h;
        // Recover width
        dest.w = extent->w;
        for(; dest.y < extent->h;)
        {
            SDL_RenderCopy(gscope.renderer, destTexture, &src, &dest);
            dest.y += src.h;
            src.h *= 2;
            dest.h = src.h;
        }

        SDL_SetRenderTarget(gscope.renderer, prevTargetTexture);
        SDL_RenderCopyEx(gscope.renderer, destTexture, nullptr, extent, Math::rad2deg * angleRad, nullptr, SDL_FLIP_NONE);
        SDL_DestroyTexture(destTexture);
    }

    void render_cache_unref(SDL_Texture *texture)
    {
        // TODO: FIXME - release unused texture
        return; // DO NOT REMOVE

        if(texture == nullptr)
            return;

        auto &_refs = currentWorld->irs->renderCacheRefs;
        auto &_cache = currentWorld->irs->renderCache;
        const auto iter1 = _refs.find(texture);
        if(iter1 != std::end(_refs))
        {
            const auto iter2 = _cache.find(iter1->second);
            if(iter2 != std::end(_cache))
            {
                // Rem ref
                if(--iter2->second.first == 0)
                {

                    SDL_DestroyTexture(texture);
                    _cache.erase(iter2);
                    _refs.erase(iter1);
                }
            }
            else
            {
                _refs.erase(iter1);
            }
        }
    }

} // namespace RoninEngine::Runtime
