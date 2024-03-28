#include "ronin_render_cache.h"

namespace RoninEngine::Runtime
{
    SDL_Texture *render_cache_texture(Sprite *sprite)
    {
        SDL_Texture *texture;
        std::map<Runtime::Sprite *, SDL_Texture *> *_cache;

        if(sprite == nullptr || sprite->surface == nullptr)
            return nullptr;

        for(;;)
        {
            _cache = &switched_world->irs->render_cache;

            const auto iter = _cache->find(sprite);
            if(iter == std::end(*_cache))
            {
                SDL_Rect rect;
                memcpy(&rect, &sprite->m_rect, sizeof(rect));
                texture = SDL_CreateTexture(env.renderer, ronin_default_pixelformat, SDL_TEXTUREACCESS_STREAMING, rect.w, rect.h);

                if(texture == nullptr)
                {
                    RoninSimulator::Log(SDL_GetError());
                    break;
                }

                SDL_Surface *blitDst;
                void *pixels;
                int pitch;

                blitDst = SDL_CreateRGBSurfaceWithFormat(0, rect.w, rect.h, 32, ronin_default_pixelformat);
                if(blitDst == nullptr)
                {
                    RoninSimulator::Log(SDL_GetError());
                    SDL_DestroyTexture(texture);
                    break;
                }

                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                // COPY PIXELS
                SDL_LockTexture(texture, nullptr, &pixels, &pitch);
                SDL_BlitSurface(sprite->surface, &rect, blitDst, nullptr);
                memcpy(pixels, blitDst->pixels, blitDst->pitch * blitDst->h);

                // Apply changes
                SDL_UnlockTexture(texture);
                SDL_FreeSurface(blitDst);

                _cache->insert({sprite, texture});
            }
            else
            {
                texture = iter->second;
            }

            break;
        }
        return texture;
    }

    void render_texture_extension(SDL_Texture *texture, const SDL_Rect *extent, float angleRad)
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

        destTexture = SDL_CreateTexture(env.renderer, ronin_default_pixelformat, SDL_TEXTUREACCESS_TARGET, extent->w, extent->h);
        if(destTexture == nullptr)
            return;

        SDL_SetTextureBlendMode(destTexture, SDL_BLENDMODE_BLEND);

        prevTargetTexture = SDL_GetRenderTarget(env.renderer);
        SDL_SetRenderTarget(env.renderer, destTexture);

        // Set first draw
        dest = src;
        SDL_RenderCopy(env.renderer, texture, nullptr, &dest);
        dest.x += src.w;
        for(; dest.x < extent->w;)
        {
            SDL_RenderCopy(env.renderer, destTexture, &src, &dest);
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
            SDL_RenderCopy(env.renderer, destTexture, &src, &dest);
            dest.y += src.h;
            src.h *= 2;
            dest.h = src.h;
        }

        SDL_SetRenderTarget(env.renderer, prevTargetTexture);
        SDL_RenderCopyEx(env.renderer, destTexture, nullptr, extent, Math::rad2deg * angleRad, nullptr, SDL_FLIP_NONE);
        SDL_DestroyTexture(destTexture);
    }

} // namespace RoninEngine::Runtime
