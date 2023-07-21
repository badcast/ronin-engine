#include "ronin.h"

namespace RoninEngine::Runtime
{

    extern std::uint32_t const_storm_dimensions;
    extern std::uint32_t const_storm_steps_flag;
    extern std::uint32_t const_storm_xDeterminant;
    extern std::uint32_t const_storm_yDeterminant;
    extern std::uint32_t const_storm_yDeterminant_start;
    extern std::uint32_t const_storm_yDeterminant_inverse;

    float Gizmos::angle;
    void internal_drawLine(Vec2 a, Vec2 b)
    {
        if(!Camera::main_camera())
            return;

        Vec2 p = Camera::main_camera()->transform()->position();
        Vec2 dst;

        Resolution resolution = RoninSimulator::get_current_resolution();

        // dst.x = ((rect.w - dst.w) / 2.0f - (point->x + sourcePoint->x) *
        // squarePerPixels); dst.y = ((rect.h - dst.h) / 2.0f + (point->y -
        // sourcePoint->y) * squarePerPixels);

        // scalars

        Vec2 scale;
        SDL_RenderGetScale(RoninEngine::renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        dst.x = resolution.width / 2.f;
        dst.y = resolution.height / 2.f;
        a.x = dst.x - (p.x - a.x) * scale.x;
        a.y = dst.y + (p.y - a.y) * scale.y;
        b.x = dst.x - (p.x - b.x) * scale.x;
        b.y = dst.y + (p.y - b.y) * scale.y;

        SDL_RenderDrawLineF(RoninEngine::renderer, a.x, a.y, b.x, b.y);
    }

    Color Gizmos::get_color()
    {
        Color clb;
        SDL_GetRenderDrawColor(RoninEngine::renderer, &clb.r, &clb.g, &clb.b, &clb.a);
        return clb;
    }

    void Gizmos::set_color(const Color &newColor)
    {
        SDL_SetRenderDrawColor(RoninEngine::renderer, newColor.r, newColor.g, newColor.b, newColor.a);
    }

    void Gizmos::draw_line(Vec2 a, Vec2 b)
    {
        internal_drawLine(a, b);
    }

    void Gizmos::draw_line_pivot(Vec2 ared, Vec2 bblue)
    {
        Color last = get_color();
        draw_line(ared, bblue);

        set_color(Color::red);
        draw_circle(ared, 0.1f);
        set_color(Color::blue);
        draw_circle(bblue, 0.1f);
        set_color(last);
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

    void Gizmos::draw_position(const Vec2 &origin, float scalar)
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

    void Gizmos::draw_square(Vec2 origin, float width)
    {
        draw_rectangle(origin, width, width);
    }

    void Gizmos::draw_rectangle(Vec2 origin, float width, float height)
    {
        origin = Camera::main_camera()->world_to_screen(origin);
        std::uint16_t x, y;
        width *= pixelsPerPoint;
        height *= pixelsPerPoint;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        rectangleColor(RoninEngine::renderer, x, y, x + width, y + height, get_color());
    }

    void Gizmos::draw_square_rounded(Vec2 origin, float width, std::uint16_t radius)
    {
        draw_rectangle_rounded(origin, width, width, radius);
    }

    void Gizmos::draw_rectangle_rounded(Vec2 origin, float width, float height, std::uint16_t radius)
    {
        origin = Camera::main_camera()->world_to_screen(origin);
        std::uint16_t x, y;
        width *= pixelsPerPoint;
        height *= pixelsPerPoint;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        roundedRectangleColor(RoninEngine::renderer, x, y, x + width, y + height, radius, get_color());
    }

    void Gizmos::draw_2D_world_space(const Vec2 &origin, int depth)
    {
        int i;
        Vec2 dest1, dest2;
        dest1 = dest2 = origin;
        Color lastColor = get_color();
        set_color(0xaa575757);

        // Draw H and V position
        draw_position(std::move(origin), maxWorldScalar);

        for(i = 0; i < depth; ++i)
        {
            dest1 += Vec2::one;
            draw_position(std::move(dest1), maxWorldScalar);

            dest2 += Vec2::minusOne;
            draw_position(std::move(dest2), maxWorldScalar);
        }

        set_color(lastColor);
    }

    void Gizmos::draw_nav_mesh(AI::NavMesh *mesh, bool drawText)
    {
        Vec2 lastPoint;
        AI::Neuron *p;
        Vec2Int p1, p2;
        Color prev;
        Color next;
        int yDefault;
        prev = get_color();
        set_color(next = 0xfff6f723);
        mesh->get(Camera::screen_to_world(Vec2::zero), p1);
        mesh->get(Camera::screen_to_world(Vec2(active_resolution.width - 1, active_resolution.height - 1)), p2);
        yDefault = p1.y;
        while(p1.x <= p2.x)
        {
            while(p1.y <= p2.y)
            {
                p = mesh->get(p1);
                lastPoint = mesh->point_to_world_position(p1);
                if(p == nullptr || mesh->has_locked(p))
                {
                    next.r = 255;
                    next.g = 0;
                    next.b = 0;
                }
                else
                {
                    next.r = 53;
                    next.g = p->f ? 200 : 0;
                    next.b = 246;
                }
                set_color(next);
                draw_position(lastPoint, 0.01f);
                ++p1.y;
            }
            p1.y = yDefault;
            ++p1.x;
        }
        set_color(prev);

        if(drawText)
        {
            static std::uint32_t maxTotal = 0;
            static float upplow = 0;
            if(TimeEngine::time() > upplow)
            {
                maxTotal = 0;
                upplow = TimeEngine::time() + 1;
            }
            std::uint32_t totalC = mesh->get_cache_size();
            maxTotal = std::max(maxTotal, totalC);

            Gizmos::draw_text(
                Camera::screen_to_world(Vec2::zero), "Cached " + std::to_string(totalC) + " (" + std::to_string(maxTotal) + ")");
        }
    }

    void Gizmos::draw_triangle(Vec2 origin, float base, float height, bool fill)
    {
        Vec2 a, b, pivot = origin;
        pivot.y -= height / 2;
        a = b = pivot;
        a.x -= base / 2;
        b.x += base / 2;
        pivot.y += height;

        // draw base
        draw_line(a, b);

        // draw left side
        draw_line(a, pivot);

        // draw right side
        draw_line(b, pivot);

        if(fill)
        {
            base /= 2;
            height /= 2;

            pivot.y = a.y + height / 2;
            draw_fill_rect(pivot, base, height);
        }
    }
    void Gizmos::draw_text(Vec2 origin, const std::string &text)
    {
        origin = Camera::world_to_screen(origin);
        RoninEngine::UI::draw_font_at(RoninEngine::renderer, text, 11, Vec2::round_to_int(origin), get_color(), true);
    }
    void Gizmos::draw_text_legacy(Vec2 origin, const std::string &text)
    {
        Rect r;
        auto cam = Camera::main_camera();
        // relative to
        origin = cam->world_to_screen(origin);
        r.x = static_cast<int>(origin.x);
        r.y = static_cast<int>(origin.y);

        UI::render_string_legacy(r, text.c_str(), text.length(), 2);
    }

    void Gizmos::draw_circle(Vec2 origin, float distance)
    {
        origin = Camera::main_camera()->world_to_screen(origin);
        std::uint16_t x, y, r;
        x = Math::number(origin.x);
        y = Math::number(origin.y);
        r = static_cast<std::uint16_t>(distance * pixelsPerPoint);
        Color m_color = get_color();

        circleRGBA(RoninEngine::renderer, x, y, r, m_color.r, m_color.g, m_color.b, m_color.a);
    }

    void Gizmos::draw_arrow(Vec2 origin, Vec2 dir, float tailLength)
    {
        // tail
        tailLength = Math::abs(tailLength);
        Vec2 pos2 = origin + (dir - origin) * tailLength; // world to local and set point to dir

        internal_drawLine(origin, pos2);
        float angle = Vec2::angle(pos2, dir);

        origin = pos2 + Vec2::rotate_around(pos2, Vec2::one * tailLength, angle);
        internal_drawLine(pos2, origin);
    }

    void Gizmos::draw_fill_rect(Vec2 center, float width, float height)
    {
        center = Camera::world_to_screen(center);
        width *= pixelsPerPoint;
        height *= pixelsPerPoint;
        Rectf rect {center.x - width / 2, center.y - height / 2, width, height};

        SDL_RenderFillRectF(RoninEngine::renderer, reinterpret_cast<SDL_FRect *>(&rect));
    }

    void Gizmos::draw_fill_rect_rounded(Vec2 origin, float width, float height, uint16_t radius)
    {
        origin = Camera::main_camera()->world_to_screen(origin);
        std::uint16_t x, y;
        width *= pixelsPerPoint;
        height *= pixelsPerPoint;
        x = origin.x - width / 2;
        y = origin.y - height / 2;

        roundedBoxColor(RoninEngine::renderer, x, y, x + width, y + height, radius, get_color());
    }

    void Gizmos::draw_fill_square(Vec2 origin, float width)
    {
        origin = Camera::world_to_screen(origin);
        width *= pixelsPerPoint;
        Rectf rect {origin.x - width / 2, origin.y - width / 2, width, width};

        SDL_RenderFillRectF(RoninEngine::renderer, reinterpret_cast<SDL_FRect *>(&rect));
    }

    void Gizmos::draw_fill_square_rounded(Vec2 origin, float width, std::uint16_t radius)
    {
        draw_fill_rect_rounded(origin, width, width, radius);
    }

    void Gizmos::draw_storm(Vec2 ray, int edges, int delim)
    {
        /*
        Описание данных stormMember
        Младшие 4 байта, это все для шаги

        stormMember low bits == steps
        stormMember high bits == maxSteps

        stormFlags = int 8 байта (64 бита)
        первые 3 байта (24 бита) = dimensions, от 0 до 0xFFFFFF значений
        остаток 1 байт (8 бит) stormFlags >> 24 = determinants (определители
        направлений луча) 0xF000000    xDeterminant = stormFlags >> 24 & 0xF -
        горизонтальный детерминант x оси (абцис) 0xF0000000   yDeterminant =
        stormFlags >> 28       - вертикальный детерминант y оси (ординат)
        */

        Vec2 last = ray;
        std::uint64_t stormMember = 0;
        std::uint64_t stormFlags = 1;

        // draw current point

        Color lastColor = get_color();
        int v = 0;
        set_color(0xfff6f6f7);

        if(edges > 0)
            for(;;)
            {

                std::uint32_t steps = static_cast<std::uint32_t>(stormMember & const_storm_steps_flag);
                std::uint32_t maxSteps = static_cast<std::uint32_t>(stormMember >> 32);
                // Шаг заканчивается (step = turnSteps) происходит поворот
                if(steps % std::max(1u, (maxSteps / 4)) == 0)
                {
                    // переход на новое измерение
                    // при steps == maxsteps
                    if(steps == maxSteps)
                    {
                        if(--edges <= -1)
                            break;

                        stormMember = (8ul * (stormFlags = stormFlags & const_storm_dimensions)) << 32;
                        stormFlags = ((stormFlags & const_storm_dimensions) + 1) | const_storm_yDeterminant_start;
                    }
                    else
                    {
                        if(stormFlags >> 28)
                        {
                            stormFlags ^= stormFlags & const_storm_xDeterminant;                                     // clear x
                            stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 4) & const_storm_xDeterminant; // x = y
                            stormFlags ^= stormFlags & const_storm_yDeterminant;                                     // clear y
                        }
                        else
                        {
                            stormFlags ^= stormFlags & const_storm_yDeterminant;                                     // clear y
                            stormFlags |= ((stormFlags & const_storm_xDeterminant) << 4) & const_storm_yDeterminant; // y = x
                            stormFlags ^= const_storm_yDeterminant_inverse;                                          // inverse
                            stormFlags ^= stormFlags & const_storm_xDeterminant;                                     // clear x
                        }
                    }
                }

                char xDeter = static_cast<char>(stormFlags >> 24 & 0xf);
                char yDeter = static_cast<char>(stormFlags >> 28);
                ray.x += xDeter == 2 ? -1 : xDeter;
                ray.y += yDeter == 2 ? -1 : yDeter;

                draw_line(last / delim, ray / delim);
                last = ray;
                ++v;
                if(!(stormMember & const_storm_steps_flag))
                {
                    if(yDeter)
                    {
                        stormFlags ^= stormFlags & const_storm_xDeterminant;                                     // clear x
                        stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 4) & const_storm_xDeterminant; // x = y
                        stormFlags ^= stormFlags & const_storm_yDeterminant;                                     // clear y
                    }
                    else if(xDeter)
                    {
                        stormFlags ^= stormFlags & const_storm_yDeterminant;                                     // clear y
                        stormFlags |= ((stormFlags & const_storm_xDeterminant) << 4) & const_storm_yDeterminant; // y = x
                        stormFlags ^= stormFlags & const_storm_xDeterminant;                                     // clear x
                    }
                }

                ++(*reinterpret_cast<std::uint32_t *>(&stormMember));
            }

        set_color(lastColor);
    }

} // namespace RoninEngine::Runtime
