#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine::Runtime
{
    std::uint32_t const_storm_dimensions = 0xFFFFFF
        /* =  = */,
                  const_storm_steps_flag = std::numeric_limits<std::uint32_t>::max()
        /* =  = */,
                  const_storm_xDeterminant = 0xF000000
        /* =  = */,
                  const_storm_yDeterminant = 0xF0000000
        /* =  = */,
                  const_storm_yDeterminant_start = 0x20000000
        /* =  = */,
                  const_storm_yDeterminant_inverse = 0x30000000;

    template <typename container_result, typename Pred>
    container_result storm_cast_eq(Vec2 origin, int edges, int layer, Pred predicate)
    {
        /*
        Описание данных stormMember
        Младшие 4 байта, отвечают за шаги (steps) график использования приведена ниже

        stormMember low bits == steps
        stormMember high bits == maxSteps

        stormFlags = int 4 байта (32 бита)
        первые 3 байта (24 бита) = dimensions, от 0 до 0xFFFFFF значений
        остаток 1 байт (8 бит) stormFlags >> 24 = determinants (определители направлений луча)
        0xF000000    xDeterminant = stormFlags >> 24 & 0xF - горизонтальный детерминант x оси (абцис)
        0xF0000000   yDeterminant = stormFlags >> 28       - вертикальный детерминант y оси (ординат)

        VARIABLE    |   DATA   | bits | DESCRIPTION
        ------------|------------------------------
        stormMember | LOWBITS  |  32  | STEPS
                    | HIGHBITS |  32  | MAXSTEPS
        -------------------------------------------
        stormFlags  | first    |  24  | DIMENSIONS
                    | second   |  8   | DETERMINANTS


               Method finder: Storm
                ' * * * * * * * * *'
                ' * * * * * * * * *'   n = 9
                ' * * * * * * * * *'   n0 (first input point) = 0
                ' * * * 4 5 6 * * *'   n10 (last input point) = 8
                ' * * * 3 0 7 * * *'
                ' * * * 2 1 8 * * *'
                ' * * * * * * * * *'
                ' * * * * * * * * *'
                ' * * * * * * * * *'

        */
        std::unordered_map<Vec2Int, std::set<Transform *>> &mx = World::self()->internal_resources->matrix;
        Vec2Int ray = Matrix::matrix_get_key(origin);
        std::uint64_t stormMember = 0;
        std::uint64_t stormFlags = 1;
        container_result result;

        if(edges > 0)
        {
            for(;;)
            {
                std::uint32_t steps = static_cast<std::uint32_t>(stormMember & const_storm_steps_flag);
                std::uint32_t maxSteps = static_cast<std::uint32_t>(stormMember >> 32);

                // Шаг заканчивается (step = turnSteps) происходит поворот
                if(steps % Math::max(1u, (maxSteps / 4)) == 0)
                {
                    // переход на новое измерение
                    // при steps == maxsteps
                    if(steps == maxSteps)
                    {
                        if(--edges <= -1)
                            break;

                        stormMember = (8ul * (stormFlags = stormFlags & const_storm_dimensions)) << 32;
                        stormFlags = ((stormFlags & const_storm_dimensions) + 1) | const_storm_yDeterminant_start;
                    }
                    else
                    {
                        if(stormFlags >> 28)
                        {
                            // stormFlags ^= stormFlags & const_storm_xDeterminant;
                            stormFlags &= ~const_storm_xDeterminant;                                                 // clear x
                            stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 4) & const_storm_xDeterminant; // x = y
                            stormFlags &= ~const_storm_yDeterminant;                                                 // clear y
                            // stormFlags ^= stormFlags & const_storm_yDeterminant;
                        }
                        else
                        {
                            // stormFlags ^= stormFlags & const_storm_yDeterminant;
                            stormFlags &= ~const_storm_yDeterminant;                                                 // clear y
                            stormFlags |= ((stormFlags & const_storm_xDeterminant) << 4) & const_storm_yDeterminant; // y = x
                            stormFlags ^= const_storm_yDeterminant_inverse;                                          // inverse
                            stormFlags &= ~const_storm_xDeterminant;                                                 // clear x
                            // stormFlags ^= stormFlags & const_storm_xDeterminant;
                        }
                    }
                }

                char xDeter = static_cast<char>(stormFlags >> 24 & 0xf);
                char yDeter = static_cast<char>(stormFlags >> 28);

                auto iter = mx.find(ray);
                if(iter != std::end(mx))
                {
                    for(Transform *x : iter->second)
                        if(x->layer & layer)
                        {
                            if constexpr(not std::is_same<Pred, std::nullptr_t>::value)
                            {
                                if(predicate(origin, x->position()) == false)
                                    continue;
                            }
                            if constexpr(std::is_same<container_result, std::set<Transform *>>::value)
                                result.insert(x);
                            else
                                result.emplace_back(x);
                        }
                }
                ray.x += xDeter == 2 ? -1 : xDeter;
                ray.y += yDeter == 2 ? -1 : yDeter;

                if(!(stormMember & const_storm_steps_flag))
                {
                    if(yDeter)
                    {
                        // stormFlags ^= stormFlags & const_storm_xDeterminant;
                        stormFlags &= ~const_storm_xDeterminant;                                                 // clear x
                        stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 4) & const_storm_xDeterminant; // x = y
                        stormFlags &= ~const_storm_yDeterminant;                                                 // clear y
                        // stormFlags ^= stormFlags & const_storm_yDeterminant;
                    }
                    else if(xDeter)
                    {
                        // stormFlags ^= stormFlags & const_storm_yDeterminant;
                        stormFlags &= ~const_storm_yDeterminant;                                                 // clear y
                        stormFlags |= ((stormFlags & const_storm_xDeterminant) << 4) & const_storm_yDeterminant; // y = x
                        stormFlags &= ~const_storm_xDeterminant;                                                 // clear x
                        // stormFlags ^= stormFlags & const_storm_xDeterminant;
                    }
                }

                ++(*reinterpret_cast<std::uint32_t *>(&stormMember));
            }
            return result;
        }
    }

    template <typename container_result>
    container_result Physics2D::rect_cast(Vec2 center, Vec2 size, int layer)
    {
        container_result result;
        auto &mx = World::self()->internal_resources->matrix;

        size = Vec2::abs(size) / 2.f;
        Vec2Int leftUpPoint {Matrix::matrix_get_key(center - size)};
        Vec2Int rightDownPoint {Matrix::matrix_get_key(center + size)};
        Vec2Int pointer = leftUpPoint;

        for(; pointer.x <= rightDownPoint.x; ++pointer.x)
        {
            for(pointer.y = leftUpPoint.y; pointer.y <= rightDownPoint.y; ++pointer.y)
            {
                auto findedIter = mx.find(pointer);
                if(findedIter != std::end(mx))
                {
                    // filtering
                    for(auto lhs : findedIter->second)
                    {
                        if((lhs->layer & layer) == 0x0000)
                            continue;
                        if constexpr(std::is_same<container_result, std::set<Transform *>>::value)
                            result.insert(lhs);
                        else
                            result.emplace_back(lhs);
                    }
                }
            }
        }

        return result;
    }
    template <typename container_result>
    container_result Physics2D::storm_cast(Vec2 origin, int edges, int layer)
    {
        return storm_cast_eq<container_result>(origin, edges, layer, nullptr);
    }

    template <typename container_result>
    container_result Physics2D::sphere_cast(Vec2 origin, float distance, int layer)
    {
        distance *= distance; // Sqr
        container_result result = storm_cast_eq<container_result>(
            origin,
            Math::number(Math::ceil(distance)),
            layer,
            [distance](Vec2 lhs, Vec2 rhs)
            {
                // condition
                return (lhs - rhs).sqr_magnitude() <= distance;
            });
        return result;
    }

    template RONIN_API std::list<Transform *> Physics2D::storm_cast(Vec2, int, int);
    template RONIN_API std::list<Transform *> Physics2D::rect_cast(Vec2, Vec2, int);
    template RONIN_API std::list<Transform *> Physics2D::sphere_cast(Vec2, float, int);

    template RONIN_API std::vector<Transform *> Physics2D::storm_cast(Vec2, int, int);
    template RONIN_API std::vector<Transform *> Physics2D::rect_cast(Vec2, Vec2, int);
    template RONIN_API std::vector<Transform *> Physics2D::sphere_cast(Vec2, float, int);

    template RONIN_API std::set<Transform *> Physics2D::storm_cast(Vec2, int, int);
    template RONIN_API std::set<Transform *> Physics2D::rect_cast(Vec2, Vec2, int);
    template RONIN_API std::set<Transform *> Physics2D::sphere_cast(Vec2, float, int);

} // namespace RoninEngine::Runtime
