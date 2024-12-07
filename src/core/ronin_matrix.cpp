#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime
{
    extern int _matrix_dimensity_;

    namespace Matrix
    {
        matrix_key_t matrix_get_key(Vec2 position)
        {
            return Vec2::RoundToInt(position * _matrix_dimensity_);
        }

        void matrix_update(Transform *target, const matrix_key_t &lastPoint)
        {
            matrix_update(target, matrix_get_key(target->_position), lastPoint);
        }

        // THIS is matrix get from world space
        void matrix_update(Transform *target, const matrix_key_t &newPoint, const matrix_key_t &lastPoint)
        {
            // Ignore are not changed
            if(newPoint == lastPoint)
                return;

            // Delete last point source
            auto findIter = currentWorld->irs->matrix.find(target->_owner->m_layer);
            if(std::end(currentWorld->irs->matrix) != findIter)
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
            currentWorld->irs->matrix[target->_owner->m_layer][newPoint].insert(target);
        }

        bool matrix_remove(Transform *target)
        {
            bool result = false;
            auto findIter = currentWorld->irs->matrix.find(target->_owner->m_layer);
            if(std::end(currentWorld->irs->matrix) != findIter)
            {
                auto layer = findIter->second.find(matrix_get_key(target->_position));
                if(layer != std::end(findIter->second))
                {
                    auto eq = layer->second.find(target);
                    if((result = (eq != std::end(layer->second))))
                        layer->second.erase(eq);
                }
            }
            return result;
        }
    } // namespace Matrix

} // namespace RoninEngine::Runtime
