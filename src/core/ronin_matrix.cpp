#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine::Runtime
{
    extern int _matrix_overflow_;
    extern World* switched_world;

    Vec2Int Matrix::matrix_get_key(Vec2 position)
    {
        return Vec2::round_to_int(position) * _matrix_overflow_;
    }

    void Matrix::matrix_nature(Transform* target, Vec2Int lastPoint)
    {
        Matrix::matrix_nature(target, Matrix::matrix_get_key(target->position()), lastPoint);
    }

    // THIS is matrix get from world space
    void Matrix::matrix_nature(Transform* target, const Vec2Int& newPoint, const Vec2Int& lastPoint)
    {
        // Ignore are not changed
        if (newPoint == lastPoint)
            return;

        // Delete last point source
        auto iter = switched_world->internal_resources->matrix.find(lastPoint);
        if (std::end(switched_world->internal_resources->matrix) != iter)
        {
            iter->second.erase(target);
        }

        // Add point to new source
        switched_world->internal_resources->matrix[newPoint].insert(target);
    }

    bool Matrix::matrix_nature_pickup(Transform* target)
    {
        bool result;
        auto iter = switched_world->internal_resources->matrix.find(Matrix::matrix_get_key(target->position()));

        if (result = (std::end(switched_world->internal_resources->matrix) != iter))
        {
            iter->second.erase(target);
        }
        return result;
    }

}
