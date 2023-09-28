#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Runtime;

// MatrixLayerComparer - For compare of the layer from Transform component
bool matrix_compare_layer(RoninEngine::Runtime::Transform const *lhs, RoninEngine::Runtime::Transform const *rhs)
{
    /*
        std::int64_t l, r;
        l = Math::Max(lhs->id, 1u) + Math::OutsideNonZero(lhs->_layer_);
        r = Math::Max(rhs->id, 1u) + Math::OutsideNonZero(rhs->_layer_);
        return (l) < (r);
    */

    return (lhs->_layer_) < (rhs->_layer_);
}

namespace RoninEngine::Runtime
{
    extern int _matrix_pack_;
    namespace Matrix
    {
        MatrixKey matrix_get_key(Vec2 position)
        {
            return Vec2::RoundToInt(position * _matrix_pack_);
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
            auto iter = switched_world->irs->matrix.find(lastPoint);
            if(std::end(switched_world->irs->matrix) != iter)
            {
                auto equal_range = iter->second.equal_range(target);
                auto eq = std::find(equal_range.first, equal_range.second, target);
                if(eq != std::end(iter->second))
                    iter->second.erase(eq);
                else
                {
                    int x = 0;
                }
            }

            // Add point to new source
            switched_world->irs->matrix[newPoint].insert(target);
        }

        bool matrix_nature_pickup(Transform *target)
        {
            bool result;
            auto iter = switched_world->irs->matrix.find(matrix_get_key(target->position()));

            if(result = (std::end(switched_world->irs->matrix) != iter))
            {
                auto equal_range = iter->second.equal_range(target);
                auto eq = std::find(equal_range.first, equal_range.second, target);
                if(eq != std::end(iter->second))
                    iter->second.erase(eq);
            }
            return result;
        }
    } // namespace Matrix

} // namespace RoninEngine::Runtime
