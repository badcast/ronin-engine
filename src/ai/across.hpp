/************************************************************************   *** *** *** *** *** *** *** *** ***  ***   *********************
 *                                                                          ***                                  ***
 * ASTAR Algorithm composed to RoninEngine (Navigation AI)                  ***              badcast             ***
 * creation date: 18.01.2023                                                ***                                  ***
 * author: badcast <lmecomposer@gmail.com>                                  *** *** *** *** *** *** *** *** ***  ***
 * C++17 minimum required
 * License: GPLv3
 *
 *  ** see HeuristicMethod
 *  ** basic_across_map::set_heuristic(HeuristicMethod)
 *
 *******************************************************************************************************************************************/

#ifndef _ACROSS_HPP_
#define _ACROSS_HPP_

#include <cstdint>
#include <cmath>
#include <vector>
#include <set>
#include <stdexcept>
#include <cstring>
#include <algorithm>

#define ACROSS_TEMPLATE template <typename ISite, typename INeuron>
#define ACROSS_DEFINE basic_brain_map<ISite, INeuron>

namespace across
{
    enum class NavigationStatus
    {
        Undefined,
        Locked,
        Closed,
        Opened
    };

    enum class MatrixIdentity
    {
        DiagonalMethod = 0,
        PlusMethod = 1,
        CrossMethod = 2
    };

    enum class HeuristicMethod
    {
        Invalid = -1,
        Pythagorean = 0,
        Manhattan = 1,
        Chebyshev = 2,
        Octile = 3
    };

    typedef int weight_t;

    struct the_neuron
    {
        std::uint8_t flags;
        weight_t g;         // - стоимость пути от начальной_точки до текущей_точки
        weight_t h;         // - оценка эвристики (обычно расстояние до конечной_точки)
        weight_t f;         // - общая оценка стоимости (f = g + h)
        the_neuron *parent; // родитель
    };

    struct the_site
    {
        int x;
        int y;
    };

    bool operator==(const the_site &lhs, const the_site &rhs);

    bool operator!=(const the_site &lhs, const the_site &rhs);

    struct brain_breakfast
    {
        weight_t widthSpace;
        weight_t heightSpace;
        int len;
        void *data;
        brain_breakfast() : widthSpace(0), heightSpace(0), len(0), data(nullptr)
        {
        }
    };

    ACROSS_TEMPLATE
    class immune_system;
    ACROSS_TEMPLATE
    class basic_brain_map;

    template <typename IList>
    struct navigate_result
    {
        NavigationStatus status;
        typename IList::value_type from;
        typename IList::value_type to;
        IList connections;
    };

    ACROSS_TEMPLATE
    class basic_brain_map
    {
    public:
        friend class immune_system<ISite, INeuron>;

        template <typename T>
        using basic_list = std::vector<T>;

        using site_type = ISite;
        using neuron_type = INeuron;
        using immune_system = immune_system<ISite, INeuron>;
        using list_site = basic_list<ISite>;
        using list_neuron = basic_list<INeuron *>;
        using result_site = navigate_result<list_site>;
        using result_neuron = navigate_result<list_neuron>;

        explicit basic_brain_map(int xlength, int ylength);

        ~basic_brain_map();

        inline void clear(bool clearLocks = false);
        inline void fill(bool fillLocks = false);
        inline void randomize_hardware(int flagFilter = 0xdeadbeff);
        inline std::size_t size();
        inline int reserved_bits();

        inline bool set_identity(MatrixIdentity identity);

        inline bool set_heuristic(HeuristicMethod method);
        inline HeuristicMethod get_heuristic();

        inline int get_width();
        inline int get_height();

        inline INeuron *get(int x, int y);
        inline INeuron *get(const ISite &point);

        inline bool contains(const ISite &site);
        inline bool contains(const INeuron *neuron);

        inline INeuron *front();
        inline INeuron *back();

        // pointer with point
        inline bool has_lock(const ISite &site);
        inline void set_lock(const ISite &site, const bool lock_state);

        // pointer with pointer
        inline bool has_lock(const INeuron *neuron);
        inline void set_lock(const INeuron *neuron, const bool lock_state);

        inline const ISite get_point(const INeuron *neuron);

        template <typename ListType = result_site>
        bool find(navigate_result<ListType> &navResult, INeuron *firstNeuron, INeuron *lastNeuron);
        template <typename ListType = result_site>
        bool find(navigate_result<ListType> &navResult, const ISite &first, const ISite &last);

        inline void load(const brain_breakfast &);
        inline void save(brain_breakfast *);

        inline std::size_t get_cached_size();

    protected:
        INeuron *neurons;
        weight_t (*__heuristic__)(weight_t dx, weight_t dy);
        int _seg_off;
        int _xsize, _ysize;

        struct
        {
            int length;
            std::int8_t *horizontal;
            std::int8_t *vertical;
            std::int8_t *g_weight;
        } identity;

        void _internal_realloc();
    };

#include "across_impl.hpp"

    typedef basic_brain_map<the_site, the_neuron> Brain;

} // namespace across

#undef ACROSS_TEMPLATE
#undef ACROSS_DEFINE

#endif
