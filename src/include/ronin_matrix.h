#pragma once

#include "ronin.h"

namespace RoninEngine::Runtime::Matrix
{
    matrix_key_t matrix_get_key(Vec2 position);
    matrix_map_t matrix_make();
    void matrix_update(Transform *target, const matrix_key_t &lastPoint);
    void matrix_update(Transform *target, const matrix_key_t &newPoint, const matrix_key_t &lastPoint);
    bool matrix_remove(Transform *target);
} // namespace RoninEngine::Runtime::Matrix
