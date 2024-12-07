#include "ronin.h"

namespace RoninEngine::Runtime
{
    Sprite::Sprite() : m_center(Vec2::half)
    {
    }

    Sprite::Sprite(Image *src, const Rect &rect) : m_rect(rect), m_center(Vec2::half), surface(src)
    {
    }

    void Sprite::setSurface(Image *surface)
    {
        this->surface = surface;

        if(surface)
        {
            m_rect = surface->clip_rect;
        }
    }

    Image *Sprite::getImage() const
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
        return m_rect.w;
    }

    int Sprite::height() const
    {
        return m_rect.h;
    }

    const Vec2 Sprite::size() const
    {
        return {width() / currentWorld->irs->metricPixelsPerPoint.x, height() / currentWorld->irs->metricPixelsPerPoint.y};
    }

    Rect Sprite::realityRect(float opaque)
    {
        // NOTE: Формула пикселей для SDL :: Y Offset * (Pitch/BytesPerPixel) + X Offset
        return m_rect;
    }

    SpriteRef Sprite::CreateEmptySprite()
    {
        return Primitive::CreateEmptySprite();
    }

    SpriteRef Sprite::CreateBlackSprite()
    {
        return Primitive::CreateSpriteRectangle( Vec2::one, Color::black);
    }

    SpriteRef Sprite::CreateWhiteSprite()
    {
        return Primitive::CreateSpriteRectangle(Vec2::one, Color::white);
    }

} // namespace RoninEngine::Runtime
