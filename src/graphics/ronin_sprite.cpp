#include "ronin.h"

namespace RoninEngine::Runtime
{
Sprite::Sprite(SDL_Surface *src, const Rect &rect)
:m_rect(rect), surface(src)
{

}

const Rect Sprite::rect() const { return this->m_rect; }
    void Sprite::rect(const Rect& rect) { this->m_rect = rect; }

    void Sprite::center(Vec2 center) { this->m_center = center; }
    const Vec2 Sprite::center() const { return m_center; }

    const bool Sprite::valid() const{ return m_rect.w - m_rect.x >= 0 || m_rect.h - m_rect.y >= 0 || surface; }

    int Sprite::width() const{ return Math::abs(m_rect.w - m_rect.x); }
    int Sprite::height() const{ return Math::abs(m_rect.h - m_rect.y); }

    Rect Sprite::realityRect(float&& opaque)
    {
        // NOTE: Формула пикселей для SDL :: Y Offset * (Pitch/BytesPerPixel) + X Offset
        // pixel = (SDL_Color*)pLegacyFont->surfNormal->pixels + (cy * (model->pitch / model->format->BytesPerPixel) + cx);
        Rect rect = m_rect;
        bool _place[2] {};
        if (surface) {
            Color* pixels = static_cast<Color*>(surface->pixels);
            Color* argb;
            // pitch /= SDL_BYTESPERPIXEL(texture->format());
            for (; !_place[0] || !_place[1];) {
                if (!_place[0]) {
                    argb = pixels + (rect.y * surface->pitch + rect.x);
                    _place[0] = (Math::map<float>(argb->a, 0, 255, 0, opaque) > opaque);
                }
                if (!_place[1]) {
                    argb = pixels + (rect.w * surface->pitch + rect.h);
                    _place[1] = (Math::map<float>(argb->a, 0, 255, 0, opaque) > opaque);
                }

                if (!_place[0]) {
                    if (rect.x < rect.w)
                        ++rect.x;
                    else {
                        rect.x = 0;
                        if (rect.y < rect.h)
                            ++rect.y;
                        else
                            _place[0] = true;
                    }
                }

                if (!_place[1]) {
                    if (rect.w > rect.x)
                        --rect.w;
                    else {
                        rect.w = m_rect.w;
                        if (rect.h > rect.y)
                            --rect.h;
                        else
                            _place[1] = true;
                    }
                }
            }
        }
        return rect;
    }

    Sprite* Sprite::spriteEmpty() { return Primitive::create_sprite2D_box(Vec2::one, Color::black); }
    Sprite* Sprite::spriteBlack() { return Primitive::create_sprite2D_box(Vec2::one, Color::black); }
    Sprite* Sprite::spriteWhite() { return Primitive::create_sprite2D_box(Vec2::one, Color::white); }

} // namespace RoninEngine::Runtime
