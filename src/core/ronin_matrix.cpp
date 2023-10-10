#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Runtime;

//// MatrixLayerComparer - For compare of the layer from Transform component
// bool matrix_compare_layer(RoninEngine::Runtime::Transform const *lhs, RoninEngine::Runtime::Transform const *rhs)
//{
//     /*
//         std::int64_t l, r;
//         l = Math::Max(lhs->id, 1u) + Math::OutsideNonZero(lhs->_layer_);
//         r = Math::Max(rhs->id, 1u) + Math::OutsideNonZero(rhs->_layer_);
//         return (l) < (r);
//     */

//    return (lhs->_layer_) < (rhs->_layer_);
//}

namespace RoninEngine::Runtime
{
    extern int _matrix_pack_;
    namespace Matrix
    {
        MatrixKey matrix_get_key(Vec2 position)
        {
            return Vec2::RoundToInt(position * _matrix_pack_);
        }

        void matrix_update(Transform *target, MatrixKey lastPoint)
        {
            matrix_update(target, matrix_get_key(target->position()), lastPoint);
        }

        // THIS is matrix get from world space
        void matrix_update(Transform *target, const MatrixKey &newPoint, const MatrixKey &lastPoint)
        {
            // Ignore are not changed
            if(newPoint == lastPoint)
                return;

            // Delete last point source
            auto findIter = switched_world->irs->matrix.find(target->_owner->m_layer);
            if(std::end(switched_world->irs->matrix) != findIter)
            {
                auto layer = findIter->second.find(lastPoint);
                if(layer != std::end(findIter->second))
                {
                    auto eq = layer->second.find(target);
                    if(eq != std::end(layer->second))
                        layer->second.erase(eq);
                }
            }

            // Add point to new source
            switched_world->irs->matrix[target->_owner->m_layer][newPoint].insert(target);
        }

        bool matrix_remove(Transform *target)
        {
            bool result = false;
            auto findIter = switched_world->irs->matrix.find(target->_owner->m_layer);
            if(std::end(switched_world->irs->matrix) != findIter)
            {
                auto layer = findIter->second.find(matrix_get_key(target->position()));
                if(layer != std::end(findIter->second))
                {
                    auto eq = layer->second.find(target);
                    if(result = (eq != std::end(layer->second)))
                        layer->second.erase(eq);
                }
            }
            return result;
        }
    } // namespace Matrix

} // namespace RoninEngine::Runtime
