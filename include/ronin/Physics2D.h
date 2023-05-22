#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{

    class RONIN_API Physics2D
    {
    public:
        static std::list<Transform*> storm_cast(const Vec2& origin, int edges, int layer = -1);
        static std::list<Transform*> rect_cast(Vec2 origin, float distance, int layer = -1);
        static std::list<Transform*> sphere_cast(Vec2 origin, float distance, int layer = -1);
    };
} // namespace RoninEngine::Runtime
