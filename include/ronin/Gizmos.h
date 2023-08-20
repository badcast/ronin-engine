#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Gizmos
    {
    public:
        static float angle;
        static Color get_color();
        static void set_color(const Color &newColor);

        static void draw_line(Vec2 a, Vec2 b);
        static void draw_line_pivot(Vec2 ared, Vec2 bblue);
        static void draw_position(const Vec2 &origin, float scalar);
        static void draw_square(Vec2 origin, float width);
        static void draw_rectangle(Vec2 origin, float width, float height);
        static void draw_square_rounded(Vec2 origin, float width, std::uint16_t radius);
        static void draw_rectangle_rounded(Vec2 origin, float width, float height, std::uint16_t radius);
        static void draw_2D_world_space(const Vec2 &origin = Vec2::zero, int depth = 32);
        static void draw_nav_mesh(AI::NavMesh *mesh);
        static void draw_triangle(Vec2 origin, float base, float height, bool fill = false);
        static void draw_text(Vec2 origin, const std::string &text);
        static void draw_text_legacy(Vec2 origin, const std::string &text);
        static void draw_circle(Vec2 origin, float distance);
        static void draw_arrow(Vec2 origin, Vec2 dir, float tailLength = 0.5f);

        static void draw_fill_rect(Vec2 origin, float width, float height);
        static void draw_fill_rect_rounded(Vec2 origin, float width, float height, std::uint16_t radius);
        static void draw_fill_square(Vec2 origin, float width);
        static void draw_fill_square_rounded(Vec2 origin, float width, std::uint16_t radius);

        static void draw_text_to_screen(Vec2Int screen_point, const std::string &text, int font_size = 11);

        //@origin offset point
        //@edges count edge
        //@delim delim > 1 = scale mode, for inverse use negative value
        static void draw_storm(Vec2 origin, int edges, int delim = -1);
    };
} // namespace RoninEngine::Runtime
