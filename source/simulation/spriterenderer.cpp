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
        , size(proto.size)
        , renderType(proto.renderType)
        , renderOut(proto.renderOut)
        , renderPresentMode(proto.renderPresentMode)
        , color(proto.color)
        , flip(proto.flip)
        , offset(proto.offset)
    {
    }

    SpriteRenderer::~SpriteRenderer() { }

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

    void SpriteRenderer::offset_from_local_position(Vec2 localPosition) { offset_from_world_position(transform()->position() + localPosition); }

    void SpriteRenderer::offset_from_world_position(Vec2 position)
    {
        // Convert world position to Screen Point
        offset = Camera::world_2_screen(transform()->position()) - Camera::world_2_screen(position);
    }

    void SpriteRenderer::render(Rendering* rendering)
    {
        static std::uint16_t x, y;
        static SDL_Rect dest;
        SDL_Texture* textureCache = nullptr;
        auto&& _srcRect = (rendering->src);
        auto&& _dstRect = (rendering->dst);
        //    auto& _srcRect = (render->src);
        //    auto& _dstRect = (render->dst);

        if (sprite && sprite->width() && sprite->height()) {
            switch (this->renderType) {
            case SpriteRenderType::Simple:
                textureCache = SDL_CreateTextureFromSurface(rendering->renderer, sprite->source);
                _srcRect.w = sprite->width();
                _srcRect.h = sprite->height();
                _dstRect.w = sprite->width() * abs(this->size.x) / pixelsPerPoint;
                _dstRect.h = sprite->height() * abs(this->size.y) / pixelsPerPoint;
                switch (renderPresentMode) {
                    // render as fixed (Resize mode)
                case SpriteRenderPresentMode::Fixed:
                    //                        _dstRect.w = sprite->width() * abs(this->size.x) / pixelsPerPoint;
                    //                        _dstRect.h = sprite->height() * abs(this->size.y) / pixelsPerPoint;
                    break;
                    // render as cut
                case SpriteRenderPresentMode::Place:
                    _srcRect.w *= Math::abs(this->size.x);
                    _srcRect.h *= Math::abs(this->size.y);
                    break;
                }
                break;
            case SpriteRenderType::Tile: {
                _srcRect.w = abs(this->size.x) * sprite->width();
                _srcRect.h = abs(this->size.y) * sprite->width();
                _dstRect.w = sprite->width() * abs(this->size.x) / pixelsPerPoint;
                _dstRect.h = sprite->height() * abs(this->size.y) / pixelsPerPoint;

                // if (this->tileRenderPresent == SpriteRenderTile::Fixed) {
                //     _srcRect.w = (_srcRect.w / sprite->width()) * sprite->width();
                //     _srcRect.h = (_srcRect.h / sprite->height()) * sprite->height();
                // }

                // generate tiles

                textureCache = SDL_CreateTexture(rendering->renderer, sdl_default_pixelformat, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET, _srcRect.w, _srcRect.h);
                if (textureCache == nullptr)
                    Application::fail("Texture create fail");

                SDL_SetRenderTarget(rendering->renderer, textureCache);

                dest.w = sprite->width();
                dest.h = sprite->height();

                _srcRect.x = _srcRect.w / dest.w;
                _srcRect.y = _srcRect.h / dest.h;

                SDL_Texture* _texture = SDL_CreateTextureFromSurface(Application::get_renderer(), sprite->source);

                // render tile
                switch (this->renderPresentMode) {
                case SpriteRenderPresentMode::Fixed: {
                    for (x = 0; x < _srcRect.x; ++x) {
                        for (y = 0; y < _srcRect.y; ++y) {
                            dest.x = x * dest.w;
                            dest.y = y * dest.h;
                            SDL_RenderCopy(rendering->renderer, _texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                        }
                    }
                    break;
                }
                case SpriteRenderPresentMode::Place: {
                    for (x = 0; x < _srcRect.x; ++x) {
                        for (y = 0; y < _srcRect.y; ++y) {
                            dest.x = x * dest.w;
                            dest.y = y * dest.h;

                            SDL_RenderCopy(rendering->renderer, _texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                        }
                    }
                    // place remained
                    for (x = 0, dest.y = _srcRect.h / dest.h * dest.h, dest.h = size.y - dest.y; x < _srcRect.x; ++x) {
                        dest.x = x * dest.w;
                        SDL_RenderCopy(rendering->renderer, _texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                    }
                    ++_srcRect.y;
                    for (y = 0, dest.x = _srcRect.w / dest.w * dest.w, dest.h = sprite->height(), dest.w = size.x - dest.x; y < _srcRect.y; ++y) {
                        dest.y = y * dest.h;
                        SDL_RenderCopy(rendering->renderer, _texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                    }

                    break;
                }
                    SDL_DestroyTexture(_texture);
                }
                SDL_SetRenderTarget(rendering->renderer, nullptr);

            } break;
            }
        }

        _dstRect.w *= flip.x;
        _dstRect.h *= flip.y;

        _dstRect.x -= offset.x;
        _dstRect.y -= offset.y;

        rendering->texture = textureCache;
    }
} // namespace RoninEngine::Runtime
