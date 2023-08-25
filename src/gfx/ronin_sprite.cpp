#include "ronin.h"

namespace RoninEngine::Runtime
{
    Sprite::Sprite() : m_center(Vec2::half)
    {
    }

    Sprite::Sprite(native_surface_t *src, const Rect &rect) : m_rect(rect), m_center(Vec2::half), surface(src)
    {
    }

    void Sprite::set_surface(native_surface_t *surface)
    {
        this->surface = surface;

        if(surface)
        {
            m_rect = surface->clip_rect;
        }
    }

    const native_surface_t *Sprite::get_surface() const
    {
        return surface;
    }

    const Rect Sprite::rect() const
    {
        return this->m_rect;
    }
    void Sprite::rect(const Rect &rect)
    {
        this->m_rect = rect;
    }

    void Sprite::center(Vec2 center)
    {
        this->m_center = center;
    }
    const Vec2 Sprite::center() const
    {
        return m_center;
    }

    const bool Sprite::valid() const
    {
        return m_rect.w - m_rect.x >= 0 || m_rect.h - m_rect.y >= 0 || surface;
    }

    int Sprite::width() const
    {
        return Math::abs(m_rect.w - m_rect.x);
    }
    int Sprite::height() const
    {
        return Math::abs(m_rect.h - m_rect.y);
    }

    const Vec2 Sprite::size() const
    {
        return {width() / pixelsPerPoint, height() / pixelsPerPoint};
    }

    Rect Sprite::realityRect(float opaque)
    {
        // NOTE: Формула пикселей для SDL :: Y Offset * (Pitch/BytesPerPixel) + X Offset
        return m_rect;
    }

    Sprite *Sprite::spriteEmpty()
    {
        return Primitive::create_empty_sprite2D();
    }
    Sprite *Sprite::spriteBlack()
    {
        return Primitive::create_sprite2D_box(Vec2::one, Color::black);
    }
    Sprite *Sprite::spriteWhite()
    {
        return Primitive::create_sprite2D_box(Vec2::one, Color::white);
    }

} // namespace RoninEngine::Runtime
