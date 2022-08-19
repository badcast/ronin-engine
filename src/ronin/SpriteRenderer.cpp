#include "framework.h"

namespace RoninEngine::Runtime {

//FIXME: OPTIMIZING SPRITE RENDERER COMPONENT

SpriteRenderer::SpriteRenderer() : SpriteRenderer(typeid(*this).name()) {}

SpriteRenderer::SpriteRenderer(const std::string& name)
    : Renderer(name),
      sprite(nullptr),
      texture(nullptr),
      flip(Vec2::one),
      renderType(SpriteRenderType::Simple),
      renderTilePresent(SpriteRenderPresentTiles::Place),
      color(Color::white) {}

RoninEngine::Runtime::SpriteRenderer::SpriteRenderer(const SpriteRenderer& proto)
    : Renderer(proto.m_name),
      texture(proto.texture),
      sprite(proto.sprite),
      size(proto.size),
      renderType(proto.renderType),
      renderTilePresent(proto.renderTilePresent),
      color(proto.color),
      flip(proto.flip),
      offset(proto.offset) {}

SpriteRenderer::~SpriteRenderer() {}

Vec2 SpriteRenderer::GetSize() { return Vec2::Abs(this->size); }

void SpriteRenderer::setSprite(Sprite* sprite) {
    if (this->sprite == nullptr && (!this->size.x || !this->size.y)) {
        this->size = Vec2::one;
    }
    this->sprite = sprite;
}

void SpriteRenderer::setSpriteFromTextureToGC(Texture* texture) {
    Sprite* spr;
    GC::gc_alloc_sprite_with(&spr, texture);
    setSprite(spr);
}
Sprite* SpriteRenderer::getSprite() { return this->sprite; }

void SpriteRenderer::offsetFromLocalPosition(Vec2 localPosition) {
    offsetFromWorldPosition(transform()->position() + localPosition);
}

void SpriteRenderer::offsetFromWorldPosition(Vec2 position) {
    // Convert world position to Screen Point
    offset = Camera::WorldToScreenPoint(transform()->position()) - Camera::WorldToScreenPoint(position);
}

void SpriteRenderer::Render(Render_info* render) {
    Rect& _srcRect = render->src;
    Rectf_t& _dstRect = render->dst;
    uint16_t x, y;
    SDL_Rect dest;

    if (sprite && sprite->width() && sprite->height()) {
        switch (this->renderType) {
            case SpriteRenderType::Simple:
                if (!texture) {
                    texture = sprite->texture;
                }
                _srcRect.w = sprite->width();
                _srcRect.h = sprite->height();
                _dstRect.w = sprite->width() * abs(this->size.x) / pixelsPerPoint;
                _dstRect.h = sprite->height() * abs(this->size.y) / pixelsPerPoint;

                break;
            case SpriteRenderType::Tile: {
                _srcRect.w = abs(this->size.x) * sprite->width();
                _srcRect.h = abs(this->size.y) * sprite->width();
                _dstRect.w = sprite->width() * abs(this->size.x) / pixelsPerPoint;
                _dstRect.h = sprite->height() * abs(this->size.y) / pixelsPerPoint;

                //                if (this->tileRenderPresent == SpriteRenderTile::Fixed) {
                //                    _srcRect.w = (_srcRect.w / sprite->width()) * sprite->width();
                //                    _srcRect.h = (_srcRect.h / sprite->height()) * sprite->height();
                //                }

                if (!texture) {
                    GC::gc_alloc_texture(&texture, _srcRect.w, _srcRect.h, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET);

                    if (texture == nullptr) Application::fail("Texture create fail");

                    SDL_SetRenderTarget(render->renderer, texture->native());

                    dest.w = sprite->width();
                    dest.h = sprite->height();

                    _srcRect.x = _srcRect.w / dest.w;
                    _srcRect.y = _srcRect.h / dest.h;

                    // render tile
                    switch (this->renderTilePresent) {
                        case SpriteRenderPresentTiles::Fixed: {
                            for (x = 0; x < _srcRect.x; ++x) {
                                for (y = 0; y < _srcRect.y; ++y) {
                                    dest.x = x * dest.w;
                                    dest.y = y * dest.h;
                                    SDL_RenderCopy(render->renderer, sprite->texture->native(), (SDL_Rect*)&sprite->m_rect,
                                                   (SDL_Rect*)&dest);
                                }
                            }
                            break;
                        }
                        case SpriteRenderPresentTiles::Place: {
                            for (x = 0; x < _srcRect.x; ++x) {
                                for (y = 0; y < _srcRect.y; ++y) {
                                    dest.x = x * dest.w;
                                    dest.y = y * dest.h;

                                    SDL_RenderCopy(render->renderer, sprite->texture->native(), (SDL_Rect*)&sprite->m_rect,
                                                   (SDL_Rect*)&dest);
                                }
                            }
                            // place remained
                            for (x = 0, dest.y = _srcRect.h / dest.h * dest.h, dest.h = size.y - dest.y; x < _srcRect.x; ++x) {
                                dest.x = x * dest.w;
                                SDL_RenderCopy(render->renderer, sprite->texture->native(), (SDL_Rect*)&sprite->m_rect,
                                               (SDL_Rect*)&dest);
                            }
                            ++_srcRect.y;
                            for (y = 0, dest.x = _srcRect.w / dest.w * dest.w, dest.h = sprite->height(),
                                dest.w = size.x - dest.x;
                                 y < _srcRect.y; ++y) {
                                dest.y = y * dest.h;
                                SDL_RenderCopy(render->renderer, sprite->texture->native(), (SDL_Rect*)&sprite->m_rect,
                                               (SDL_Rect*)&dest);
                            }

                            break;
                        }
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

    render->texture = texture;
}
}  // namespace RoninEngine::Runtime
