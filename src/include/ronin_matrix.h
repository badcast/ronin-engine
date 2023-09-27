#pragma once

#include "ronin.h"

namespace RoninEngine::Runtime::Matrix
{
    typedef Vec2Int MatrixKey;
    MatrixKey matrix_get_key(Vec2 position);
    void matrix_nature(Transform *target, MatrixKey lastPoint);
    void matrix_nature(Transform *target, const MatrixKey &newPoint, const MatrixKey &lastPoint);
    bool matrix_nature_pickup(Transform *target);
} // namespace RoninEngine::Runtime::Matrix
