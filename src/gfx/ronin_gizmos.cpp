#include "ronin.h"
#include "ronin_render_cache.h"

namespace RoninEngine::Runtime
{

    void internal_drawLine(Vec2 a, Vec2 b)
    {
        if(!Camera::mainCamera())
            return;

        a = Camera::WorldToScreenPoint(a);
        b = Camera::WorldToScreenPoint(b);

        SDL_RenderDrawLineF(gscope.renderer, a.x, a.y, b.x, b.y);
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
        if(!currentWorld || !currentWorld->irs)
            return;

        Vec2 a, b;

        a = currentWorld->irs->metricPixelsPerPoint;
        b = a;
        b.x *= -1;
        internal_drawLine(a, b);

        a = currentWorld->irs->metricPixelsPerPoint;
        a.y *= -1;
        b = currentWorld->irs->metricPixelsPerPoint * -1;
        internal_drawLine(a, b);

        a = currentWorld->irs->metricPixelsPerPoint;
        a.x *= -1;
        b = currentWorld->irs->metricPixelsPerPoint * -1;
        internal_drawLine(a, b);

        a = currentWorld->irs->metricPixelsPerPoint;
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
        if(gscope.renderer == nullptr)
            return;
        origin = Camera::WorldToScreenPoint(origin);
        Vec2 camScale = (currentWorld && currentWorld->irs && currentWorld->irs->mainCamera) ? currentWorld->irs->mainCamera->res->scale : Vec2::one;
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);
        width *= mpp.x * camScale.x;
        height *= mpp.y * camScale.y;
        float x = origin.x - width / 2.f;
        float y = origin.y - height / 2.f;

        Color color = GetColor();
        rectangleRGBA(gscope.renderer, static_cast<Sint16>(x), static_cast<Sint16>(y), static_cast<Sint16>(x + width), static_cast<Sint16>(y + height), color.r, color.g, color.b, color.a);
    }

    void RenderUtility::DrawRectangleRotate(Vec2 origin, Vec2 size, float angleRadian)
    {
        if(gscope.renderer == nullptr)
            return;
        float cos, sin;
        int i;
        float dx, dy, cx, cy;
        Vec2 camScale = (currentWorld && currentWorld->irs && currentWorld->irs->mainCamera) ? currentWorld->irs->mainCamera->res->scale : Vec2::one;
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);

        cos = Math::Cos(angleRadian);
        sin = Math::Sin(angleRadian);

        origin = Camera::WorldToScreenPoint(origin);
        size = Vec2::Scale(size, Vec2::Scale(mpp, camScale));
        cx = origin.x;
        cy = origin.y;
        origin -= size / 2.f;
        size += origin;
        Vec2 vertices[4];
        for(i = 0; i < 4; ++i)
        {
            dx = (i == 1 || i == 2) ? size.x : origin.x;
            dy = (i == 2 || i == 3) ? size.y : origin.y;
            vertices[i].x = cx + (dx - cx) * cos - (dy - cy) * sin;
            vertices[i].y = cy + (dx - cx) * sin + (dy - cy) * cos;
        }

        for(i = 0; i < 4; ++i)
        {
            int next = (i + 1) % 4;
            SDL_RenderDrawLineF(gscope.renderer, vertices[i].x, vertices[i].y, vertices[next].x, vertices[next].y);
        }
    }

    void RenderUtility::DrawSquareRounded(Vec2 origin, float width, std::uint16_t radius)
    {
        DrawRectangleRounded(origin, width, width, radius);
    }

    void RenderUtility::DrawRectangleRounded(Vec2 origin, float width, float height, std::uint16_t radius)
    {
        if(gscope.renderer == nullptr)
            return;
        origin = Camera::WorldToScreenPoint(origin);
        Vec2 camScale = (currentWorld && currentWorld->irs && currentWorld->irs->mainCamera) ? currentWorld->irs->mainCamera->res->scale : Vec2::one;
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);
        width *= mpp.x * camScale.x;
        height *= mpp.y * camScale.y;
        float x = origin.x - width / 2.f;
        float y = origin.y - height / 2.f;

        Color color = GetColor();
        roundedRectangleRGBA(gscope.renderer, static_cast<Sint16>(x), static_cast<Sint16>(y), static_cast<Sint16>(x + width), static_cast<Sint16>(y + height), radius, color.r, color.g, color.b, color.a);
    }

    void RenderUtility::Draw2DWorldSpace(const Vec2 &origin, int depth)
    {
        int i;
        Vec2 dest1, dest2;
        dest1 = dest2 = origin;
        Color lastColor = GetColor();
        SetColor(0xaa575757);

        // Draw H and V position
        DrawPosition(origin, defaultMaxWorldScalar);

        for(i = 0; i < depth; ++i)
        {
            dest1 += Vec2::one;
            DrawPosition(dest1, defaultMaxWorldScalar);

            dest2 += Vec2::minusOne;
            DrawPosition(dest2, defaultMaxWorldScalar);
        }

        SetColor(lastColor);
    }

    void RenderUtility::DrawNavMesh(AI::NavMesh *navMesh)
    {
        using namespace AI;
        Vec2Int p1, p2;
        Color prev;
        Color next;

        Neuron *self;

        // Viewport corner world points
        Vec2 wpTopLeft = Camera::ScreenToWorldPoint(Vec2::zero);
        Vec2 wpBottomRight = Camera::ScreenToWorldPoint(Vec2(gscope.activeResolution.width, gscope.activeResolution.height));

        navMesh->get(wpTopLeft, p1);
        navMesh->get(wpBottomRight, p2);

        int minX = std::min(p1.x, p2.x);
        int maxX = std::max(p1.x, p2.x);
        int minY = std::min(p1.y, p2.y);
        int maxY = std::max(p1.y, p2.y);

        prev = GetColor();
        SetColor(next = 0xfff6f723);

        for(int x = minX; x <= maxX; ++x)
        {
            for(int y = minY; y <= maxY; ++y)
            {
                Vec2Int currentPt(x, y);
                bool draw_pos = true;
                self = navMesh->get(currentPt);
                Vec2 lastPosition = navMesh->PointToWorldPoint(currentPt);
                if(self == nullptr)
                {
                    next.r = 64;
                    next.g = 0;
                    next.b = 0;
                }
                else if(navMesh->hasLocked(currentPt))
                {
                    next.r = 255;
                    next.g = 255;
                    next.b = 255;
                    SetColor(next);

                    // draw line
                    AI::NavListSite neigbours = navMesh->GetNeighbours(AI::NavIdentity::PlusMethod, currentPt);
                    for(int idx = 0; idx < neigbours.size(); ++idx)
                    {
                        Vec2Int *npoint = &neigbours[idx];
                        if(navMesh->hasLocked(*npoint))
                        {
                            internal_drawLine(lastPosition, navMesh->PointToWorldPoint(*npoint));
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
            }
        }

        SetColor(prev);
    }

    void RenderUtility::DrawTriangle(Vec2 origin, float base, float height, bool fill)
    {
        Vec2 offset = Vec2(base / 2, height / 3);

        Vec2 point1 = origin - offset;
        Vec2 point2 = origin + Vec2(base, 0) - offset;
        Vec2 point3 = origin + Vec2(base / 2, height) - offset;

        if(fill)
        {
            if(gscope.renderer == nullptr)
                return;
            Vec2 s1 = Camera::WorldToScreenPoint(point1);
            Vec2 s2 = Camera::WorldToScreenPoint(point2);
            Vec2 s3 = Camera::WorldToScreenPoint(point3);
            Color color = GetColor();
            filledTrigonRGBA(gscope.renderer, static_cast<Sint16>(s1.x), static_cast<Sint16>(s1.y), static_cast<Sint16>(s2.x), static_cast<Sint16>(s2.y), static_cast<Sint16>(s3.x), static_cast<Sint16>(s3.y), color.r, color.g, color.b, color.a);
        }
        else
        {
            internal_drawLine(point1, point2);
            internal_drawLine(point2, point3);
            internal_drawLine(point3, point1);
        }
    }

    void RenderUtility::DrawText(Vec2 origin, const std::string &text)
    {
        using namespace RoninEngine::UI;

        origin = Camera::WorldToScreenPoint(origin);
        Render_String_ttf(text.data(), 11, Vec2::RoundToInt(origin), true);
    }

    void RenderUtility::DrawTextLegacy(Vec2 origin, const std::string &text, bool hilight, bool center)
    {
        Rect r;
        // relative to
        origin = Camera::WorldToScreenPoint(origin);
        r.x = static_cast<int>(origin.x);
        r.y = static_cast<int>(origin.y);

        font2d_string(r, text.c_str(), text.length(), 2, (center ? UI::UIAlign::Center : UI::UIAlign::Left), true, hilight);
    }

    void RenderUtility::DrawTextClassic(Vec2 origin, const std::string &text)
    {
        if(gscope.renderer == nullptr)
            return;
        // relative to
        origin = Camera::WorldToScreenPoint(origin);

        Color color = GetColor();
        stringRGBA(gscope.renderer, origin.x, origin.y, text.c_str(), color.r, color.g, color.b, color.a);
    }

    void RenderUtility::DrawCircle(Vec2 origin, float distance)
    {
        if(gscope.renderer == nullptr)
            return;
        origin = Camera::WorldToScreenPoint(origin);
        Vec2 camScale = (currentWorld && currentWorld->irs && currentWorld->irs->mainCamera) ? currentWorld->irs->mainCamera->res->scale : Vec2::one;
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);
        Sint16 x = static_cast<Sint16>(origin.x);
        Sint16 y = static_cast<Sint16>(origin.y);
        Sint16 r = static_cast<Sint16>(distance * mpp.x * camScale.x);
        Color m_color = GetColor();

        circleRGBA(gscope.renderer, x, y, r, m_color.r, m_color.g, m_color.b, m_color.a);
    }

    void RenderUtility::DrawArrow(Vec2 origin, Vec2 dir, float tailLength, float arrowSize)
    {
        tailLength = Math::Abs(tailLength);
        Vec2 pos2 = origin + (dir - origin) * tailLength; // world to local and set point to dir
        internal_drawLine(origin, pos2);

        float angle = Vec2::Angle(pos2, dir);
        origin = pos2 + Vec2::RotateAround(pos2, Vec2::one * tailLength, angle);
        internal_drawLine(pos2, origin);

        Vec2 arrowEnd1 = Vec2::RotateAround(origin, Vec2(arrowSize, 0), angle + Math::Pi / 4);
        Vec2 arrowEnd2 = Vec2::RotateAround(origin, Vec2(arrowSize, 0), angle - Math::Pi / 4);
        internal_drawLine(origin, arrowEnd1);
        internal_drawLine(origin, arrowEnd2);
    }

    void RenderUtility::DrawSprite(SpriteRef sprite, Vec2 origin, Vec2 size, float angleRadian)
    {
        if(!sprite || !sprite->surface)
            return;

        Rect rect;
        origin = Camera::WorldToScreenPoint(origin);
        Vec2 camScale = (currentWorld && currentWorld->irs && currentWorld->irs->mainCamera) ? currentWorld->irs->mainCamera->res->scale : Vec2::one;
        Vec2 c = sprite->center();

        rect.w = static_cast<int>(sprite->width() * size.x * camScale.x);
        rect.h = static_cast<int>(sprite->height() * size.y * camScale.y);
        rect.x = static_cast<int>(origin.x - rect.w * c.x);
        rect.y = static_cast<int>(origin.y - rect.h * c.y);

        DrawSpriteToScreen(sprite, rect, angleRadian);
    }

    void RenderUtility::DrawSpriteToScreen(SpriteRef sprite, const Rect &rect, float angleRadian)
    {
        SDL_Texture *texture;

        if(gscope.renderer == nullptr || sprite == nullptr || sprite->surface == nullptr)
            return;

        texture = render_cache_texture(sprite.ptr_);
        if(texture)
        {
            Vec2 c = sprite->center();
            SDL_Point rotCenter = { static_cast<int>(rect.w * c.x), static_cast<int>(rect.h * c.y) };
            SDL_RenderCopyEx(gscope.renderer, texture, reinterpret_cast<const SDL_Rect *>(&sprite->m_rect), reinterpret_cast<const SDL_Rect *>(&rect), angleRadian * Math::rad2deg, &rotCenter, SDL_RendererFlip::SDL_FLIP_NONE);
            render_cache_unref(texture);
        }
    }

    void RenderUtility::DrawSpriteExtent(SpriteRef sprite, const Rect &rect, float angleRadian)
    {
        SDL_Texture *texture = render_cache_texture(sprite.ptr_);
        if(texture)
        {
            render_texture_extension(texture, nullptr, reinterpret_cast<SDL_Rect *>(&sprite->m_rect), reinterpret_cast<const SDL_Rect *>(&rect), angleRadian);
            render_cache_unref(texture);
        }
    }

    void RenderUtility::DrawSpriteExtent(SpriteRef sprite, Vec2Int baseSize, const Rect &rect, float angleRadian)
    {
        SDL_Texture *texture = render_cache_texture(sprite.ptr_);
        if(texture)
        {
            render_texture_extension(texture, &baseSize, reinterpret_cast<SDL_Rect *>(&sprite->m_rect), reinterpret_cast<const SDL_Rect *>(&rect), angleRadian);
            render_cache_unref(texture);
        }
    }

    void RenderUtility::DrawFillRect(Vec2 center, float width, float height)
    {
        if(gscope.renderer == nullptr)
            return;
        center = Camera::WorldToScreenPoint(center);
        Vec2 camScale = (currentWorld && currentWorld->irs && currentWorld->irs->mainCamera) ? currentWorld->irs->mainCamera->res->scale : Vec2::one;
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);
        width *= mpp.x * camScale.x;
        height *= mpp.y * camScale.y;
        SDL_FRect rect {center.x - width / 2.f, center.y - height / 2.f, width, height};

        SDL_RenderFillRectF(gscope.renderer, &rect);
    }

    void RenderUtility::DrawFillRectRounded(Vec2 origin, float width, float height, uint16_t radius)
    {
        if(gscope.renderer == nullptr)
            return;
        origin = Camera::WorldToScreenPoint(origin);
        Vec2 camScale = (currentWorld && currentWorld->irs && currentWorld->irs->mainCamera) ? currentWorld->irs->mainCamera->res->scale : Vec2::one;
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);
        width *= mpp.x * camScale.x;
        height *= mpp.y * camScale.y;
        float x = origin.x - width / 2.f;
        float y = origin.y - height / 2.f;

        Color color = GetColor();
        roundedBoxRGBA(gscope.renderer, static_cast<Sint16>(x), static_cast<Sint16>(y), static_cast<Sint16>(x + width), static_cast<Sint16>(y + height), radius, color.r, color.g, color.b, color.a);
    }

    void RenderUtility::DrawFillSquare(Vec2 origin, float width)
    {
        DrawFillRect(origin, width, width);
    }

    void RenderUtility::DrawFillSquareRounded(Vec2 origin, float width, std::uint16_t radius)
    {
        DrawFillRectRounded(origin, width, width, radius);
    }

    void RenderUtility::DrawFillCircle(Vec2 origin, float distance)
    {
        if(gscope.renderer == nullptr)
            return;
        origin = Camera::WorldToScreenPoint(origin);
        Vec2 camScale = (currentWorld && currentWorld->irs && currentWorld->irs->mainCamera) ? currentWorld->irs->mainCamera->res->scale : Vec2::one;
        Vec2 mpp = (currentWorld && currentWorld->irs) ? currentWorld->irs->metricPixelsPerPoint : Vec2(100.0f, 100.0f);
        Sint16 x = static_cast<Sint16>(origin.x);
        Sint16 y = static_cast<Sint16>(origin.y);
        Sint16 r = static_cast<Sint16>(distance * mpp.x * camScale.x);
        Color m_color = GetColor();

        filledCircleRGBA(gscope.renderer, x, y, r, m_color.r, m_color.g, m_color.b, m_color.a);
    }

    void RenderUtility::DrawTextToScreen(Vec2Int screenPoint, const std::string &text, int font_size)
    {
        RoninEngine::UI::Render_String_ttf(text.data(), font_size, screenPoint, false);
    }
} // namespace RoninEngine::Runtime
