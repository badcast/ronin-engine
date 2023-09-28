#pragma once

#include "ronin.h"

namespace RoninEngine::Runtime::Matrix
{
    MatrixKey matrix_get_key(Vec2 position);
    matrix_map_t matrix_make();
    void matrix_nature(Transform *target, MatrixKey lastPoint);
    void matrix_nature(Transform *target, const MatrixKey &newPoint, const MatrixKey &lastPoint);
    bool matrix_nature_pickup(Transform *target);
} // namespace RoninEngine::Runtime::Matrix
