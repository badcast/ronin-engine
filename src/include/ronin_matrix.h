#pragma once

#include "ronin.h"

namespace RoninEngine::Runtime::Matrix
{
    Vec2Int matrix_get_key(Vec2 position);
    void matrix_nature(Transform *target, Vec2Int lastPoint);
    void matrix_nature(Transform *target, const Vec2Int &newPoint, const Vec2Int &lastPoint);
    bool matrix_nature_pickup(Transform *target);
} // namespace RoninEngine::Runtime::Matrix
