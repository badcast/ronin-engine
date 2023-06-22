#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine::Runtime
{
    extern int _matrix_overflow_;
    extern World* switched_world;

    Vec2Int Matrix::matrix_get_key(Vec2 position) { return Vec2::round_to_int(position) * _matrix_overflow_; }

    void Matrix::matrix_nature(Transform* target, Vec2Int lastPoint) { Matrix::matrix_nature(target, Matrix::matrix_get_key(target->position()), lastPoint); }

    // THIS is matrix get from world space
    void Matrix::matrix_nature(Transform* target, const Vec2Int& newPoint, const Vec2Int& lastPoint)
    {
        if (newPoint == lastPoint)
            return;

        // 1. delete last point source
        auto iter = switched_world->internal_resources->matrix.find(lastPoint);

        if (std::end(switched_world->internal_resources->matrix) != iter) {
            iter->second.erase(target);
            /*
                        // 2. erase empty matrix element
                        if (iter->second.empty())
                            internal_resources->matrixWorld.erase(iter);
            */
        }

        // 3. add point to new source
        switched_world->internal_resources->matrix[newPoint].insert(target);
    }

    void Matrix::matrix_nature_pickup(Runtime::Transform* target)
    {
        auto iter = switched_world->internal_resources->matrix.find(Matrix::matrix_get_key(target->position()));

        if (std::end(switched_world->internal_resources->matrix) != iter) {
            iter->second.erase(target);
            // erase empty matrix element
            if (iter->second.empty())
                switched_world->internal_resources->matrix.erase(iter);
        }
    }

}
