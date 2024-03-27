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
                texture = SDL_CreateTexture(
                    env.renderer,
                    ronin_default_pixelformat,
                    SDL_TEXTUREACCESS_STREAMING,
                    rect.w,
                    rect.h);

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
} // namespace RoninEngine::Runtime
