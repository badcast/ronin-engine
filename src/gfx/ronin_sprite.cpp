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
        return surface != nullptr && m_rect.w > 0 && m_rect.h > 0;
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
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);
        return {width() / mpp.x, height() / mpp.y};
    }

    Rect Sprite::realityRect(float opaque)
    {
        if(surface == nullptr || m_rect.w <= 0 || m_rect.h <= 0)
            return m_rect;

        Uint8 minAlpha = static_cast<Uint8>(Math::Clamp01(opaque) * 255.0f);
        int minX = m_rect.w, minY = m_rect.h, maxX = -1, maxY = -1;

        SDL_LockSurface(surface);
        int bpp = surface->format->BytesPerPixel;
        int startX = std::max(0, m_rect.x);
        int startY = std::max(0, m_rect.y);
        int endX = std::min(m_rect.x + m_rect.w, surface->w);
        int endY = std::min(m_rect.y + m_rect.h, surface->h);

        for(int y = startY; y < endY; ++y)
        {
            Uint8 *row = static_cast<Uint8 *>(surface->pixels) + y * surface->pitch;
            for(int x = startX; x < endX; ++x)
            {
                Uint32 pixel = 0;
                Uint8 a = 255;
                if(bpp == 4)
                {
                    pixel = *reinterpret_cast<Uint32 *>(row + x * 4);
                    if(surface->format->Amask != 0)
                        a = static_cast<Uint8>((pixel & surface->format->Amask) >> surface->format->Ashift);
                }
                else if(bpp == 1 && surface->format->palette)
                {
                    pixel = *(row + x);
                    a = surface->format->palette->colors[pixel].a;
                }

                if(a >= minAlpha)
                {
                    int localX = x - startX;
                    int localY = y - startY;
                    if(localX < minX) minX = localX;
                    if(localX > maxX) maxX = localX;
                    if(localY < minY) minY = localY;
                    if(localY > maxY) maxY = localY;
                }
            }
        }
        SDL_UnlockSurface(surface);

        if(maxX < minX || maxY < minY)
            return Rect {0, 0, 0, 0};

        return Rect {minX, minY, maxX - minX + 1, maxY - minY + 1};
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
