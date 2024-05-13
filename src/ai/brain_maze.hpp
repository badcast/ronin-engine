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
        //  + - - - - - - - - - +   --   It's storm cast view
        //  ' → → → → → → → → ↓ '   --   It's storm cast view
        //  ' ↑ → → → → → → ↓ ↓ '   --   It's storm cast view
        //  ' ↑ ↑ → → → → ↓ ↓ ↓ '   --   It's storm cast view
        //  ' ↑ ↑ ↑ → → ↓ ↓ ↓ ↓ '   --   It's storm cast view
        //  ' ↑ ↑ ↑ ↑ ← ↓ ↓ ↓ ↓ '   --   It's storm cast view
        //  ' ↑ ↑ ↑ ← ← ← ↓ ↓ ↓ '   --   It's storm cast view
        //  ' ↑ ↑ ← ← ← ← ← ↓ ↓ '   --   It's storm cast view
        //  ' ↑ ← ← ← ← ← ← ← ↓ '   --   It's storm cast view
        //  ' ← ← ← ← ← ← ← ← ← '   --   It's storm cast view
        //  + - - - - - - - - - +   --   It's storm cast view
        constexpr int nvec = 4;
        static const ISite storm_vec[nvec] {
            /*X  Y*/
            {-1, 0}, // ←
            {0, 1},  // ↑
            {1, 0},  // →
            {0, -1}  // ↓
        };

        int section, s, iv, x, y, dx, dy, edges = static_cast<int>(map->size());

        if(map->contains(start))
        {
            map->set_lock(start, map->random_number(0, 1000) < 250);
        }
        // Edge
        for(section = 0; edges-- > 0;)
        {
            // Line
            for(iv = 0; iv < nvec; ++iv)
            {
                // Section
                if(!(iv & 1))
                    ++section;

                s = 0;
                do
                {
                    start += storm_vec[iv];

                    if(map->contains(start))
                    {
                        map->set_lock(start, map->random_number(0, 1000) < 250);
                    }
                } while(++s < section);
            }
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
                iv = 0;
                for(auto &p : neighbours)
                {
                    if(iv = map->has_lock(p))
                    {
                        break;
                    }
                }

                if(!iv)
                    map->set_lock(site, false);
            }
        }
    }

    BRAIN_TEMPLATE
    static void maze_forced_chaos(BRAIN_DEFINE *map, ISite startOf, ISite endOf)
    {
        int lbits, rbits, segbits;
        std::uint32_t x, y, dx = map->get_width(), dy = map->get_height();
        std::div_t locked_data;
        decltype(map->get_neighbours(MatrixIdentity::PlusMethod, startOf)) neighbours;

        map->fill_locks(false);

        locked_data = std::div(std::max<std::uint32_t>(dx * dy, ByteSize), ByteSize);
        rbits = segbits = locked_data.quot + locked_data.rem;

        do
        {
            // get value from random function
            lbits = static_cast<std::uint32_t>(map->random_number(0, std::numeric_limits<int>::max()));
            memcpy(reinterpret_cast<void *>(reinterpret_cast<std::size_t>(map->data()) + segbits - rbits), &lbits, std::min(rbits, static_cast<int>(sizeof(lbits))));
            rbits -= std::min(rbits, static_cast<int>(sizeof(lbits)));
        } while(rbits > 0);

        // clear only one point (Wall only randomized)
        dx /= 2;
        dy /= 2;
        for(x = 0; x < dx; ++x)
        {
            for(y = 0; y < dy; ++y)
            {
                ISite site {static_cast<int>(x) + 1, static_cast<int>(y) + 1};

                if(!map->has_lock(site))
                    continue;

                neighbours = map->get_neighbours(MatrixIdentity::PlusMethod, site);
                bool catched = false;
                for(auto &p : neighbours)
                {
                    if(catched = map->has_lock(p))
                    {
                        break;
                    }
                }

                if(!catched)
                    map->set_lock(site, false);
            }
        }
        // Start
        neighbours = map->get_neighbours(MatrixIdentity::DiagonalMethod, startOf);
        for(auto &p : neighbours)
            map->set_lock(p, false);
        // End
        neighbours = map->get_neighbours(MatrixIdentity::DiagonalMethod, endOf);
        for(auto &p : neighbours)
            map->set_lock(p, false);
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
            maze_system::maze_storm_space(this, {static_cast<int>(_xsize / 2), static_cast<int>(_ysize / 2)});
            break;
        case MazeAlgorithm::ForcedChaos:
            maze_system::maze_forced_chaos(this, startOf, endOf);
    }
}

#endif
