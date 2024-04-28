#include "ronin.h"
#include "ronin_render_cache.h"

namespace RoninEngine::Runtime
{

    void internal_drawLine(Vec2 a, Vec2 b)
    {
        if(!Camera::mainCamera())
            return;

        Vec2 p = Camera::mainCamera()->transform()->position();
        Vec2 dst;

        Vec2 scale;
        scale = switched_world->irs->metricPixelsPerPoint;

        dst.x = gscope.activeResolution.width / 2.f;
        dst.y = gscope.activeResolution.height / 2.f;

        a.x = dst.x - (p.x - a.x) * scale.x;
        a.y = dst.y + (p.y - a.y) * scale.y;
        b.x = dst.x - (p.x - b.x) * scale.x;
        b.y = dst.y + (p.y - b.y) * scale.y;

        int x1 = static_cast<int>(a.x);
        int y1 = static_cast<int>(a.y);
        int x2 = static_cast<int>(b.x);
        int y2 = static_cast<int>(b.y);

        SDL_RenderDrawLine(gscope.renderer, x1, y1, x2, y2);
    }

    Color RenderUtility::GetColor()
    {
        Color clb;
        SDL_GetRenderDrawColor(gscope.renderer, &clb.r, &clb.g, &clb.b, &clb.a);
        return clb;
    }

    void RenderUtility::SetColor(const Color &newColor)
    {
        SDL_SetRenderDrawColor(gscope.renderer, newColor.r, newColor.g, newColor.b, newColor.a);
    }

    void RenderUtility::DrawLine(Vec2 a, Vec2 b)
    {
        internal_drawLine(a, b);
    }

    void RenderUtility::DrawLineWithPivot(Vec2 ared, Vec2 bblue)
    {
        Color last = GetColor();
        DrawLine(ared, bblue);

        SetColor(Color::red);
        DrawCircle(ared, 0.1f);
        SetColor(Color::blue);
        DrawCircle(bblue, 0.1f);
        SetColor(last);
    }

    void drawBox()
    {
        Vec2 a, b;

        a = switched_world->irs->metricPixelsPerPoint;
        b = a;
        b.x *= -1;
        internal_drawLine(a, b);

        a = switched_world->irs->metricPixelsPerPoint;
        a.y *= -1;
        b = switched_world->irs->metricPixelsPerPoint * -1;
        internal_drawLine(a, b);

        a = switched_world->irs->metricPixelsPerPoint;
        a.x *= -1;
        b = switched_world->irs->metricPixelsPerPoint * -1;
        internal_drawLine(a, b);

        a = switched_world->irs->metricPixelsPerPoint;
        b = a;
        b.y *= -1;
        internal_drawLine(a, b);
    }

    void RenderUtility::DrawPosition(const Vec2 &origin, float scalar)
    {
        Vec2 a(origin), b(origin);

        // Draw Line H
        a.x -= scalar;
        b.x += scalar;

        internal_drawLine(a, b);

        // Draw Line V
        b = a = origin;
        a.y -= scalar;
        b.y += scalar;
        internal_drawLine(a, b);
    }

    void RenderUtility::DrawSquare(Vec2 origin, float width)
    {
        DrawRectangle(origin, width, width);
    }

    void RenderUtility::DrawRectangle(Vec2 origin, float width, float height)
    {
        origin = Camera::WorldToScreenPoint(origin);
        std::uint16_t x, y;
        width *= switched_world->irs->metricPixelsPerPoint.x;
        height *= switched_world->irs->metricPixelsPerPoint.y;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        rectangleColor(gscope.renderer, x, y, x + width, y + height, GetColor());
    }

    void RenderUtility::DrawRectangleRotate(Vec2 origin, Vec2 size, float angleRadian)
    {
        float cos, sin;
        int i, dx, dy, cx, cy;
        SDL_Point vertices[4];

        cos = Math::Cos(angleRadian);
        sin = Math::Sin(angleRadian);

        origin = Camera::WorldToScreenPoint(origin);
        size = Vec2::Scale(size,switched_world->irs->metricPixelsPerPoint) ;
        cx = origin.x;
        cy = origin.y;
        origin -= size / 2;
        size += origin;
        for(i = 0; i < 4; ++i)
        {
            dx = (i == 1 || i == 2) ? size.x : origin.x;
            dy = (i == 2 || i == 3) ? size.y : origin.y;
            vertices[i].x = cx + (dx - cx) * cos - (dy - cy) * sin;
            vertices[i].y = cy + (dx - cx) * sin + (dy - cy) * cos;
        }

        for(i = 0; i < 4; ++i)
        {
            dx = (i + 1) % 4;
            SDL_RenderDrawLine(gscope.renderer, vertices[i].x, vertices[i].y, vertices[dx].x, vertices[dx].y);
        }
    }

    void RenderUtility::DrawSquareRounded(Vec2 origin, float width, std::uint16_t radius)
    {
        DrawRectangleRounded(origin, width, width, radius);
    }

    void RenderUtility::DrawRectangleRounded(Vec2 origin, float width, float height, std::uint16_t radius)
    {
        origin = Camera::WorldToScreenPoint(origin);
        std::uint16_t x, y;
        width *= switched_world->irs->metricPixelsPerPoint.x;
        height *= switched_world->irs->metricPixelsPerPoint.y;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        roundedRectangleColor(gscope.renderer, x, y, x + width, y + height, radius, GetColor());
    }

    void RenderUtility::Draw2DWorldSpace(const Vec2 &origin, int depth)
    {
        int i;
        Vec2 dest1, dest2;
        dest1 = dest2 = origin;
        Color lastColor = GetColor();
        SetColor(0xaa575757);

        // Draw H and V position
        DrawPosition(std::move(origin), defaultMaxWorldScalar);

        for(i = 0; i < depth; ++i)
        {
            dest1 += Vec2::one;
            DrawPosition(std::move(dest1), defaultMaxWorldScalar);

            dest2 += Vec2::minusOne;
            DrawPosition(std::move(dest2), defaultMaxWorldScalar);
        }

        SetColor(lastColor);
    }

    void RenderUtility::DrawNavMesh(AI::NavMesh *navMesh)
    {
        using namespace AI;
        Vec2Int p1, p2;
        Color prev;
        Color next;
        std::uint32_t yDefault;

        Neuron *self;

        // First Point (Top Left)
        navMesh->get(Camera::ScreenToWorldPoint(Vec2::zero), p1);
        // Last Point (Bottom Right)
        navMesh->get(Camera::ScreenToWorldPoint(Vec2(gscope.activeResolution.width, gscope.activeResolution.height)), p2);

        prev = GetColor();
        SetColor(next = 0xfff6f723);

        yDefault = p1.y;
        while(p1.x <= p2.x)
        {
            while(p1.y <= p2.y)
            {
                bool draw_pos = true;
                self = navMesh->get(p1);
                Vec2 lastPosition = navMesh->PointToWorldPoint(p1);
                if(self == nullptr)
                {
                    next.r = 64;
                    next.g = 0;
                    next.b = 0;
                }
                else if(navMesh->hasLocked(p1))
                {
                    next.r = 255;
                    next.g = 255;
                    next.b = 255;
                    // draw indexed
                    SetColor(next);

                    // draw line
                    AI::NavListSite neigbours = navMesh->GetNeighbours(AI::NavIdentity::PlusMethod, p1);
                    for(int x = 0; x < neigbours.size(); ++x)
                    {
                        Vec2Int *npoint = &neigbours[x];
                        if(navMesh->hasLocked(*npoint))
                        {
                            internal_drawLine(navMesh->PointToWorldPoint(p1), navMesh->PointToWorldPoint(*npoint));
                        }
                    }
                    draw_pos = false;
                }
                else
                {
                    if(self->f)
                    {
                        next.r = 53;
                        next.g = 200;
                        next.b = 100;
                    }
                    else
                        draw_pos = false;
                }
                SetColor(next);
                if(draw_pos)
                    DrawPosition(lastPosition, 0.01f);
                ++p1.y;
            }
            p1.y = yDefault;
            ++p1.x;
        }

        SetColor(prev);
    }

    void RenderUtility::DrawTriangle(Vec2 origin, float base, float height, bool fill)
    {
        Vec2 a, b, pivot = origin;
        pivot.y -= height / 2;
        a = b = pivot;
        a.x -= base / 2;
        b.x += base / 2;
        pivot.y += height;

        // draw base
        DrawLine(a, b);

        // draw left side
        DrawLine(a, pivot);

        // draw right side
        DrawLine(b, pivot);

        if(fill)
        {
            base /= 2;
            height /= 2;

            pivot.y = a.y + height / 2;
            DrawFillRect(pivot, base, height);
        }
    }

    void RenderUtility::DrawText(Vec2 origin, const std::string &text)
    {
        using namespace RoninEngine::UI;

        origin = Camera::WorldToScreenPoint(origin);
        Render_String_ttf(text.data(), 11, Vec2::RoundToInt(origin), true);
    }

    void RenderUtility::DrawTextLegacy(Vec2 origin, const std::string &text, bool hilight)
    {
        using namespace RoninEngine::UI;

        Rect r;
        // relative to
        origin = Camera::WorldToScreenPoint(origin);
        r.x = static_cast<int>(origin.x);
        r.y = static_cast<int>(origin.y);

        UI::Render_String_Legacy(r, text.c_str(), text.length(), 2, UI::Align::Left, true, hilight);
    }

    void RenderUtility::DrawCircle(Vec2 origin, float distance)
    {
        origin = Camera::WorldToScreenPoint(origin);
        std::uint16_t x, y, r;
        x = Math::Number(origin.x);
        y = Math::Number(origin.y);
        r = static_cast<std::uint16_t>(distance * switched_world->irs->metricPixelsPerPoint.x);
        Color m_color = GetColor();

        circleRGBA(gscope.renderer, x, y, r, m_color.r, m_color.g, m_color.b, m_color.a);
    }

    void RenderUtility::DrawArrow(Vec2 origin, Vec2 dir, float tailLength)
    {
        // tail
        tailLength = Math::Abs(tailLength);
        Vec2 pos2 = origin + (dir - origin) * tailLength; // world to local and set point to dir

        internal_drawLine(origin, pos2);
        float angle = Vec2::Angle(pos2, dir);

        origin = pos2 + Vec2::RotateAround(pos2, Vec2::one * tailLength, angle);
        internal_drawLine(pos2, origin);
    }

    void RenderUtility::DrawSprite(Sprite *sprite, Vec2 origin, Vec2 size, float angleRadian)
    {
        Rect rect;
        origin = Camera::WorldToScreenPoint(origin);

        rect.x = sprite->width() * sprite->m_center.x;
        rect.y = sprite->height() * sprite->m_center.y;

        rect.x = origin.x - rect.x;
        rect.y = origin.y - rect.y;
        rect.w = sprite->width() * size.x;
        rect.h = sprite->height() * size.x;

        DrawSpriteToScreen(sprite, rect, angleRadian);
    }

    void RenderUtility::DrawSpriteToScreen(Sprite *sprite, const Rect &rect, float angleRadian)
    {
        SDL_Texture *texture;

        if(sprite == nullptr || sprite->surface == nullptr)
            return;

        texture = render_cache_texture(sprite);
        if(texture)
        {
            // TODO: Move SDL_RenderCopyEx to Go
            SDL_RenderCopyEx(
                gscope.renderer,
                texture,
                reinterpret_cast<const SDL_Rect *>(&sprite->m_rect),
                reinterpret_cast<const SDL_Rect *>(&rect),
                angleRadian * Math::rad2deg,
                nullptr,
                SDL_RendererFlip::SDL_FLIP_NONE);
        }
    }

    void RenderUtility::DrawSpriteExtent(Sprite *sprite, const Rect &rect, float angleRadian)
    {
        SDL_Texture *texture = render_cache_texture(sprite);

        render_texture_extension(
            texture, nullptr, reinterpret_cast<SDL_Rect *>(&sprite->m_rect), reinterpret_cast<const SDL_Rect *>(&rect), angleRadian);
    }

    void RenderUtility::DrawSpriteExtent(Sprite *sprite, Vec2Int baseSize, const Rect &rect, float angleRadian)
    {
        SDL_Texture *texture = render_cache_texture(sprite);

        render_texture_extension(
            texture, &baseSize, reinterpret_cast<SDL_Rect *>(&sprite->m_rect), reinterpret_cast<const SDL_Rect *>(&rect), angleRadian);
    }

    void RenderUtility::DrawFillRect(Vec2 center, float width, float height)
    {
        center = Camera::WorldToScreenPoint(center);
        width *= switched_world->irs->metricPixelsPerPoint.x;
        height *= switched_world->irs->metricPixelsPerPoint.y;
        Rectf rect {center.x - width / 2, center.y - height / 2, width, height};

        SDL_RenderFillRectF(gscope.renderer, reinterpret_cast<SDL_FRect *>(&rect));
    }

    void RenderUtility::DrawFillRectRounded(Vec2 origin, float width, float height, uint16_t radius)
    {
        std::uint16_t x, y;
        origin = Camera::WorldToScreenPoint(origin);
        width *= switched_world->irs->metricPixelsPerPoint.x;
        height *= switched_world->irs->metricPixelsPerPoint.y;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        roundedBoxColor(gscope.renderer, x, y, x + width, y + height, radius, GetColor());
    }

    void RenderUtility::DrawFillSquare(Vec2 origin, float width)
    {
        origin = Camera::WorldToScreenPoint(origin);
        width *= switched_world->irs->metricPixelsPerPoint.x;
        Rectf rect {origin.x - width / 2, origin.y - width / 2, width, width};

        SDL_RenderFillRectF(gscope.renderer, reinterpret_cast<SDL_FRect *>(&rect));
    }

    void RenderUtility::DrawFillSquareRounded(Vec2 origin, float width, std::uint16_t radius)
    {
        DrawFillRectRounded(origin, width, width, radius);
    }

    void RenderUtility::DrawFillCircle(Vec2 origin, float distance)
    {
        origin = Camera::WorldToScreenPoint(origin);
        std::uint16_t x, y, r;
        x = Math::Number(origin.x);
        y = Math::Number(origin.y);
        r = static_cast<std::uint16_t>(distance * switched_world->irs->metricPixelsPerPoint.x);
        Color m_color = GetColor();

        filledCircleColor(gscope.renderer, x, y, r, m_color);
    }

    void RenderUtility::DrawTextToScreen(Vec2Int screenPoint, const std::string &text, int font_size)
    {
        RoninEngine::UI::Render_String_ttf(text.data(), font_size, screenPoint, false);
    }
} // namespace RoninEngine::Runtime
