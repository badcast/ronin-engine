#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine::Runtime
{
    extern int _matrix_threshold_;

    namespace Matrix
    {
        MatrixKey matrix_get_key(Vec2 position)
        {
            return Vec2::RoundToInt(position * _matrix_threshold_);
        }

        void matrix_nature(Transform *target, MatrixKey lastPoint)
        {
            matrix_nature(target, matrix_get_key(target->position()), lastPoint);
        }

        // THIS is matrix get from world space
        void matrix_nature(Transform *target, const MatrixKey &newPoint, const MatrixKey &lastPoint)
        {
            // Ignore are not changed
            if(newPoint == lastPoint)
                return;

            // Delete last point source
            auto iter = switched_world->internal_resources->matrix.find(lastPoint);
            if(std::end(switched_world->internal_resources->matrix) != iter)
            {
                iter->second.erase(target);
            }

            // Add point to new source
            switched_world->internal_resources->matrix[newPoint].insert(target);
        }

        bool matrix_nature_pickup(Transform *target)
        {
            bool result;
            auto iter = switched_world->internal_resources->matrix.find(matrix_get_key(target->position()));

            if(result = (std::end(switched_world->internal_resources->matrix) != iter))
            {
                iter->second.erase(target);
            }
            return result;
        }

    } // namespace Matrix
} // namespace RoninEngine::Runtime
