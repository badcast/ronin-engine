#pragma once
#include "dependency.h"

namespace RoninEngine::Runtime {
class SHARK Gizmos {
   public:
    static float angle;
    static Color getColor();
    static void setColor(const Color &newColor);

    static void DrawLine(Vec2 a, Vec2 b);
    static void DrawPosition(const Vec2& origin, float scalar = maxWorldScalar);
    static void DrawSquare(Vec2 origin, float width);
    static void DrawRectangle(Vec2 origin, float width, float height);
    static void DrawSquareRounded(Vec2 origin, float width, std::uint16_t radius);
    static void DrawRectangleRounded(Vec2 origin, float width, float height, std::uint16_t radius);
    static void Draw2DWorldSpace(const Vec2& origin = Vec2::zero, int depth = 32);
    static void DrawNavMesh(AIPathFinder::NavMesh* map, bool drawText = true);
    static void DrawTriangle(Vec2 origin, float base, float height, bool fill = false);
    static void DrawTextOnPosition(Vec2 origin, const std::string& text);
    static void DrawTextOnPosition_Legacy(Vec2 origin, const std::string& text);
    static void DrawCircle(Vec2 origin, float distance);

    static void DrawFillRect(Vec2 center, float width, float height);
    static void DrawFillSquare(Vec2 center, float width);

    //@origin offset point
    //@edges count edge
    //@delim delim > 1 = scale mode, for inverse use negative value
    static void DrawStorm(Vec2 origin, int edges, int delim = -1);

    static float square(float x);
    static float square_rectangle(float a, float b);
    static float square_rectangle(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4);
    static float square_circle(float radius);
    static float square_triangle(float base, float height);
    static float square_triangle(Vec2 p1, Vec2 p2, Vec2 p3);
    static float square_mesh(std::list<Vec2>&& vec);
};
}  // namespace RoninEngine::Runtime
