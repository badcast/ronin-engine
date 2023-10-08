#include "ronin.h"

namespace RoninEngine::Runtime
{
    // FIXME: OPTIMIZING SPRITE RENDERER COMPONENT

    SpriteRenderer::SpriteRenderer() : SpriteRenderer(DESCRIBE_AS_MAIN_OFF(SpriteRenderer))
    {
    }

    SpriteRenderer::SpriteRenderer(const std::string &name)
        : Renderer(DESCRIBE_AS_ONLY_NAME(SpriteRenderer)),
          sprite(nullptr),
          save_texture(nullptr),
          flip(Vec2::one),
          renderType(SpriteRenderType::Simple),
          renderOut(SpriteRenderOut::Centering),
          renderPresentMode(SpriteRenderPresentMode::Fixed),
          color(Color::white)
    {
        DESCRIBE_AS_MAIN(SpriteRenderer);
    }

    SpriteRenderer::SpriteRenderer(const SpriteRenderer &proto)
        : Renderer(proto.m_name),
          sprite(proto.sprite),
          save_texture(nullptr),
          m_size(proto.m_size),
          renderType(proto.renderType),
          renderOut(proto.renderOut),
          renderPresentMode(proto.renderPresentMode),
          color(proto.color),
          flip(proto.flip),
          offset(proto.offset)
    {
    }

    SpriteRenderer::~SpriteRenderer()
    {
        free_render_cache();
    }

    const SpriteRenderType SpriteRenderer::getRenderType() const
    {
        return renderType;
    }

    void SpriteRenderer::setRenderType(SpriteRenderType value)
    {
        free_render_cache();
        renderType = value;
    }

    const SpriteRenderOut SpriteRenderer::getRenderOut() const
    {
        return renderOut;
    }

    void SpriteRenderer::setRenderOut(SpriteRenderOut value)
    {
        free_render_cache();
        renderOut = value;
    }

    const SpriteRenderPresentMode SpriteRenderer::getPresentMode() const
    {
        return renderPresentMode;
    }

    void SpriteRenderer::setPresentMode(SpriteRenderPresentMode value)
    {
        free_render_cache();
        renderPresentMode = value;
    }

    const Color SpriteRenderer::getColor() const
    {
        return color;
    }

    void SpriteRenderer::setColor(Color value)
    {
        free_render_cache();
        color = value;
    }

    const Vec2 SpriteRenderer::getSize() const
    {
        return this->m_size;
    }

    void SpriteRenderer::setSize(const Vec2 &newSize)
    {
        free_render_cache();
        m_size = newSize;
    }

    void SpriteRenderer::setRealSize()
    {
        if(sprite)
            setSize(sprite->size());
    }

    Vec2 SpriteRenderer::get_offset()
    {
        Vec2 outOffset;
        switch(renderOut)
        {
            case SpriteRenderOut::Origin:
                outOffset = -m_size + m_size / 2 + Vec2::half;

                break;
        }
        return outOffset;
    }

    Rect SpriteRenderer::get_relative_size()
    {
        Rect rect;
        if(sprite)
        {
            rect = sprite->realityRect();
        }
        return rect;
    }

    void SpriteRenderer::setSprite(Sprite *sprite)
    {
        if(this->sprite == nullptr && (!this->m_size.x || !this->m_size.y))
        {
            this->m_size = Vec2::one;
        }
        this->sprite = sprite;

        // cache free from last sprite
        free_render_cache();
    }

    const Sprite *SpriteRenderer::getSprite() const
    {
        return this->sprite;
    }

    void SpriteRenderer::free_render_cache()
    {
        if(save_texture)
        {
            SDL_DestroyTexture(save_texture);
            save_texture = nullptr;
        }
    }

    void SpriteRenderer::render(Rendering *rendering)
    {
        if(sprite == nullptr)
            return;

        std::uint16_t x, y;
        SDL_Rect dest;

        if(save_texture == nullptr)
        {
            switch(this->renderType)
            {
                case SpriteRenderType::Simple:
                    save_texture = SDL_CreateTextureFromSurface(renderer, sprite->surface);
                    rendering->src.w = sprite->width();
                    rendering->src.h = sprite->height();
                    rendering->dst.w = sprite->width() * abs(this->m_size.x) / pixelsPerPoint;
                    rendering->dst.h = sprite->height() * abs(this->m_size.y) / pixelsPerPoint;
                    switch(renderPresentMode)
                    {
                            // render as fixed (Resize mode)
                        case SpriteRenderPresentMode::Fixed:
                            //                        rendering->dst.w = sprite->width() * abs(this->size.x) /
                            //                        pixelsPerPoint; rendering->dst.h = sprite->height() *
                            //                        abs(this->size.y) / pixelsPerPoint;
                            break;
                            // render as cut
                        case SpriteRenderPresentMode::Place:
                            rendering->src.w *= Math::Abs(this->m_size.x);
                            rendering->src.h *= Math::Abs(this->m_size.y);
                            break;
                    }
                    break;
                case SpriteRenderType::Tile:
                {
                    rendering->src.w = abs(this->m_size.x) * sprite->width();
                    rendering->src.h = abs(this->m_size.y) * sprite->width();
                    rendering->dst.w = sprite->width() * abs(this->m_size.x) / pixelsPerPoint;
                    rendering->dst.h = sprite->height() * abs(this->m_size.y) / pixelsPerPoint;

                    // if (this->tileRenderPresent == SpriteRenderTile::Fixed) {
                    //     rendering->src.w = (rendering->src.w / sprite->width()) * sprite->width();
                    //     rendering->src.h = (rendering->src.h / sprite->height()) * sprite->height();
                    // }

                    // generate tiles
                    save_texture = SDL_CreateTexture(
                        // renderer, sdl_default_pixelformat, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET, rendering->src.w,
                        // rendering->src.h);
                        renderer,
                        sdl_default_pixelformat,
                        SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET,
                        1024,
                        1024);
                    if(save_texture == nullptr)
                        RoninSimulator::ShowMessageFail("Texture create fail");

                    SDL_SetRenderTarget(renderer, save_texture);

                    dest.w = sprite->width();
                    dest.h = sprite->height();

                    rendering->src.x = rendering->src.w / dest.w;
                    rendering->src.y = rendering->src.h / dest.h;

                    SDL_Texture *temp_texture = SDL_CreateTextureFromSurface(renderer, sprite->surface);

                    // render tile
                    switch(this->renderPresentMode)
                    {
                        case SpriteRenderPresentMode::Fixed:
                        {
                            for(x = 0; x < rendering->src.x; ++x)
                            {
                                for(y = 0; y < rendering->src.y; ++y)
                                {
                                    dest.x = x * dest.w;
                                    dest.y = y * dest.h;
                                    SDL_RenderCopy(renderer, temp_texture, (SDL_Rect *) &sprite->m_rect, (SDL_Rect *) &dest);
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

                                    SDL_RenderCopy(renderer, temp_texture, (SDL_Rect *) &sprite->m_rect, (SDL_Rect *) &dest);
                                }
                            }
                            // place remained
                            for(x = 0, dest.y = rendering->src.h / dest.h * dest.h, dest.h = m_size.y - dest.y; x < rendering->src.x; ++x)
                            {
                                dest.x = x * dest.w;
                                SDL_RenderCopy(renderer, temp_texture, (SDL_Rect *) &sprite->m_rect, (SDL_Rect *) &dest);
                            }
                            ++rendering->src.y;
                            for(y = 0, dest.x = rendering->src.w / dest.w * dest.w, dest.h = sprite->height(), dest.w = m_size.x - dest.x;
                                y < rendering->src.y;
                                ++y)
                            {
                                dest.y = y * dest.h;
                                SDL_RenderCopy(renderer, temp_texture, (SDL_Rect *) &sprite->m_rect, (SDL_Rect *) &dest);
                            }

                            break;
                        }
                    }
                    SDL_DestroyTexture(temp_texture);
                    SDL_SetRenderTarget(renderer, nullptr);
                    break;
                }
            }
            save_src = rendering->src;
            save_dst = rendering->dst;
        }
        else
        {
            rendering->src = save_src;
            rendering->dst = save_dst;
        }
        rendering->dst.w *= flip.x;
        rendering->dst.h *= flip.y;

        rendering->dst.x -= offset.x;
        rendering->dst.y -= offset.y;

        rendering->texture = save_texture;
    }
} // namespace RoninEngine::Runtime
