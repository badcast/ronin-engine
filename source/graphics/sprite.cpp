#include "ronin.h"

namespace RoninEngine::Runtime
{
    const RoninEngine::Runtime::Rect Sprite::rect() { return this->m_rect; }
    void Sprite::rect(RoninEngine::Runtime::Rect rect) { this->m_rect = rect; }

    void Sprite::Center(Vec2 center) { this->m_center = center; }
    const Vec2 Sprite::Center() { return m_center; }

    const bool Sprite::valid() { return m_rect.w - m_rect.x >= 0 || m_rect.h - m_rect.y >= 0 || source; }

    int Sprite::width() { return Math::abs(m_rect.w - m_rect.x); }
    int Sprite::height() { return Math::abs(m_rect.h - m_rect.y); }

    Rect Sprite::realityRect(float&& opaque)
    {
        // NOTE: Формула пикселей для SDL :: Y Offset * (Pitch/BytesPerPixel) + X Offset
        // pixel = (SDL_Color*)pLegacyFont->surfNormal->pixels + (cy * (model->pitch / model->format->BytesPerPixel) + cx);
        Rect rect = m_rect;
        bool _place[2] {};
        if (source) {
            Color* pixels = static_cast<Color*>(source->pixels);
            Color* argb;
            // pitch /= SDL_BYTESPERPIXEL(texture->format());
            for (; !_place[0] || !_place[1];) {
                if (!_place[0]) {
                    argb = pixels + (rect.y * source->pitch + rect.x);
                    _place[0] = (Math::map<float>(argb->a, 0, 255, 0, opaque) > opaque);
                }
                if (!_place[1]) {
                    argb = pixels + (rect.w * source->pitch + rect.h);
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
