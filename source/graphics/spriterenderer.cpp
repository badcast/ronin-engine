#include "ronin.h"

namespace RoninEngine::Runtime
{

    // FIXME: OPTIMIZING SPRITE RENDERER COMPONENT

    SpriteRenderer::SpriteRenderer()
        : SpriteRenderer(typeid(*this).name())
    {
    }

    SpriteRenderer::SpriteRenderer(const std::string& name)
        : Renderer(name)
        , sprite(nullptr)
        , textureCache(nullptr)
        , flip(Vec2::one)
        , renderType(SpriteRenderType::Simple)
        , renderOut(SpriteRenderOut::Centering)
        , renderPresentMode(SpriteRenderPresentMode::Place)
        , color(Color::white)
    {
    }

    SpriteRenderer::SpriteRenderer(const SpriteRenderer& proto)
        : Renderer(proto.m_name)
        , textureCache(proto.textureCache)
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

    Vec2 SpriteRenderer::getSize() { return Vec2::Abs(this->size); }

    Vec2 SpriteRenderer::getOffset()
    {
        Vec2 outOffset;
        switch (renderOut) {
        case SpriteRenderOut::Origin:
            outOffset = -size + size / 2 + Vec2::half;

            break;
        }
        return outOffset;
    }

    Rect SpriteRenderer::getFactical()
    {
        Rect rect;
        if (sprite) {
            rect = sprite->realityRect();
        }
        return rect;
    }

    void SpriteRenderer::setSprite(Sprite* sprite)
    {
        if (this->sprite == nullptr && (!this->size.x || !this->size.y)) {
            this->size = Vec2::one;
        }
        this->sprite = sprite;
    }

    void SpriteRenderer::setSpriteFromTextureToGC(SDL_Surface* src)
    {
        Sprite* spr;
        GC::gc_alloc_sprite_with(&spr, src);
        setSprite(spr);
    }
    Sprite* SpriteRenderer::getSprite() { return this->sprite; }

    void SpriteRenderer::offsetFromLocalPosition(Vec2 localPosition) { offsetFromWorldPosition(transform()->position() + localPosition); }

    void SpriteRenderer::offsetFromWorldPosition(Vec2 position)
    {
        // Convert world position to Screen Point
        offset = Camera::WorldToScreenPoint(transform()->position()) - Camera::WorldToScreenPoint(position);
    }

    void SpriteRenderer::Render(Render_info* render)
    {
        static std::uint16_t x, y;
        static SDL_Rect dest;
#define _srcRect (render->src)
#define _dstRect (render->dst)
        //    auto& _srcRect = (render->src);
        //    auto& _dstRect = (render->dst);

        if (sprite && sprite->width() && sprite->height()) {
            switch (this->renderType) {
            case SpriteRenderType::Simple:
                if (!textureCache) {
                    GC::gc_alloc_texture_from(&textureCache, sprite->source);
                }
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
                if (!textureCache) {
                    GC::gc_alloc_texture(&textureCache, _srcRect.w, _srcRect.h, SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET);
                    if (textureCache == nullptr)
                        Application::fail("Texture create fail");

                    SDL_SetRenderTarget(render->renderer, textureCache->native());

                    dest.w = sprite->width();
                    dest.h = sprite->height();

                    _srcRect.x = _srcRect.w / dest.w;
                    _srcRect.y = _srcRect.h / dest.h;

                    SDL_Texture* _texture = SDL_CreateTextureFromSurface(Application::getRenderer(), sprite->source);

                    // render tile
                    switch (this->renderPresentMode) {
                    case SpriteRenderPresentMode::Fixed: {
                        for (x = 0; x < _srcRect.x; ++x) {
                            for (y = 0; y < _srcRect.y; ++y) {
                                dest.x = x * dest.w;
                                dest.y = y * dest.h;
                                SDL_RenderCopy(render->renderer, _texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                            }
                        }
                        break;
                    }
                    case SpriteRenderPresentMode::Place: {
                        for (x = 0; x < _srcRect.x; ++x) {
                            for (y = 0; y < _srcRect.y; ++y) {
                                dest.x = x * dest.w;
                                dest.y = y * dest.h;

                                SDL_RenderCopy(render->renderer, _texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                            }
                        }
                        // place remained
                        for (x = 0, dest.y = _srcRect.h / dest.h * dest.h, dest.h = size.y - dest.y; x < _srcRect.x; ++x) {
                            dest.x = x * dest.w;
                            SDL_RenderCopy(render->renderer, _texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                        }
                        ++_srcRect.y;
                        for (y = 0, dest.x = _srcRect.w / dest.w * dest.w, dest.h = sprite->height(), dest.w = size.x - dest.x; y < _srcRect.y; ++y) {
                            dest.y = y * dest.h;
                            SDL_RenderCopy(render->renderer, _texture, (SDL_Rect*)&sprite->m_rect, (SDL_Rect*)&dest);
                        }

                        break;
                    }
                        SDL_DestroyTexture(_texture);
                    }
                    SDL_SetRenderTarget(render->renderer, nullptr);
                }
            } break;
            }
        }

        _dstRect.w *= flip.x;
        _dstRect.h *= flip.y;

        _dstRect.x -= offset.x;
        _dstRect.y -= offset.y;

        render->texture = textureCache;
#undef _dstRect
#undef _srcRect
    }
} // namespace RoninEngine::Runtime
