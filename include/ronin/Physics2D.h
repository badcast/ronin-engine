#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{

    class RONIN_API Physics2D
    {
    public:
        template <typename container_result = std::list<Transform*>>
        static container_result GetStormCast(Vec2 origin, int edges, int layer = -1);
        template <typename container_result = std::list<Transform*>>
        static container_result GetRectangleCast(Vec2 center, Vec2 size, int layer = -1);
        template <typename container_result = std::list<Transform*>>
        static container_result GetCircleCast(Vec2 origin, float distance, int layer = -1);
        
        // Математическая утилита: Спиральный обход (Storm Algorithm) вокруг точки
        // Если fillInside = true, обходит все внутренние ячейки (площадь), иначе только периметр.
        static void SimulateStorm(Vec2Int origin, int edges, std::function<void(const Vec2Int&)> predicate, bool fillInside = true);

        // SAT Oriented Bounding Box collision check
        static bool CheckCollision(Rectf rectA, float angleRadA, Rectf rectB, float angleRadB);
        static bool CheckCollision(Transform *a, Vec2 sizeA, Transform *b, Vec2 sizeB);
    };
} // namespace RoninEngine::Runtime
