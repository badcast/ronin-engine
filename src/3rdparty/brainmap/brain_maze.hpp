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

        // Place walls at odd offsets from (l, t) and passages at even offsets
        int v_count = (rw - 1) / 2;
        int h_count = (rh - 1) / 2;
        if(v_count <= 0 || h_count <= 0)
            return;

        int vs = l + 1 + map->random_number(0, v_count) * 2;
        int hs = t + 1 + map->random_number(0, h_count) * 2;

        // horizontal wall
        for(int x = l; x <= r; ++x)
        {
            if(INeuron *n = map->get(x, hs))
                map->set_lock(n, true);
        }

        // vertical wall
        for(int y = t; y <= b; ++y)
        {
            if(INeuron *n = map->get(vs, y))
                map->set_lock(n, true);
        }

        // Carve passages in 3 of the 4 wall segments at even offsets from (l, t)
        int skip_wall = map->random_number(0, 4);
        if(skip_wall != 0)
        {
            int px = l + map->random_number(0, (vs - 1 - l) / 2 + 1) * 2;
            if(INeuron *n = map->get(px, hs))
                map->set_lock(n, false);
        }
        if(skip_wall != 1)
        {
            int px = vs + 1 + map->random_number(0, (r - (vs + 1)) / 2 + 1) * 2;
            if(INeuron *n = map->get(px, hs))
                map->set_lock(n, false);
        }
        if(skip_wall != 2)
        {
            int py = t + map->random_number(0, (hs - 1 - t) / 2 + 1) * 2;
            if(INeuron *n = map->get(vs, py))
                map->set_lock(n, false);
        }
        if(skip_wall != 3)
        {
            int py = hs + 1 + map->random_number(0, (b - (hs + 1)) / 2 + 1) * 2;
            if(INeuron *n = map->get(vs, py))
                map->set_lock(n, false);
        }

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

        int dx = static_cast<int>(map->get_width());
        int dy = static_cast<int>(map->get_height());
        int max_span = std::max(dx, dy) + 2;
        int section, s, iv, x, y;

        map->clear(true);

        if(map->contains(start))
        {
            map->set_lock(start, map->random_number(0, 1000) < 250);
        }
        // Edge
        for(section = 0; section <= max_span;)
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
                    start.x += storm_vec[iv].x;
                    start.y += storm_vec[iv].y;

                    if(map->contains(start))
                    {
                        map->set_lock(start, map->random_number(0, 1000) < 250);
                    }
                } while(++s < section);
            }
        }

        // clear only one point (Wall only randomized)
        dx /= 2;
        dy /= 2;
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
                    if((iv = map->has_lock(p)))
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
            memcpy(
                reinterpret_cast<void *>(reinterpret_cast<std::size_t>(map->data()) + segbits - rbits),
                &lbits,
                std::min(rbits, static_cast<int>(sizeof(lbits))));
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
        if(map->contains(startOf))
        {
            map->set_lock(startOf, false);
            neighbours = map->get_neighbours(MatrixIdentity::DiagonalMethod, startOf);
            for(auto &p : neighbours)
                map->set_lock(p, false);
        }
        // End
        if(map->contains(endOf))
        {
            map->set_lock(endOf, false);
            neighbours = map->get_neighbours(MatrixIdentity::DiagonalMethod, endOf);
            for(auto &p : neighbours)
                map->set_lock(p, false);
        }
    }

    /**
     * @brief Randomized Prim's Maze Algorithm
     * Builds a spanning tree from start using a randomized frontier list.
     * Produces branching mazes with many short dead ends.
     */
    BRAIN_TEMPLATE
    static void maze_randomized_prim(BRAIN_DEFINE *map, ISite start)
    {
        static brain::brain_identity maze_identity;
        static auto __g_bss__ {brain::immune_system::get_matrix(brain::MatrixIdentity::PlusMethod, maze_identity)};

        if(!map->contains(start))
            start = {0, 0};
        if(!map->contains(start))
            return;

        map->clear(false);
        map->fill_locks(true);

        std::vector<ISite> frontier;
        INeuron *start_neuron = map->get(start);
        start_neuron->flags = brain::NEURON_CAPTURE_CAPITALIZED;
        map->set_lock(start_neuron, false);

        auto add_frontier = [&](const ISite &origin) {
            for(int d = 0; d < maze_identity.length; ++d)
            {
                ISite next {origin.x + maze_identity.horizontal[d] * 2, origin.y + maze_identity.vertical[d] * 2};
                INeuron *n = map->get(next);
                if(n != nullptr && n->flags == brain::NEURON_CAPTURE_RELEASED)
                {
                    n->flags = brain::NEURON_CAPTURE_OPEN_LIST;
                    frontier.push_back(next);
                }
            }
        };

        add_frontier(start);

        ISite visited_neighbors[4];
        while(!frontier.empty())
        {
            int idx = map->random_number(0, static_cast<int>(frontier.size()));
            ISite current = frontier[idx];
            frontier[idx] = frontier.back();
            frontier.pop_back();

            int vcount = 0;
            for(int d = 0; d < maze_identity.length; ++d)
            {
                ISite nb {current.x + maze_identity.horizontal[d] * 2, current.y + maze_identity.vertical[d] * 2};
                INeuron *n = map->get(nb);
                if(n != nullptr && n->flags == brain::NEURON_CAPTURE_CAPITALIZED)
                {
                    visited_neighbors[vcount++] = nb;
                }
            }

            if(vcount > 0)
            {
                ISite chosen = visited_neighbors[map->random_number(0, vcount)];
                ISite wall {(current.x + chosen.x) / 2, (current.y + chosen.y) / 2};
                map->set_lock(wall, false);
                map->set_lock(current, false);
                map->get(current)->flags = brain::NEURON_CAPTURE_CAPITALIZED;
                add_frontier(current);
            }
        }

        map->clear(false);
    }

    /**
     * @brief Randomized Kruskal's Maze Algorithm
     * Merges disjoint sets of cells across randomly ordered walls using Union-Find.
     */
    BRAIN_TEMPLATE
    static void maze_randomized_kruskal(BRAIN_DEFINE *map, ISite start = {0, 0})
    {
        int w = static_cast<int>(map->get_width());
        int h = static_cast<int>(map->get_height());
        int ox = map->contains(start) ? (start.x & 1) : 0;
        int oy = map->contains(start) ? (start.y & 1) : 0;

        int cols = (w - ox + 1) / 2;
        int rows = (h - oy + 1) / 2;
        if(cols <= 0 || rows <= 0)
            return;

        map->clear(false);
        map->fill_locks(true);

        int total_cells = cols * rows;
        std::vector<int> parent(total_cells);
        std::vector<int> rank_vec(total_cells, 0);
        for(int i = 0; i < total_cells; ++i)
            parent[i] = i;

        auto find_set = [&](int v) {
            int root = v;
            while(root != parent[root])
                root = parent[root];
            while(v != root)
            {
                int nxt = parent[v];
                parent[v] = root;
                v = nxt;
            }
            return root;
        };

        auto union_sets = [&](int a, int b) -> bool {
            a = find_set(a);
            b = find_set(b);
            if(a == b)
                return false;
            if(rank_vec[a] < rank_vec[b])
                std::swap(a, b);
            parent[b] = a;
            if(rank_vec[a] == rank_vec[b])
                ++rank_vec[a];
            return true;
        };

        struct kruskal_edge
        {
            int u;
            int v;
            ISite wall;
        };

        std::vector<kruskal_edge> edges;
        edges.reserve(total_cells * 2);

        for(int r = 0; r < rows; ++r)
        {
            for(int c = 0; c < cols; ++c)
            {
                int x = ox + c * 2;
                int y = oy + r * 2;
                map->set_lock({x, y}, false);
                int id = r * cols + c;
                if(c + 1 < cols)
                    edges.push_back({id, id + 1, {x + 1, y}});
                if(r + 1 < rows)
                    edges.push_back({id, id + cols, {x, y + 1}});
            }
        }

        for(int i = static_cast<int>(edges.size()) - 1; i > 0; --i)
        {
            int j = map->random_number(0, i + 1);
            std::swap(edges[i], edges[j]);
        }

        for(const auto &edge : edges)
        {
            if(union_sets(edge.u, edge.v))
            {
                map->set_lock(edge.wall, false);
            }
        }
    }

    /**
     * @brief Eller's Maze Algorithm
     * Row-by-row streaming generation using set tracking in O(W * H) time and O(W) memory.
     */
    BRAIN_TEMPLATE
    static void maze_ellers(BRAIN_DEFINE *map, ISite start = {0, 0})
    {
        int w = static_cast<int>(map->get_width());
        int h = static_cast<int>(map->get_height());
        int ox = map->contains(start) ? (start.x & 1) : 0;
        int oy = map->contains(start) ? (start.y & 1) : 0;

        int cols = (w - ox + 1) / 2;
        int rows = (h - oy + 1) / 2;
        if(cols <= 0 || rows <= 0)
            return;

        map->clear(false);
        map->fill_locks(true);

        std::vector<int> row_set(cols, 0);
        int next_set_id = 1;

        for(int r = 0; r < rows; ++r)
        {
            int y = oy + r * 2;
            bool is_last_row = (r == rows - 1);

            for(int c = 0; c < cols; ++c)
            {
                map->set_lock({ox + c * 2, y}, false);
                if(row_set[c] == 0)
                    row_set[c] = next_set_id++;
            }

            // Horizontal connections
            for(int c = 0; c < cols - 1; ++c)
            {
                if(row_set[c] != row_set[c + 1] && (is_last_row || map->random_number(0, 2) == 0))
                {
                    map->set_lock({ox + c * 2 + 1, y}, false);
                    int target_set = row_set[c + 1];
                    int replacement_set = row_set[c];
                    for(int k = 0; k < cols; ++k)
                    {
                        if(row_set[k] == target_set)
                            row_set[k] = replacement_set;
                    }
                }
            }

            // Vertical connections to next row
            if(!is_last_row)
            {
                std::vector<int> next_row_set(cols, 0);
                std::vector<bool> processed(cols, false);

                for(int c = 0; c < cols; ++c)
                {
                    if(processed[c])
                        continue;

                    int current_set = row_set[c];
                    std::vector<int> members;
                    for(int k = c; k < cols; ++k)
                    {
                        if(row_set[k] == current_set)
                        {
                            processed[k] = true;
                            members.push_back(k);
                        }
                    }

                    int mandatory = members[map->random_number(0, static_cast<int>(members.size()))];
                    for(int idx : members)
                    {
                        if(idx == mandatory || map->random_number(0, 2) == 0)
                        {
                            map->set_lock({ox + idx * 2, y + 1}, false);
                            map->set_lock({ox + idx * 2, y + 2}, false);
                            next_row_set[idx] = current_set;
                        }
                    }
                }

                row_set = std::move(next_row_set);
            }
        }
    }

    /**
     * @brief Hunt-and-Kill Maze Algorithm
     * Combines a depth-first random walk with a grid scan ("hunt" phase) requiring O(1) extra stack space.
     */
    BRAIN_TEMPLATE
    static void maze_hunt_and_kill(BRAIN_DEFINE *map, ISite start)
    {
        static brain::brain_identity maze_identity;
        static auto __g_bss__ {brain::immune_system::get_matrix(brain::MatrixIdentity::PlusMethod, maze_identity)};

        if(!map->contains(start))
            start = {0, 0};
        if(!map->contains(start))
            return;

        int w = static_cast<int>(map->get_width());
        int h = static_cast<int>(map->get_height());
        int ox = start.x & 1;
        int oy = start.y & 1;

        map->clear(false);
        map->fill_locks(true);

        ISite current = start;
        map->get(current)->flags = brain::NEURON_CAPTURE_CAPITALIZED;
        map->set_lock(current, false);

        int hunt_start_y = oy;
        ISite candidates[4];

        for(;;)
        {
            int count = 0;
            for(int d = 0; d < maze_identity.length; ++d)
            {
                ISite nb {current.x + maze_identity.horizontal[d] * 2, current.y + maze_identity.vertical[d] * 2};
                INeuron *n = map->get(nb);
                if(n != nullptr && n->flags != brain::NEURON_CAPTURE_CAPITALIZED)
                {
                    candidates[count++] = nb;
                }
            }

            if(count > 0)
            {
                ISite next = candidates[map->random_number(0, count)];
                map->set_lock({(current.x + next.x) / 2, (current.y + next.y) / 2}, false);
                map->set_lock(next, false);
                map->get(next)->flags = brain::NEURON_CAPTURE_CAPITALIZED;
                current = next;
                continue;
            }

            // Hunt phase
            bool found = false;
            for(int y = hunt_start_y; y < h && !found; y += 2)
            {
                bool row_all_visited = true;
                for(int x = ox; x < w; x += 2)
                {
                    INeuron *cell_n = map->get(x, y);
                    if(cell_n == nullptr || cell_n->flags == brain::NEURON_CAPTURE_CAPITALIZED)
                        continue;

                    row_all_visited = false;
                    int vcount = 0;
                    for(int d = 0; d < maze_identity.length; ++d)
                    {
                        ISite nb {x + maze_identity.horizontal[d] * 2, y + maze_identity.vertical[d] * 2};
                        INeuron *nb_n = map->get(nb);
                        if(nb_n != nullptr && nb_n->flags == brain::NEURON_CAPTURE_CAPITALIZED)
                        {
                            candidates[vcount++] = nb;
                        }
                    }

                    if(vcount > 0)
                    {
                        ISite visited_nb = candidates[map->random_number(0, vcount)];
                        current = {x, y};
                        map->set_lock({(current.x + visited_nb.x) / 2, (current.y + visited_nb.y) / 2}, false);
                        map->set_lock(current, false);
                        cell_n->flags = brain::NEURON_CAPTURE_CAPITALIZED;
                        found = true;
                        break;
                    }
                }
                if(row_all_visited && y == hunt_start_y)
                {
                    hunt_start_y += 2;
                }
            }

            if(!found)
                break;
        }

        map->clear(false);
    }

    /**
     * @brief Binary Tree Maze Algorithm
     * Fast single-pass generator carving either North or West for every cell.
     */
    BRAIN_TEMPLATE
    static void maze_binary_tree(BRAIN_DEFINE *map, ISite start = {0, 0})
    {
        int w = static_cast<int>(map->get_width());
        int h = static_cast<int>(map->get_height());
        int ox = map->contains(start) ? (start.x & 1) : 0;
        int oy = map->contains(start) ? (start.y & 1) : 0;

        map->clear(false);
        map->fill_locks(true);

        for(int y = oy; y < h; y += 2)
        {
            for(int x = ox; x < w; x += 2)
            {
                map->set_lock({x, y}, false);
                bool can_north = (y - 2 >= oy);
                bool can_west = (x - 2 >= ox);

                if(can_north && can_west)
                {
                    if(map->random_number(0, 2) == 0)
                        map->set_lock({x, y - 1}, false);
                    else
                        map->set_lock({x - 1, y}, false);
                }
                else if(can_north)
                {
                    map->set_lock({x, y - 1}, false);
                }
                else if(can_west)
                {
                    map->set_lock({x - 1, y}, false);
                }
            }
        }
    }

    /**
     * @brief Sidewinder Maze Algorithm
     * Row-by-row algorithm grouping horizontal cell runs and carving one North passage per run.
     */
    BRAIN_TEMPLATE
    static void maze_sidewinder(BRAIN_DEFINE *map, ISite start = {0, 0})
    {
        int w = static_cast<int>(map->get_width());
        int h = static_cast<int>(map->get_height());
        int ox = map->contains(start) ? (start.x & 1) : 0;
        int oy = map->contains(start) ? (start.y & 1) : 0;

        map->clear(false);
        map->fill_locks(true);

        for(int y = oy; y < h; y += 2)
        {
            int run_start = ox;
            for(int x = ox; x < w; x += 2)
            {
                map->set_lock({x, y}, false);
                bool at_eastern_boundary = (x + 2 >= w);
                bool at_northern_boundary = (y - 2 < oy);
                bool should_close_out = at_eastern_boundary || (!at_northern_boundary && map->random_number(0, 2) == 0);

                if(should_close_out)
                {
                    if(!at_northern_boundary)
                    {
                        int run_count = (x - run_start) / 2 + 1;
                        int member_x = run_start + map->random_number(0, run_count) * 2;
                        map->set_lock({member_x, y - 1}, false);
                    }
                    run_start = x + 2;
                }
                else
                {
                    map->set_lock({x + 1, y}, false);
                }
            }
        }
    }

    /**
     * @brief Aldous-Broder Maze Algorithm
     * Generates a uniform spanning tree (unbiased perfect maze) via random walk.
     */
    BRAIN_TEMPLATE
    static void maze_aldous_broder(BRAIN_DEFINE *map, ISite start)
    {
        static brain::brain_identity maze_identity;
        static auto __g_bss__ {brain::immune_system::get_matrix(brain::MatrixIdentity::PlusMethod, maze_identity)};

        if(!map->contains(start))
            start = {0, 0};
        if(!map->contains(start))
            return;

        int w = static_cast<int>(map->get_width());
        int h = static_cast<int>(map->get_height());
        int ox = start.x & 1;
        int oy = start.y & 1;
        int cols = (w - ox + 1) / 2;
        int rows = (h - oy + 1) / 2;
        int unvisited = cols * rows;
        if(unvisited <= 0)
            return;

        map->clear(false);
        map->fill_locks(true);

        ISite current = start;
        map->get(current)->flags = brain::NEURON_CAPTURE_CAPITALIZED;
        map->set_lock(current, false);
        --unvisited;

        ISite neighbours[4];
        while(unvisited > 0)
        {
            int count = 0;
            for(int d = 0; d < maze_identity.length; ++d)
            {
                ISite nb {current.x + maze_identity.horizontal[d] * 2, current.y + maze_identity.vertical[d] * 2};
                if(map->contains(nb))
                {
                    neighbours[count++] = nb;
                }
            }

            if(count == 0)
                break;

            ISite next = neighbours[map->random_number(0, count)];
            INeuron *next_n = map->get(next);
            if(next_n->flags != brain::NEURON_CAPTURE_CAPITALIZED)
            {
                next_n->flags = brain::NEURON_CAPTURE_CAPITALIZED;
                map->set_lock({(current.x + next.x) / 2, (current.y + next.y) / 2}, false);
                map->set_lock(next, false);
                --unvisited;
            }
            current = next;
        }

        map->clear(false);
    }

    /**
     * @brief Wilson's Maze Algorithm
     * Generates a uniform spanning tree using Loop-Erased Random Walks.
     */
    BRAIN_TEMPLATE
    static void maze_wilsons(BRAIN_DEFINE *map, ISite start)
    {
        static brain::brain_identity maze_identity;
        static auto __g_bss__ {brain::immune_system::get_matrix(brain::MatrixIdentity::PlusMethod, maze_identity)};

        if(!map->contains(start))
            start = {0, 0};
        if(!map->contains(start))
            return;

        int w = static_cast<int>(map->get_width());
        int h = static_cast<int>(map->get_height());
        int ox = start.x & 1;
        int oy = start.y & 1;

        map->clear(false);
        map->fill_locks(true);

        map->get(start)->flags = brain::NEURON_CAPTURE_CAPITALIZED;
        map->set_lock(start, false);

        ISite neighbours[4];
        for(int y = oy; y < h; y += 2)
        {
            for(int x = ox; x < w; x += 2)
            {
                ISite cell {x, y};
                INeuron *cell_n = map->get(cell);
                if(cell_n == nullptr || cell_n->flags == brain::NEURON_CAPTURE_CAPITALIZED)
                    continue;

                // Loop-erased random walk using INeuron::parent to record next step
                ISite walker = cell;
                while(map->get(walker)->flags != brain::NEURON_CAPTURE_CAPITALIZED)
                {
                    int count = 0;
                    for(int d = 0; d < maze_identity.length; ++d)
                    {
                        ISite nb {walker.x + maze_identity.horizontal[d] * 2, walker.y + maze_identity.vertical[d] * 2};
                        if(map->contains(nb))
                        {
                            neighbours[count++] = nb;
                        }
                    }
                    if(count == 0)
                        break;

                    ISite next = neighbours[map->random_number(0, count)];
                    map->get(walker)->parent = map->get(next);
                    walker = next;
                }

                // Carve the loop-erased path into the maze
                walker = cell;
                while(map->get(walker)->flags != brain::NEURON_CAPTURE_CAPITALIZED)
                {
                    INeuron *w_n = map->get(walker);
                    if(w_n->parent == nullptr)
                        break;
                    ISite next = map->get_point(w_n->parent);
                    w_n->flags = brain::NEURON_CAPTURE_CAPITALIZED;
                    map->set_lock(walker, false);
                    map->set_lock({(walker.x + next.x) / 2, (walker.y + next.y) / 2}, false);
                    walker = next;
                }
            }
        }

        map->clear(false);
    }

    /**
     * @brief Cellular Automata Maze / Cave Algorithm
     * Generates organic cave-like mazes using the 4-5 cellular automaton rule.
     */
    BRAIN_TEMPLATE
    static void maze_cellular_automata(BRAIN_DEFINE *map, ISite startOf, ISite endOf)
    {
        int w = static_cast<int>(map->get_width());
        int h = static_cast<int>(map->get_height());

        map->clear(true);

        // Initial random seed (~42% walls)
        for(int x = 0; x < w; ++x)
        {
            for(int y = 0; y < h; ++y)
            {
                bool is_border = (x == 0 || y == 0 || x == w - 1 || y == h - 1);
                map->set_lock({x, y}, is_border ? true : (map->random_number(0, 100) < 42));
            }
        }

        // Smooth with 4-5 rule using INeuron::flags as temporary buffer
        constexpr int iterations = 4;
        for(int iter = 0; iter < iterations; ++iter)
        {
            for(int x = 1; x < w - 1; ++x)
            {
                for(int y = 1; y < h - 1; ++y)
                {
                    int wall_count = 0;
                    for(int dx = -1; dx <= 1; ++dx)
                    {
                        for(int dy = -1; dy <= 1; ++dy)
                        {
                            if(dx == 0 && dy == 0)
                                continue;
                            if(map->has_lock({x + dx, y + dy}))
                                ++wall_count;
                        }
                    }

                    bool current_wall = map->has_lock({x, y});
                    bool next_wall = (wall_count > 4) ? true : (wall_count < 4 ? false : current_wall);
                    map->get(x, y)->flags = static_cast<std::uint8_t>(next_wall);
                }
            }

            for(int x = 1; x < w - 1; ++x)
            {
                for(int y = 1; y < h - 1; ++y)
                {
                    map->set_lock({x, y}, map->get(x, y)->flags != 0);
                }
            }
        }

        if(map->contains(startOf))
        {
            map->set_lock(startOf, false);
            auto neighbours = map->get_neighbours(MatrixIdentity::DiagonalMethod, startOf);
            for(auto &p : neighbours)
                map->set_lock(p, false);
        }
        if(map->contains(endOf))
        {
            map->set_lock(endOf, false);
            auto neighbours = map->get_neighbours(MatrixIdentity::DiagonalMethod, endOf);
            for(auto &p : neighbours)
                map->set_lock(p, false);
        }

        // Connect disconnected cave chambers so startOf and endOf are mutually reachable
        map->clear(false);
        static brain::brain_identity plus_id;
        static auto __g_plus__ {brain::immune_system::get_matrix(brain::MatrixIdentity::PlusMethod, plus_id)};

        ISite seed = map->contains(startOf) ? startOf : ISite {1, 1};
        if(map->contains(seed))
        {
            map->set_lock(seed, false);
            std::vector<ISite> queue;
            queue.push_back(seed);
            map->get(seed)->flags = brain::NEURON_CAPTURE_CAPITALIZED;

            auto flood_fill = [&]() {
                std::size_t head = 0;
                while(head < queue.size())
                {
                    ISite cur = queue[head++];
                    for(int d = 0; d < plus_id.length; ++d)
                    {
                        ISite nb {cur.x + plus_id.horizontal[d], cur.y + plus_id.vertical[d]};
                        INeuron *nb_n = map->get(nb);
                        if(nb_n != nullptr && !map->has_lock(nb) && nb_n->flags != brain::NEURON_CAPTURE_CAPITALIZED)
                        {
                            nb_n->flags = brain::NEURON_CAPTURE_CAPITALIZED;
                            queue.push_back(nb);
                        }
                    }
                }
            };

            flood_fill();

            auto dig_tunnel = [&](ISite from, ISite to) {
                ISite cur = from;
                while(cur.x != to.x || cur.y != to.y)
                {
                    if(cur.x != to.x && (cur.y == to.y || map->random_number(0, 2) == 0))
                        cur.x += (to.x > cur.x) ? 1 : -1;
                    else
                        cur.y += (to.y > cur.y) ? 1 : -1;

                    if(map->contains(cur))
                    {
                        map->set_lock(cur, false);
                        INeuron *n = map->get(cur);
                        if(n->flags != brain::NEURON_CAPTURE_CAPITALIZED)
                        {
                            n->flags = brain::NEURON_CAPTURE_CAPITALIZED;
                            queue.push_back(cur);
                        }
                    }
                }
                flood_fill();
            };

            for(int x = 1; x < w - 1; ++x)
            {
                for(int y = 1; y < h - 1; ++y)
                {
                    ISite target {x, y};
                    INeuron *tn = map->get(target);
                    if(!map->has_lock(target) && tn->flags != brain::NEURON_CAPTURE_CAPITALIZED)
                    {
                        ISite closest = queue[0];
                        int best_dist = std::abs(closest.x - target.x) + std::abs(closest.y - target.y);
                        for(const auto &pt : queue)
                        {
                            int d = std::abs(pt.x - target.x) + std::abs(pt.y - target.y);
                            if(d < best_dist)
                            {
                                best_dist = d;
                                closest = pt;
                            }
                        }
                        dig_tunnel(closest, target);
                    }
                }
            }

            if(map->contains(endOf) && map->get(endOf)->flags != brain::NEURON_CAPTURE_CAPITALIZED)
            {
                ISite closest = queue[0];
                int best_dist = std::abs(closest.x - endOf.x) + std::abs(closest.y - endOf.y);
                for(const auto &pt : queue)
                {
                    int d = std::abs(pt.x - endOf.x) + std::abs(pt.y - endOf.y);
                    if(d < best_dist)
                    {
                        best_dist = d;
                        closest = pt;
                    }
                }
                dig_tunnel(closest, endOf);
            }
        }

        map->clear(false);
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
    ISite effective_end = endOf;
    if(effective_end.x == 0 && effective_end.y == 0)
    {
        effective_end = {static_cast<int>(_xsize) - 1, static_cast<int>(_ysize) - 1};
    }

    switch(mazeAlgoritm)
    {
        case MazeAlgorithm::RecursiveBacktracer:
            maze_system::maze_recursive_backtrace(this, startOf);
            break;
        case MazeAlgorithm::RecursiveDivison:
        {
            this->clear(true);
            int l = 0, r = static_cast<int>(_xsize) - 1, t = 0, b = static_cast<int>(_ysize) - 1;
            if(startOf.x < endOf.x && startOf.y < endOf.y)
            {
                l = startOf.x;
                r = endOf.x;
                t = startOf.y;
                b = endOf.y;
            }
            else if(startOf.x < startOf.y && endOf.x < endOf.y)
            {
                l = startOf.x;
                r = startOf.y;
                t = endOf.x;
                b = endOf.y;
            }
            maze_system::maze_recursive_division(this, l, r, t, b);
            break;
        }
        case MazeAlgorithm::SelectiveStormChaos:
            maze_system::maze_storm_space(
                this,
                (startOf.x == 0 && startOf.y == 0) ? ISite {static_cast<int>(_xsize / 2), static_cast<int>(_ysize / 2)} : startOf);
            break;
        case MazeAlgorithm::ForcedChaos:
            maze_system::maze_forced_chaos(this, startOf, effective_end);
            break;
        case MazeAlgorithm::RandomizedPrim:
            maze_system::maze_randomized_prim(this, startOf);
            break;
        case MazeAlgorithm::RandomizedKruskal:
            maze_system::maze_randomized_kruskal(this, startOf);
            break;
        case MazeAlgorithm::Ellers:
            maze_system::maze_ellers(this, startOf);
            break;
        case MazeAlgorithm::HuntAndKill:
            maze_system::maze_hunt_and_kill(this, startOf);
            break;
        case MazeAlgorithm::BinaryTree:
            maze_system::maze_binary_tree(this, startOf);
            break;
        case MazeAlgorithm::Sidewinder:
            maze_system::maze_sidewinder(this, startOf);
            break;
        case MazeAlgorithm::AldousBroder:
            maze_system::maze_aldous_broder(this, startOf);
            break;
        case MazeAlgorithm::Wilsons:
            maze_system::maze_wilsons(this, startOf);
            break;
        case MazeAlgorithm::CellularAutomata:
            maze_system::maze_cellular_automata(this, startOf, effective_end);
            break;
    }
}

#endif
