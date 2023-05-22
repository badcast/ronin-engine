
#include "ronin.h"

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

    std::list<Transform*> Physics2D::storm_cast(const Vec2& origin, int edges, int layer)
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
        /*        char msg_info[156];

                std::sprintf(msg_info, "origin=(x)%f (y)%f edges=%d layer=%d", origin.x, origin.y, edges, layer);
                Application::show_message(msg_info);
        */
        std::unordered_map<Vec2Int, std::set<Transform*>>& mx = Level::self()->matrixWorld;
        Vec2Int ray = Vec2::round_to_int(origin);
        std::uint64_t stormMember = 0;
        std::uint64_t stormFlags = 1;
        std::list<Transform*> grubbed;

        if (edges > 0)
            for (;;) {
                std::uint32_t steps = (stormMember & const_storm_steps_flag);
                std::uint32_t maxSteps = (stormMember >> 32);

                // Шаг заканчивается (step = turnSteps) происходит поворот
                if (steps % std::max(1u, (maxSteps / 4)) == 0) {
                    // переход на новое измерение
                    // при steps == maxsteps
                    if (steps == maxSteps) {
                        if (--edges <= -1)
                            break;

                        stormMember = (8ul * (stormFlags = stormFlags & const_storm_dimensions)) << 32;
                        stormFlags = ((stormFlags & const_storm_dimensions) + 1) | const_storm_yDeterminant_start;
                    } else {
                        if (stormFlags >> 28) {
                            // stormFlags ^= stormFlags & const_storm_xDeterminant;
                            stormFlags &= ~const_storm_xDeterminant; // clear x
                            stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 4) & const_storm_xDeterminant; // x = y
                            stormFlags &= ~const_storm_yDeterminant; // clear y
                            // stormFlags ^= stormFlags & const_storm_yDeterminant;
                        } else {
                            // stormFlags ^= stormFlags & const_storm_yDeterminant;
                            stormFlags &= ~const_storm_yDeterminant; // clear y
                            stormFlags |= ((stormFlags & const_storm_xDeterminant) << 4) & const_storm_yDeterminant; // y = x
                            stormFlags ^= const_storm_yDeterminant_inverse; // inverse
                            stormFlags &= ~const_storm_xDeterminant; // clear x
                            // stormFlags ^= stormFlags & const_storm_xDeterminant;
                        }
                    }
                }

                // std::sprintf(msg_info, "grubbed=%d steps=%d maxSteps=%d edges=%d const_storm_steps_flag=%d stormMember=%d", grubbed.size(), steps, maxSteps, edges, const_storm_steps_flag, stormMember);
                // Application::show_message(msg_info);

                char xDeter = (stormFlags >> 24 & 0xf);
                char yDeter = stormFlags >> 28;
                auto iter = mx.find(ray);
                if (iter != std::end(mx)) {
                    for (auto x : iter->second)
                        if (x->layer & layer)
                            grubbed.emplace_back(x);
                }
                ray.x += xDeter == 2 ? -1 : xDeter;
                ray.y += yDeter == 2 ? -1 : yDeter;

                if (!(stormMember & const_storm_steps_flag)) {
                    if (yDeter) {
                        // stormFlags ^= stormFlags & const_storm_xDeterminant;
                        stormFlags &= ~const_storm_xDeterminant; // clear x
                        stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 4) & const_storm_xDeterminant; // x = y
                        stormFlags &= ~const_storm_yDeterminant; // clear y
                        // stormFlags ^= stormFlags & const_storm_yDeterminant;
                    } else if (xDeter) {
                        // stormFlags ^= stormFlags & const_storm_yDeterminant;
                        stormFlags &= ~const_storm_yDeterminant; // clear y
                        stormFlags |= ((stormFlags & const_storm_xDeterminant) << 4) & const_storm_yDeterminant; // y = x
                        stormFlags &= ~const_storm_xDeterminant; // clear x
                        // stormFlags ^= stormFlags & const_storm_xDeterminant;
                    }
                }

                ++(*reinterpret_cast<std::uint32_t*>(&stormMember));
            }
        return grubbed;
    }

    std::list<Transform*> Physics2D::rect_cast(Vec2 origin, float distance, int layer)
    {
        auto& mx = Level::self()->matrixWorld;
        std::list<Transform*> _cont;

        Vec2Int env;
        Vec2Int originRounded = Vec2::round_to_int(origin);
        float distanceRounded = Math::ceil(distance);
        for (env.x = originRounded.x - distanceRounded; env.x <= originRounded.x + distanceRounded; ++env.x) {
            for (env.y = originRounded.y - distanceRounded; env.y <= originRounded.y + distanceRounded; ++env.y) {
                auto findedIter = mx.find(env);
                if (findedIter != std::end(mx)) {
                    // filtering by distance
                    for (auto lhs : findedIter->second) {
                        if (Vec2::distance(lhs->p, origin) <= distance)
                            _cont.emplace_back(lhs);
                    }
                }
            }
        }

        return _cont;
    }

    std::list<Transform*> Physics2D::sphere_cast(Vec2 origin, float distance, int layer)
    {
        std::list<Transform*> _cont = storm_cast(origin, Math::number(Math::ceil(distance)), layer);

        _cont.remove_if([&](Transform* lhs) { return Vec2::distance(lhs->p, origin) > distance; });

        return _cont;
    }

} // namespace RoninEngine::Runtime
