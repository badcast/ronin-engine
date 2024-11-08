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
    };
} // namespace RoninEngine::Runtime
