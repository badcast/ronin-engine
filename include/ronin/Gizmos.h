#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Gizmos
    {
    public:
        static Color GetColor();
        static void SetColor(const Color &newColor);

        static void DrawLine(Vec2 a, Vec2 b);
        static void DrawLineWithPivot(Vec2 ared, Vec2 bblue);
        static void DrawPosition(const Vec2 &origin, float scalar);
        static void DrawSquare(Vec2 origin, float width);
        static void DrawRectangle(Vec2 origin, float width, float height);
        static void DrawRectangleRotate(Vec2 origin, Vec2 size, float angleRadian);
        static void DrawSquareRounded(Vec2 origin, float width, std::uint16_t radius);
        static void DrawRectangleRounded(Vec2 origin, float width, float height, std::uint16_t radius);
        static void Draw2DWorldSpace(const Vec2 &origin = Vec2::zero, int depth = 32);
        static void DrawNavMesh(AI::NavMesh *navMesh);
        static void DrawTriangle(Vec2 origin, float base, float height, bool fill = false);
        static void DrawText(Vec2 origin, const std::string &text);
        static void DrawTextLegacy(Vec2 origin, const std::string &text);
        static void DrawCircle(Vec2 origin, float distance);
        static void DrawArrow(Vec2 origin, Vec2 dir, float tailLength = 0.5f);

        static void DrawFillRect(Vec2 origin, float width, float height);
        static void DrawFillRectRounded(Vec2 origin, float width, float height, std::uint16_t radius);
        static void DrawFillSquare(Vec2 origin, float width);
        static void DrawFillSquareRounded(Vec2 origin, float width, std::uint16_t radius);
        static void DrawFillCircle(Vec2 origin, float distance);

        static void DrawTextToScreen(Vec2Int screen_point, const std::string &text, int font_size = 11);
    };
} // namespace RoninEngine::Runtime
