#pragma once

#include "ronin.h"

namespace RoninEngine::Runtime::Matrix
{
    matrix_key_t matrix_get_key(Vec2 position);
    void matrix_update(TransformRef target, const matrix_key_t &lastPoint);
    void matrix_update(TransformRef target, const matrix_key_t &newPoint, const matrix_key_t &lastPoint);
    void matrix_update_r(RendererRef targetRender);
    bool matrix_remove(TransformRef target);
} // namespace RoninEngine::Runtime::Matrix
