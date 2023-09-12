// declarations brain_map part-file
#ifndef _BRAIN_MAZE_HPP_
#define _BRAIN_MAZE_HPP_

#include "brain_map.hpp"

#if not defined(BRAIN_TEMPLATE)
#error "main header not included"
#endif

#ifndef BRAIN_TEMPLATE
#define BRAIN_TEMPLATE template <typename ISite, typename INeuron>
#endif

#ifndef BRAIN_DEFINE
#define BRAIN_DEFINE brain::basic_brain_map<ISite, INeuron>
#endif

namespace maze_system
{
    BRAIN_TEMPLATE
    static void maze_recursive_division(BRAIN_DEFINE *map, int l, int r, int t, int b)
    {
        if(r < l || b < t)
            return;

        int rw = r - l;
        int rh = b - t;

        if(rw < 2 || rh < 2)
            return;

        int hs = map->random_number(t + 1, b - 1);
        int vs = map->random_number(l + 1, r - 1);

        // horizontal
        for(int x = l; x <= r; ++x)
        {
            map->set_lock(map->get(x, hs), true);
        }

        // vertical
        for(int y = t; y <= b; ++y)
        {
            map->set_lock(map->get(y, vs), true);
        }

        int dx = map->random_number(l, r);
        int dy = map->random_number(t, b);
        map->set_lock(map->get(dx, dy), false);

        maze_recursive_division(map, l, vs - 1, t, hs - 1);
        maze_recursive_division(map, vs + 1, r, t, hs - 1);
        maze_recursive_division(map, l, vs - 1, hs + 1, b);
        maze_recursive_division(map, vs + 1, r, hs + 1, b);
    }

    BRAIN_TEMPLATE
    static void maze_recursive_backtrace(BRAIN_DEFINE *map, ISite start)
    {
        static brain::brain_identity maze_identity;
        static auto __g_bss__ {brain::immune_system::get_matrix(brain::MatrixIdentity::PlusMethod, maze_identity)};

        int dx, dy;
        INeuron *free_neighbours[4];
        INeuron *current, *self;
        ISite current_site, self_site;
        std::vector<INeuron *> stack;

        if(!map->contains(start))
            return;

        map->clear();
        map->fill_locks(true);

        current = map->get(start);
        current->flags = brain::NEURON_CAPTURE_CAPITALIZED;

        do
        {
            current_site = map->get_point(current);

            // get neighbors
            for(dx = dy = 0; dx < maze_identity.length; ++dx)
            {
                // Get the closest neurons
                self_site.x = current_site.x + maze_identity.horizontal[dx] * 2;
                self_site.y = current_site.y + maze_identity.vertical[dx] * 2;
                self = map->get(self_site);
                if(self == nullptr || self->flags == brain::NEURON_CAPTURE_CAPITALIZED)
                {
                    continue;
                }

                free_neighbours[dy++] = self;
            }

            if(!dy)
            {
                current = stack.back();
                stack.pop_back();
            }
            else
            {

                self = free_neighbours[map->random_number(0, dy)];
                self_site = map->get_point(self);

                // Bottom wall
                if(current_site.x == self_site.x)
                {
                    if(current_site.y > self_site.y)
                    {
                        current_site.y--;
                        if(map->contains(current_site))
                            map->set_lock(current_site, false);
                    }
                    else
                    {
                        self_site.y--;
                        if(map->contains(self_site))
                            map->set_lock(self_site, false);
                    }
                }
                // Left wall
                else
                {
                    if(current_site.x > self_site.x)
                    {
                        current_site.x--;
                        if(map->contains(current_site))
                            map->set_lock(current_site, false);
                    }
                    else
                    {
                        self_site.x--;
                        if(map->contains(self_site))
                            map->set_lock(self_site, false);
                    }
                }

                // locked and visited
                self->flags = brain::NEURON_CAPTURE_CAPITALIZED;
                map->set_lock(self, false);
                stack.push_back(self);
                current = self;
            }
        } while(!stack.empty());
    }

    BRAIN_TEMPLATE
    static void maze_storm_space(BRAIN_DEFINE *map, ISite start)
    {
        constexpr std::uint32_t const_storm_dimensions = 0xFFFFFF;
        constexpr std::uint32_t const_storm_steps_flag = std::numeric_limits<std::uint32_t>::max();
        constexpr std::uint32_t const_storm_xDeterminant = 0xF000000;
        constexpr std::uint32_t const_storm_yDeterminant = 0xF0000000;
        constexpr std::uint32_t const_storm_yDeterminant_start = 0x20000000;
        constexpr std::uint32_t const_storm_yDeterminant_inverse = 0x30000000;

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
        std::uint64_t stormMember = 0;
        std::uint64_t stormFlags = 1;
        std::size_t edges = map->size();
        std::uint32_t x, y, dx = map->get_width() / 2, dy = map->get_height() / 2;

        map->fill_locks(false);

        for(; edges > 0;)
        {
            std::uint32_t steps = static_cast<std::uint32_t>(stormMember & const_storm_steps_flag);
            std::uint32_t maxSteps = static_cast<std::uint32_t>(stormMember >> 32);

            // Шаг заканчивается (step = turnSteps) происходит поворот
            if(steps % std::max(1u, (maxSteps / 4)) == 0)
            {
                // переход на новое измерение
                // при steps == maxsteps
                if(steps == maxSteps)
                {
                    stormMember = (8ul * (stormFlags = stormFlags & const_storm_dimensions)) << 32;
                    stormFlags = ((stormFlags & const_storm_dimensions) + 1) | const_storm_yDeterminant_start;
                }
                else
                {
                    if(stormFlags >> 28)
                    {
                        // stormFlags ^= stormFlags & const_storm_xDeterminant;
                        stormFlags &= ~const_storm_xDeterminant;                                                   // clear x
                        stormFlags |= ((stormFlags & const_storm_yDeterminant) >> 0x4) & const_storm_xDeterminant; // x = y
                        stormFlags &= ~const_storm_yDeterminant;                                                   // clear y
                        // stormFlags ^= stormFlags & const_storm_yDeterminant;
                    }
                    else
                    {
                        // stormFlags ^= stormFlags & const_storm_yDeterminant;
                        stormFlags &= ~const_storm_yDeterminant;                                                   // clear y
                        stormFlags |= ((stormFlags & const_storm_xDeterminant) << 0x4) & const_storm_yDeterminant; // y = x
                        stormFlags ^= const_storm_yDeterminant_inverse;                                            // inverse
                        stormFlags &= ~const_storm_xDeterminant;                                                   // clear x
                        // stormFlags ^= stormFlags & const_storm_xDeterminant;
                    }
                }
            }

            char xDeter = static_cast<char>(stormFlags >> 24 & 0xf);
            char yDeter = static_cast<char>(stormFlags >> 28);

            if(map->contains(start))
            {
                map->set_lock(start, map->random_number(0, 1000) < 250);
                --edges;
            }

            start.x += xDeter == 2 ? -1 : xDeter;
            start.y += yDeter == 2 ? -1 : yDeter;

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

        // clear only one point (Wall only randomized)

        for(x = 0; x < dx; ++x)
        {
            for(y = 0; y < dy; ++y)
            {
                ISite site {x + 1, y + 1};

                if(!map->has_lock(site))
                    continue;

                auto neighbours = map->get_neighbours(MatrixIdentity::PlusMethod, site);
                stormFlags = 0;
                for(auto &p : neighbours)
                {
                    if(stormFlags = map->has_lock(p))
                    {
                        break;
                    }
                }

                if(!stormFlags)
                    map->set_lock(site, false);
            }
        }
    }
} // namespace maze_system

BRAIN_TEMPLATE
void BRAIN_DEFINE::create_maze()
{
    maze_system::maze_recursive_backtrace(this, {0, 0});
}

BRAIN_TEMPLATE
void BRAIN_DEFINE::create_maze_ex(brain::MazeAlgorithm mazeAlgoritm, ISite startOf, ISite endOf)
{
    switch(mazeAlgoritm)
    {
        case MazeAlgorithm::RecursiveBacktracer:
            maze_system::maze_recursive_backtrace(this, startOf);
            break;
        case MazeAlgorithm::RecursiveDivison:
            maze_system::maze_recursive_division(this, startOf.x, startOf.y, endOf.x, endOf.y);
            break;
        case MazeAlgorithm::SelectiveStormChaos:
            maze_system::maze_storm_space(this, {_xsize / 2, _ysize / 2});
            break;
    }
}

#endif
