#include "ronin.h"
#include "ronin_render_cache.h"

namespace RoninEngine::Runtime
{
    ///////////////////////////////////////////////////////////////////////////
    // SPRITE RENDERER
    ///////////////////////////////////////////////////////////////////////////
    void render_sprite_renderer(RenderCommand command, SpriteRenderer *target, Rendering *rendering)
    {
        if(target->sprite == nullptr)
            return;

        switch(command)
        {
            case RenderCommand::PreRender:
            {
                if(target->save_texture == nullptr)
                {
                    std::uint16_t x, y;
                    SDL_Rect dest;

                    switch(target->renderType)
                    {
                        case SpriteRenderType::Simple:

                            target->save_texture = render_cache_texture(target->sprite);
                            rendering->src = target->sprite->m_rect;
                            rendering->dst.w = target->sprite->m_rect.w;
                            rendering->dst.h = target->sprite->m_rect.h;

                            break;
                            target->save_texture = SDL_CreateTextureFromSurface(gscope.renderer, target->sprite->surface);
                            rendering->src.w = target->sprite->width();
                            rendering->src.h = target->sprite->height();
                            rendering->dst.w = target->sprite->width();
                            rendering->dst.h = target->sprite->height();
                            switch(target->renderPresentMode)
                            {
                                    // render as fixed (Resize mode)
                                    // case SpriteRenderPresentMode::Fixed:
                                    //      rendering->dst.w = sprite->width() * abs(target->m_size.x) / pixelsPerPoint;
                                    //      rendering->dst.h = sprite->height() * abs(target->m_size.y) / pixelsPerPoint;
                                    // break;

                                    // render as cut
                                case SpriteRenderPresentMode::Place:
                                    rendering->src.w *= target->m_size.x;
                                    rendering->src.h *= target->m_size.y;
                                    break;
                            }
                            break;
                        case SpriteRenderType::Tile:
                        {
                            // TODO: Use render_texture_extension for tiles

                            rendering->src.w = abs(target->m_size.x) * target->sprite->width();
                            rendering->src.h = abs(target->m_size.y) * target->sprite->width();
                            rendering->dst.w = target->sprite->width() * abs(target->m_size.x) / _world->irs->metricPixelsPerPoint.x;
                            rendering->dst.h = target->sprite->height() * abs(target->m_size.y) / _world->irs->metricPixelsPerPoint.y;

                            // generate tiles
                            target->save_texture = SDL_CreateTexture(
                                // renderer, sdl_default_pixelformat, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET, rendering->src.w,
                                // rendering->src.h);
                                gscope.renderer,
                                defaultPixelFormat,
                                SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET,
                                1024,
                                1024);
                            if(target->save_texture == nullptr)
                                RoninSimulator::ShowMessageFail("Texture create fail");

                            SDL_SetRenderTarget(gscope.renderer, target->save_texture);

                            dest.w = target->sprite->width();
                            dest.h = target->sprite->height();

                            rendering->src.x = rendering->src.w / dest.w;
                            rendering->src.y = rendering->src.h / dest.h;

                            SDL_Texture *temp_texture = SDL_CreateTextureFromSurface(gscope.renderer, target->sprite->surface);

                            // render tile
                            switch(target->renderPresentMode)
                            {
                                case SpriteRenderPresentMode::Fixed:
                                {
                                    for(x = 0; x < rendering->src.x; ++x)
                                    {
                                        for(y = 0; y < rendering->src.y; ++y)
                                        {
                                            dest.x = x * dest.w;
                                            dest.y = y * dest.h;
                                            SDL_RenderCopy(gscope.renderer, temp_texture, (SDL_Rect *) &target->sprite->m_rect, (SDL_Rect *) &dest);
                                        }
                                    }
                                    break;
                                }
                                case SpriteRenderPresentMode::Place:
                                {
                                    for(x = 0; x < rendering->src.x; ++x)
                                    {
                                        for(y = 0; y < rendering->src.y; ++y)
                                        {
                                            dest.x = x * dest.w;
                                            dest.y = y * dest.h;

                                            SDL_RenderCopy(gscope.renderer, temp_texture, (SDL_Rect *) &target->sprite->m_rect, (SDL_Rect *) &dest);
                                        }
                                    }
                                    // place remained
                                    for(x = 0, dest.y = rendering->src.h / dest.h * dest.h, dest.h = target->m_size.y - dest.y; x < rendering->src.x; ++x)
                                    {
                                        dest.x = x * dest.w;
                                        SDL_RenderCopy(gscope.renderer, temp_texture, (SDL_Rect *) &target->sprite->m_rect, (SDL_Rect *) &dest);
                                    }
                                    ++rendering->src.y;
                                    for(y = 0, dest.x = rendering->src.w / dest.w * dest.w, dest.h = target->sprite->height(), dest.w = target->m_size.x - dest.x; y < rendering->src.y; ++y)
                                    {
                                        dest.y = y * dest.h;
                                        SDL_RenderCopy(gscope.renderer, temp_texture, (SDL_Rect *) &target->sprite->m_rect, (SDL_Rect *) &dest);
                                    }

                                    break;
                                }
                            }
                            SDL_DestroyTexture(temp_texture);
                            SDL_SetRenderTarget(gscope.renderer, nullptr);
                            break;
                        }
                    }
                    target->save_src = rendering->src;
                    target->save_dst = rendering->dst;

                    SDL_GetTextureColorMod(target->save_texture, &target->prevColorTexture.r, &target->prevColorTexture.g, &target->prevColorTexture.b);
                    SDL_GetTextureAlphaMod(target->save_texture, &target->prevColorTexture.a);
                }
                else
                {
                    rendering->src = target->save_src;
                    rendering->dst = target->save_dst;
                }

                if(target->save_texture)
                {
                    SDL_SetTextureColorMod(target->save_texture, target->color.r, target->color.g, target->color.b);
                    SDL_SetTextureAlphaMod(target->save_texture, target->color.a);
                }

                rendering->dst.w = (rendering->dst.w * target->m_size.x / _world->irs->metricPixelsPerPoint.x) * ((static_cast<int>(target->flip) & static_cast<int>(SpriteRenderFlip::FlipHorizontal)) ? -1 : 1);
                rendering->dst.h = (rendering->dst.h * target->m_size.y / _world->irs->metricPixelsPerPoint.y) * ((static_cast<int>(target->flip) & static_cast<int>(SpriteRenderFlip::FlipVertical)) ? -1 : 1);

                rendering->texture = target->save_texture;
            }
            break;
            case RenderCommand::PostRender:
                if(target->save_texture == nullptr)
                    break;
                SDL_SetTextureColorMod(target->save_texture, target->prevColorTexture.r, target->prevColorTexture.g, target->prevColorTexture.b);
                SDL_SetTextureAlphaMod(target->save_texture, target->prevColorTexture.a);
                break;
            default:;
        }
    }
} // namespace RoninEngine::Runtime
