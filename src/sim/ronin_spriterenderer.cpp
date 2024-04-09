#include "ronin.h"
#include "ronin_render_cache.h"

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
          renderType(SpriteRenderType::Simple),
          renderOut(SpriteRenderOut::Centering),
          flip(SpriteRenderFlip::FlipNone),
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
          flip(proto.flip),
          renderPresentMode(proto.renderPresentMode),
          color(proto.color),
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
        color = value;
    }

    const Vec2 SpriteRenderer::getSize() const
    {
        return this->m_size;
    }

    void SpriteRenderer::setSize(const Vec2 &newSize)
    {
        m_size = Vec2::Abs(newSize);
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

        if(save_texture == nullptr)
        {
            std::uint16_t x, y;
            SDL_Rect dest;
            SDL_Texture *newTexture;
            switch(this->renderType)
            {
                case SpriteRenderType::Simple:
                    newTexture = SDL_CreateTextureFromSurface(env.renderer, sprite->surface);
                    Uint32 format, format2;
                    int ac,ac2;

                    SDL_QueryTexture(newTexture, &format, &ac, nullptr, nullptr);

                    save_texture = render_make_texture(sprite );
                    SDL_QueryTexture(newTexture, &format2, &ac2, nullptr, nullptr);
                    SDL_DestroyTexture(newTexture);
                    rendering->src.w = sprite->width();
                    rendering->src.h = sprite->height();
                    rendering->dst.w = sprite->width();
                    rendering->dst.h = sprite->height();
                    break;
                    save_texture = SDL_CreateTextureFromSurface(env.renderer, sprite->surface);
                    rendering->src.w = sprite->width();
                    rendering->src.h = sprite->height();
                    rendering->dst.w = sprite->width();
                    rendering->dst.h = sprite->height();
                    switch(renderPresentMode)
                    {
                            // render as fixed (Resize mode)
                            // case SpriteRenderPresentMode::Fixed:
                            //      rendering->dst.w = sprite->width() * abs(this->m_size.x) / pixelsPerPoint;
                            //      rendering->dst.h = sprite->height() * abs(this->m_size.y) / pixelsPerPoint;
                            // break;

                            // render as cut
                        case SpriteRenderPresentMode::Place:
                            rendering->src.w *= m_size.x;
                            rendering->src.h *= m_size.y;
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
                        env.renderer,
                        ronin_default_pixelformat,
                        SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET,
                        1024,
                        1024);
                    if(save_texture == nullptr)
                        RoninSimulator::ShowMessageFail("Texture create fail");

                    SDL_SetRenderTarget(env.renderer, save_texture);

                    dest.w = sprite->width();
                    dest.h = sprite->height();

                    rendering->src.x = rendering->src.w / dest.w;
                    rendering->src.y = rendering->src.h / dest.h;

                    SDL_Texture *temp_texture = SDL_CreateTextureFromSurface(env.renderer, sprite->surface);

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
                                    SDL_RenderCopy(env.renderer, temp_texture, (SDL_Rect *) &sprite->m_rect, (SDL_Rect *) &dest);
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

                                    SDL_RenderCopy(env.renderer, temp_texture, (SDL_Rect *) &sprite->m_rect, (SDL_Rect *) &dest);
                                }
                            }
                            // place remained
                            for(x = 0, dest.y = rendering->src.h / dest.h * dest.h, dest.h = m_size.y - dest.y; x < rendering->src.x; ++x)
                            {
                                dest.x = x * dest.w;
                                SDL_RenderCopy(env.renderer, temp_texture, (SDL_Rect *) &sprite->m_rect, (SDL_Rect *) &dest);
                            }
                            ++rendering->src.y;
                            for(y = 0, dest.x = rendering->src.w / dest.w * dest.w, dest.h = sprite->height(), dest.w = m_size.x - dest.x;
                                y < rendering->src.y;
                                ++y)
                            {
                                dest.y = y * dest.h;
                                SDL_RenderCopy(env.renderer, temp_texture, (SDL_Rect *) &sprite->m_rect, (SDL_Rect *) &dest);
                            }

                            break;
                        }
                    }
                    SDL_DestroyTexture(temp_texture);
                    SDL_SetRenderTarget(env.renderer, nullptr);
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

        if(save_texture)
        {
            // set color for Texture
            SDL_SetTextureColorMod(save_texture, color.r, color.g, color.b);
            SDL_SetTextureAlphaMod(save_texture, color.a);
        }

        rendering->dst.x += offset.x;
        rendering->dst.y += offset.y;

        rendering->dst.w = (rendering->dst.w * m_size.x / pixelsPerPoint) *
            ((static_cast<int>(flip) & static_cast<int>(SpriteRenderFlip::FlipHorizontal)) ? -1 : 1);
        rendering->dst.h = (rendering->dst.h * m_size.y / pixelsPerPoint) *
            ((static_cast<int>(flip) & static_cast<int>(SpriteRenderFlip::FlipVertical)) ? -1 : 1);

        rendering->texture = save_texture;
    }
} // namespace RoninEngine::Runtime
