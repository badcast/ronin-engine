#include "ronin.h"

namespace RoninEngine::Runtime
{

    void internal_drawLine(Vec2 a, Vec2 b)
    {
        if(!Camera::mainCamera())
            return;

        Vec2 p = Camera::mainCamera()->transform()->position();
        Vec2 dst;

        // dst.x = ((rect.w - dst.w) / 2.0f - (point->x + sourcePoint->x) *
        // squarePerPixels); dst.y = ((rect.h - dst.h) / 2.0f + (point->y -
        // sourcePoint->y) * squarePerPixels);

        // scalars

        Vec2 scale;
        SDL_RenderGetScale(RoninEngine::renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        dst.x = active_resolution.width / 2.f;
        dst.y = active_resolution.height / 2.f;
        a.x = dst.x - (p.x - a.x) * scale.x;
        a.y = dst.y + (p.y - a.y) * scale.y;
        b.x = dst.x - (p.x - b.x) * scale.x;
        b.y = dst.y + (p.y - b.y) * scale.y;

        SDL_RenderDrawLineF(RoninEngine::renderer, a.x, a.y, b.x, b.y);
    }

    Color Gizmos::GetColor()
    {
        Color clb;
        SDL_GetRenderDrawColor(RoninEngine::renderer, &clb.r, &clb.g, &clb.b, &clb.a);
        return clb;
    }

    void Gizmos::SetColor(const Color &newColor)
    {
        SDL_SetRenderDrawColor(RoninEngine::renderer, newColor.r, newColor.g, newColor.b, newColor.a);
    }

    void Gizmos::DrawLine(Vec2 a, Vec2 b)
    {
        internal_drawLine(a, b);
    }

    void Gizmos::DrawLineWithPivot(Vec2 ared, Vec2 bblue)
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

        a.x = pixelsPerPoint;
        a.y = pixelsPerPoint;
        b.x = -pixelsPerPoint;
        b.y = pixelsPerPoint;
        internal_drawLine(a, b);

        a.x = pixelsPerPoint;
        a.y = -pixelsPerPoint;
        b.x = -pixelsPerPoint;
        b.y = -pixelsPerPoint;
        internal_drawLine(a, b);

        a.x = -pixelsPerPoint;
        a.y = pixelsPerPoint;
        b.x = -pixelsPerPoint;
        b.y = -pixelsPerPoint;
        internal_drawLine(a, b);

        a.x = pixelsPerPoint;
        a.y = pixelsPerPoint;
        b.x = pixelsPerPoint;
        b.y = -pixelsPerPoint;
        internal_drawLine(a, b);
    }

    void Gizmos::DrawPosition(const Vec2 &origin, float scalar)
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

    void Gizmos::DrawSquare(Vec2 origin, float width)
    {
        DrawRectangle(origin, width, width);
    }

    void Gizmos::DrawRectangle(Vec2 origin, float width, float height)
    {
        origin = Camera::WorldToScreenPoint(origin);
        std::uint16_t x, y;
        width *= pixelsPerPoint;
        height *= pixelsPerPoint;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        rectangleColor(RoninEngine::renderer, x, y, x + width, y + height, GetColor());
    }

    void Gizmos::DrawRectangleRotate(Vec2 origin, Vec2 size, float angleRadian)
    {
        float cos, sin;
        int i, dx, dy;
        SDL_Point vertices[4];

        cos = Math::Cos(angleRadian);
        sin = Math::Sin(angleRadian);

        origin = Camera::WorldToScreenPoint(origin);
        size *= pixelsPerPoint;
        origin -= size / 2;
        int cx = origin.x + size.x / 2;
        int cy = origin.y + size.y / 2;

        for(i = 0; i < 4; ++i)
        {
            dx = (i == 1 || i == 2) ? origin.x + size.x : origin.x;
            dy = (i == 2 || i == 3) ? origin.y + size.y : origin.y;
            vertices[i].x = cx + (dx - cx) * cos - (dy - cy) * sin;
            vertices[i].y = cy + (dx - cx) * sin + (dy - cy) * cos;
        }

        for(i = 0; i < 4; ++i)
        {
            dx = (i + 1) % 4;
            SDL_RenderDrawLine(renderer, vertices[i].x, vertices[i].y, vertices[dx].x, vertices[dx].y);
        }
    }

    void Gizmos::DrawSquareRounded(Vec2 origin, float width, std::uint16_t radius)
    {
        DrawRectangleRounded(origin, width, width, radius);
    }

    void Gizmos::DrawRectangleRounded(Vec2 origin, float width, float height, std::uint16_t radius)
    {
        origin = Camera::WorldToScreenPoint(origin);
        std::uint16_t x, y;
        width *= pixelsPerPoint;
        height *= pixelsPerPoint;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        roundedRectangleColor(RoninEngine::renderer, x, y, x + width, y + height, radius, GetColor());
    }

    void Gizmos::Draw2DWorldSpace(const Vec2 &origin, int depth)
    {
        int i;
        Vec2 dest1, dest2;
        dest1 = dest2 = origin;
        Color lastColor = GetColor();
        SetColor(0xaa575757);

        // Draw H and V position
        DrawPosition(std::move(origin), maxWorldScalar);

        for(i = 0; i < depth; ++i)
        {
            dest1 += Vec2::one;
            DrawPosition(std::move(dest1), maxWorldScalar);

            dest2 += Vec2::minusOne;
            DrawPosition(std::move(dest2), maxWorldScalar);
        }

        SetColor(lastColor);
    }

    void Gizmos::DrawNavMesh(AI::NavMesh *navMesh)
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
        navMesh->get(Camera::ScreenToWorldPoint(Vec2(active_resolution.width, active_resolution.height)), p2);

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

    void Gizmos::DrawTriangle(Vec2 origin, float base, float height, bool fill)
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

    void Gizmos::DrawText(Vec2 origin, const std::string &text)
    {
        origin = Camera::WorldToScreenPoint(origin);
        RoninEngine::UI::draw_font_at(RoninEngine::renderer, text, 11, Vec2::RoundToInt(origin), GetColor(), true);
    }

    void Gizmos::DrawTextLegacy(Vec2 origin, const std::string &text)
    {
        Rect r;
        // relative to
        origin = Camera::WorldToScreenPoint(origin);
        r.x = static_cast<int>(origin.x);
        r.y = static_cast<int>(origin.y);

        UI::render_string_legacy(r, text.c_str(), text.length(), 2);
    }

    void Gizmos::DrawCircle(Vec2 origin, float distance)
    {
        origin = Camera::WorldToScreenPoint(origin);
        std::uint16_t x, y, r;
        x = Math::Number(origin.x);
        y = Math::Number(origin.y);
        r = static_cast<std::uint16_t>(distance * pixelsPerPoint);
        Color m_color = GetColor();

        circleRGBA(RoninEngine::renderer, x, y, r, m_color.r, m_color.g, m_color.b, m_color.a);
    }

    void Gizmos::DrawArrow(Vec2 origin, Vec2 dir, float tailLength)
    {
        // tail
        tailLength = Math::Abs(tailLength);
        Vec2 pos2 = origin + (dir - origin) * tailLength; // world to local and set point to dir

        internal_drawLine(origin, pos2);
        float angle = Vec2::Angle(pos2, dir);

        origin = pos2 + Vec2::RotateAround(pos2, Vec2::one * tailLength, angle);
        internal_drawLine(pos2, origin);
    }

    void Gizmos::DrawFillRect(Vec2 center, float width, float height)
    {
        center = Camera::WorldToScreenPoint(center);
        width *= pixelsPerPoint;
        height *= pixelsPerPoint;
        Rectf rect {center.x - width / 2, center.y - height / 2, width, height};

        SDL_RenderFillRectF(RoninEngine::renderer, reinterpret_cast<SDL_FRect *>(&rect));
    }

    void Gizmos::DrawFillRectRounded(Vec2 origin, float width, float height, uint16_t radius)
    {
        std::uint16_t x, y;
        origin = Camera::WorldToScreenPoint(origin);
        width *= pixelsPerPoint;
        height *= pixelsPerPoint;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        roundedBoxColor(RoninEngine::renderer, x, y, x + width, y + height, radius, GetColor());
    }

    void Gizmos::DrawFillSquare(Vec2 origin, float width)
    {
        origin = Camera::WorldToScreenPoint(origin);
        width *= pixelsPerPoint;
        Rectf rect {origin.x - width / 2, origin.y - width / 2, width, width};

        SDL_RenderFillRectF(RoninEngine::renderer, reinterpret_cast<SDL_FRect *>(&rect));
    }

    void Gizmos::DrawFillSquareRounded(Vec2 origin, float width, std::uint16_t radius)
    {
        DrawFillRectRounded(origin, width, width, radius);
    }

    void Gizmos::DrawFillCircle(Vec2 origin, float distance)
    {
        origin = Camera::WorldToScreenPoint(origin);
        std::uint16_t x, y, r;
        x = Math::Number(origin.x);
        y = Math::Number(origin.y);
        r = static_cast<std::uint16_t>(distance * pixelsPerPoint);
        Color m_color = GetColor();

        filledCircleColor(RoninEngine::renderer, x, y, r, m_color);
    }

    void Gizmos::DrawTextToScreen(Vec2Int screen_point, const std::string &text, int font_size)
    {
        RoninEngine::UI::draw_font_at(RoninEngine::renderer, text, font_size, screen_point, GetColor(), false);
    }
} // namespace RoninEngine::Runtime
