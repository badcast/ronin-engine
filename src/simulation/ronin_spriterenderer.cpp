#include "ronin.h"

namespace RoninEngine::Runtime
{

    // FIXME: OPTIMIZING SPRITE RENDERER COMPONENT

    SpriteRenderer::SpriteRenderer()
        : SpriteRenderer(DESCRIBE_AS_MAIN_OFF(SpriteRenderer))
    {
    }

    SpriteRenderer::SpriteRenderer(const std::string& name)
        : Renderer(DESCRIBE_AS_ONLY_NAME(SpriteRenderer))
        , sprite(nullptr)
        , save_texture(nullptr)
        , flip(Vec2::one)
        , renderType(SpriteRenderType::Simple)
        , renderOut(SpriteRenderOut::Centering)
        , renderPresentMode(SpriteRenderPresentMode::Fixed)
        , color(Color::white)
    {
        DESCRIBE_AS_MAIN(SpriteRenderer);
    }

    SpriteRenderer::SpriteRenderer(const SpriteRenderer& proto)
        : Renderer(proto.m_name)
        , sprite(proto.sprite)
        , save_texture(nullptr)
        , size(proto.size)
        , renderType(proto.renderType)
        , renderOut(proto.renderOut)
        , renderPresentMode(proto.renderPresentMode)
        , color(proto.color)
        , flip(proto.flip)
        , offset(proto.offset)
    {
    }

    SpriteRenderer::~SpriteRenderer() { free_render_cache(); }

    Vec2 SpriteRenderer::get_size() { return Vec2::abs(this->size); }

    Vec2 SpriteRenderer::get_offset()
    {
        Vec2 outOffset;
        switch (renderOut) {
        case SpriteRenderOut::Origin:
            outOffset = -size + size / 2 + Vec2::half;

            break;
        }
        return outOffset;
    }

    Rect SpriteRenderer::get_relative_size()
    {
        Rect rect;
        if (sprite) {
            rect = sprite->realityRect();
        }
        return rect;
    }

    void SpriteRenderer::set_sprite(Sprite* sprite)
    {
        if (this->sprite == nullptr && (!this->size.x || !this->size.y)) {
            this->size = Vec2::one;
        }
        this->sprite = sprite;
    }

    Sprite* SpriteRenderer::get_sprite() { return this->sprite; }

    void SpriteRenderer::free_render_cache()
    {
        if (save_texture) {
            SDL_DestroyTexture(save_texture);
            save_texture = nullptr;
        }
    }

    void SpriteRenderer::render(Rendering* rendering)
    {
        if (sprite == nullptr)
            return;

        std::uint16_t x, y;
        SDL_Rect dest;

        if (save_texture == nullptr) {
            switch (this->renderType) {
            case SpriteRenderType::Simple:
                save_texture = SDL_CreateTextureFromSurface(renderer, sprite->surface);
                rendering->src.w = sprite->width();
                rendering->src.h = sprite->height();
                rendering->dst.w = sprite->width() * abs(this->size.x) / pixelsPerPoint;
                rendering->dst.h = sprite->height() * abs(this->size.y) / pixelsPerPoint;
                switch (renderPresentMode) {
                    // render as fixed (Resize mode)
                case SpriteRenderPresentMode::Fixed:
                    //                        rendering->dst.w = sprite->width() * abs(this->size.x) / pixelsPerPoint;
                    //                        rendering->dst.h = sprite->height() * abs(this->size.y) / pixelsPerPoint;
                    break;
                    // render as cut
                case SpriteRenderPresentMode::Place:
                    rendering->src.w *= Math::abs(this->size.x);
                    rendering->src.h *= Math::abs(this->size.y);
                    break;
                }
                break;
            case SpriteRenderType::Tile: {
                rendering->src.w = abs(this->size.x) * sprite->width();
                rendering->src.h = abs(this->size.y) * sprite->width();
                rendering->dst.w = sprite->width() * abs(this->size.x) / pixelsPerPoint;
                rendering->dst.h = sprite->height() * abs(this->size.y) / pixelsPerPoint;

                // if (this->tileRenderPresent == SpriteRenderTile::Fixed) {
                //     rendering->src.w = (rendering->src.w / sprite->width()) * sprite->width();
                //     rendering->src.h = (rendering->src.h / sprite->height()) * sprite->height();
                // }

                // generate tiles
                save_texture = SDL_CreateTexture(renderer, sdl_default_pixelformat, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET, rendering->src.w, rendering->src.h);
                if (save_texture == nullptr)
                    Application::fail("Texture create fail");

                SDL_SetRenderTarget(renderer, save_texture);

                dest.w = sprite->width();
                dest.h = sprite->height();

                rendering->src.x = rendering->src.w / dest.w;
                rendering->src.y = rendering->src.h / dest.h;

                SDL_Texture* temp_texture = SDL_CreateTextureFromSurface(renderer, sprite->surface);

                // render tile
                switch (this->renderPresentMode) {
                case SpriteRenderPresentMode::Fixed: {
                    for (x = 0; x < rendering->src.x; ++x) {
                        for (y = 0; y < rendering->src.y; ++y) {
                            dest.x = x * dest.w;
                            dest.y = y * dest.h;
                            SDL_RenderCopy(renderer, temp_texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                        }
                    }
                    break;
                }
                case SpriteRenderPresentMode::Place: {
                    for (x = 0; x < rendering->src.x; ++x) {
                        for (y = 0; y < rendering->src.y; ++y) {
                            dest.x = x * dest.w;
                            dest.y = y * dest.h;

                            SDL_RenderCopy(renderer, temp_texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                        }
                    }
                    // place remained
                    for (x = 0, dest.y = rendering->src.h / dest.h * dest.h, dest.h = size.y - dest.y; x < rendering->src.x; ++x) {
                        dest.x = x * dest.w;
                        SDL_RenderCopy(renderer, temp_texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                    }
                    ++rendering->src.y;
                    for (y = 0, dest.x = rendering->src.w / dest.w * dest.w, dest.h = sprite->height(), dest.w = size.x - dest.x; y < rendering->src.y; ++y) {
                        dest.y = y * dest.h;
                        SDL_RenderCopy(renderer, temp_texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
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
        } else {
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
