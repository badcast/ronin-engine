#pragma once

#include "begin.h"

namespace RoninEngine::Runtime {

class SHARK Physics2D {
   public:
    static std::list<Transform*> stormCast(const Vec2 &origin, int edges, int layer = -1);
    static std::list<Transform*> rectCast(Vec2 origin, float distance);
    static std::list<Transform*> sphereCast(Vec2 origin, float distance, int layer = -1);

};
}  // namespace RoninEngine::Runtime
