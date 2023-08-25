// declarations brain_map part-file
#ifndef _BRAIN_MAZE_HPP_
#define _BRAIN_MAZE_HPP_

#include "brain_map.hpp"

BRAIN_TEMPLATE
class maze_system
{
    using brain_map = basic_brain_map<ISite, INeuron>;

private:
    static void recursive_division(brain_map *map, int l, int r, int t, int b)
    {
        if(r < l || b < t)
            return;

        int rw = r - l;
        int rh = b - t;

        if(rw < 2 || rh < 2)
            return;

        int hs = random_number(t + 1, b - 1);
        int vs = random_number(l + 1, r - 1);

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

        int dx = random_number(l, r);
        int dy = random_number(t, b);
        map->set_lock(map->get(dx, dy), false);

        recursive_division(map, l, vs - 1, t, hs - 1);
        recursive_division(map, vs + 1, r, t, hs - 1);
        recursive_division(map, l, vs - 1, hs + 1, b);
        recursive_division(map, vs + 1, r, hs + 1, b);
    }

public:
    static int random_number(int min, int max)
    {
        return min + abs(rand()) % (max - min);
    }

    static void maze_recbuf(brain_map *map)
    {

        static struct
        {
            int length;
            std::int8_t *horizontal;
            std::int8_t *vertical;
            std::int8_t *g_weight;
        } maze_identity;
        static auto __g_bss__ {brain_map::immune_system::get_matrix(MatrixIdentity::PlusMethod, maze_identity)};

        INeuron *current, *self;
        ISite current_site, self_site;

        std::vector<INeuron *> stack;
        std::vector<INeuron *> free_neighbours;
        // reserve
        free_neighbours.reserve(maze_identity.length);

        map->clear();
        map->fill_locks(true);

        current = map->front();
        current->flags = NEURON_CAPTURE_CAPITALIZED;
        // stack.push_back(current);
        do
        {
            current_site = map->get_point(current);

            // get neighbors
            for(int s = 0; s < maze_identity.length; ++s)
            {
                // Get the closest neurons
                self_site.x = current_site.x + maze_identity.horizontal[s] * 2;
                self_site.y = current_site.y + maze_identity.vertical[s] * 2;
                self = map->get(self_site);
                if(self == nullptr || self->flags == NEURON_CAPTURE_CAPITALIZED)
                {
                    continue;
                }

                free_neighbours.push_back(self);
            }

            if(free_neighbours.empty())
            {
                current = stack.back();
                stack.pop_back();
            }
            else
            {

                self = free_neighbours[random_number(0, free_neighbours.size())];
                // remove wall
                /*
                 // a = current
                 // b = self
                 if(a.X == b.X)
                 {
                     if(a.Y > b.Y)
                         a.WallBottom = false;
                     else
                         b.WallBottom = false;
                 }
                 else
                 {
                     if(a.X > b.X)
                         a.WallLeft = false;
                     else
                         b.WallLeft = false;
                 }*/
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
                self->flags = NEURON_CAPTURE_CAPITALIZED;
                map->set_lock(self,false);
                stack.push_back(self);
                current = self;
            }

            free_neighbours.clear();
        } while(!stack.empty());
    }
    static void maze_recursive_division(brain_map *map)
    {
        recursive_division(map, 0, map->get_width() - 1, 0, map->get_height() - 1);
    }
};

BRAIN_TEMPLATE
void BRAIN_DEFINE::create_maze()
{
    maze_system::maze_recbuf(this);
}

#endif
