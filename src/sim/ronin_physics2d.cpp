#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine::Runtime
{

    /*       This is projection: Algorithm storm member used.
             + - - - - - - - - - +
             ' → → → → → → → → ↓ '
             ' ↑ → → → → → → ↓ ↓ '
             ' ↑ ↑ → → → → ↓ ↓ ↓ '
             ' ↑ ↑ ↑ → → ↓ ↓ ↓ ↓ '
             ' ↑ ↑ ↑ ↑ ← ↓ ↓ ↓ ↓ '
             ' ↑ ↑ ↑ ← ← ← ↓ ↓ ↓ '
             ' ↑ ↑ ← ← ← ← ← ↓ ↓ '
             ' ↑ ← ← ← ← ← ← ← ↓ '
             ' ← ← ← ← ← ← ← ← ← '
             + - - - - - - - - - +
    */
    template <bool foreach>
    constexpr inline void storm_cast_eq_t(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate)
    {
        constexpr std::int8_t storm_vec[][2] {
            /*X  Y*/
            {-1, 0}, // ←
            {0, 1},  // ↑
            {1, 0},  // →
            {0, -1}  // ↓
        };
        constexpr int nvec = sizeof(storm_vec) / sizeof(storm_vec[0]);

        int nsection, iv;

        predicate(origin);

        // Edge
        for(nsection = 0; edges-- > 0;)
        {
            // Line
            for(iv = 0; iv < nvec; ++iv)
            {
                // Section
                if(!(iv & 1))
                    ++nsection;

                if constexpr(foreach)
                {
                    int s = 0;
                    do
                    {
                        origin.x += storm_vec[iv][0];
                        origin.y += storm_vec[iv][1];
                        predicate(origin);
                    } while(++s < nsection);
                }
                else
                {
                    origin.x += storm_vec[iv][0] * nsection;
                    origin.y += storm_vec[iv][1] * nsection;
                    predicate(origin);
                }
            }
        }
    }

    void storm_cast_eq_all(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate)
    {
        storm_cast_eq_t<true>(origin, edges, predicate);
    }

    void storm_cast_eq_edges(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate)
    {
        storm_cast_eq_t<false>(origin, edges, predicate);
    }

    template <typename container_result, typename Pred>
    container_result storm_cast_vec_eq(Vec2 origin, int edges, int layer, Pred predicate)
    {
        container_result container;

        storm_cast_eq_t<true>(
            Matrix::matrix_get_key(origin),
            edges,
            [&](const Vec2Int &candidate)
            {
                // unordered_map<int,... <Transform*>>
                for(auto &findedIter : currentWorld->irs->matrix)
                {
                    if(findedIter.first & layer || findedIter.first == layer)
                    {
                        // unordered_map<Vec2Int,...>
                        auto layerObject = findedIter.second.find(candidate);
                        if(layerObject != std::end(findedIter.second))
                        {
                            // set<Transform*>
                            for(auto &object : layerObject->second)
                            {
                                if constexpr(not std::is_same<Pred, std::nullptr_t>::value)
                                {
                                    if(predicate(object->_position) == false)
                                        continue;
                                }
                                if constexpr(std::is_same<container_result, std::set<Transform *>>::value)
                                    container.insert(object);
                                else
                                    container.emplace_back(object);
                            }
                        }
                    }
                }
            });

        return container;
    }

    template <typename container_result>
    container_result Physics2D::GetStormCast(Vec2 origin, int edges, int layer)
    {
        return storm_cast_vec_eq<container_result>(origin, edges, layer, nullptr);
    }

    template <typename container_result>
    container_result Physics2D::GetRectangleCast(Vec2 center, Vec2 size, int layer)
    {
        container_result result;

        size = Vec2::Abs(size) / 2.f;
        Vec2Int leftUpPoint {Matrix::matrix_get_key(center - size)};
        Vec2Int rightDownPoint {Matrix::matrix_get_key(center + size)};
        Vec2Int pointer = leftUpPoint;

        for(; pointer.x <= rightDownPoint.x; ++pointer.x)
        {
            for(pointer.y = leftUpPoint.y; pointer.y <= rightDownPoint.y; ++pointer.y)
            {
                for(auto &findedIter : currentWorld->irs->matrix)
                {
                    if(findedIter.first & layer || findedIter.first == layer)
                    {
                        // unordered_map<Vec2Int,...>
                        auto layerObject = findedIter.second.find(pointer);
                        if(layerObject != std::end(findedIter.second))
                        {
                            // set<Transform*>
                            for(auto &object : layerObject->second)
                            {
                                if constexpr(std::is_same<container_result, std::set<Transform *>>::value)
                                    result.insert(object);
                                else
                                    result.emplace_back(object);
                            }
                        }
                    }
                }
            }
        }

        return result;
    }

    template <typename container_result>
    container_result Physics2D::GetCircleCast(Vec2 origin, float distance, int layer)
    {
        container_result result = storm_cast_vec_eq<container_result>(
            origin,
            Math::Number(Math::Ceil(distance)),
            layer,
            [distance, &origin](Vec2 rhs)
            {
                // condition
                return Vec2::Distance(origin, rhs) <= distance;
            });

        return result;
    }

    template RONIN_API std::list<Transform *> Physics2D::GetStormCast(Vec2, int, int);
    template RONIN_API std::list<Transform *> Physics2D::GetRectangleCast(Vec2, Vec2, int);
    template RONIN_API std::list<Transform *> Physics2D::GetCircleCast(Vec2, float, int);

    template RONIN_API std::vector<Transform *> Physics2D::GetStormCast(Vec2, int, int);
    template RONIN_API std::vector<Transform *> Physics2D::GetRectangleCast(Vec2, Vec2, int);
    template RONIN_API std::vector<Transform *> Physics2D::GetCircleCast(Vec2, float, int);

    template RONIN_API std::set<Transform *> Physics2D::GetStormCast(Vec2, int, int);
    template RONIN_API std::set<Transform *> Physics2D::GetRectangleCast(Vec2, Vec2, int);
    template RONIN_API std::set<Transform *> Physics2D::GetCircleCast(Vec2, float, int);

} // namespace RoninEngine::Runtime

#undef MX
