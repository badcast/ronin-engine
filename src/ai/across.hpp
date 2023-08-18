/************************************************************************   *** *** *** *** *** *** *** *** ***  ***   *********************
 *                                                                          ***                                  ***
 * ASTAR Algorithm composed to RoninEngine (Navigation AI)                  ***         badcast for cast         ***
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
        the_neuron *parent; //
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
        std::uint32_t widthSpace;
        std::uint32_t heightSpace;
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

        explicit basic_brain_map(std::uint32_t xlength, std::uint32_t ylength);

        ~basic_brain_map();

        /**
         * @brief clear neurons
         * @clearLocks also clear locked data
         */
        inline void clear(bool clearLocks = false);

        /**
         * @brief fill
         * @param fillLocks
         */
        inline void fill(bool fillLocks = false);

        /**
         * @brief randomize_hardware the function for randomized data
         * @param flagFilter the filter only source
         */
        inline void randomize_hardware(int flagFilter = 0xdeadbeff);

        /**
         * @brief size of the neurons
         * @return size
         */
        inline std::size_t size();

        /**
         * @brief free bits and not used diaposoned from 0 to 7
         * @return free bits
         */
        inline int reserved_bits();

        /**
         * @brief set the identity
         * @see MatrixIdentity
         * @return accepted identity value
         */
        inline bool set_identity(MatrixIdentity identity);

        /**
         * @brief set the heuristic method
         * @see HeuristicMethod
         * @return accepted heuristic method value
         */
        inline bool set_heuristic(HeuristicMethod method);

        /**
         * @brief get the heuristic method
         * @see HeuristicMethod
         * @return get heuristic method value
         */
        inline HeuristicMethod get_heuristic();

        /**
         * @brief horizontal length of the neurons
         * @return horizontal length
         */
        inline std::uint32_t get_width();

        /**
         * @brief vertical length of the neurons
         * @return vertical length
         */
        inline std::uint32_t get_height();

        /**
         * @brief get the neuron
         * @param x the horizontal point
         * @param y the vertical point
         * @return get neuron pointer
         */
        inline INeuron *get(int x, int y);

        /**
         * @brief get the neuron
         * @param point the neuron site location
         * @return get neuron pointer
         */
        inline INeuron *get(const ISite &point);

        /**
         * @brief contains site location
         * @param site for select
         * @return status contains
         */
        inline bool contains(const ISite &site);

        /**
         * @brief contains neuron
         * @param neuron for select
         * @return status contains
         */
        inline bool contains(const INeuron *neuron);

        /**
         * @brief front neuron (the most first)
         * @return get neuron pointer
         */
        inline INeuron *front();

        /**
         * @brief back neuron (the most last)
         * @return get neuron pointer
         */
        inline INeuron *back();

        /**
         * @brief check location has been locked
         * @param site location for select
         * @return status locked
         */
        inline bool has_lock(const ISite &site);

        /**
         * @brief set lock site location
         * @param site the location for select
         * @param lock_state lock state
         */
        inline void set_lock(const ISite &site, const bool lock_state);

        /**
         * @brief check neuron has been locked
         * @param neuron for select
         * @return status locked
         */
        inline bool has_lock(const INeuron *neuron);

        /**
         * @brief set lock neuron
         * @param site the neuron for select
         * @param lock_state lock state
         */
        inline void set_lock(const INeuron *neuron, const bool lock_state);

        /**
         * @brief convert neuron to location (site) obejct
         * @param select for convertable
         * @return location result
         */
        inline const ISite get_point(const INeuron *neuron);

        /**
         * @brief find neurons for selects
         * @param navResult the result of the selected (for the site result type or the neurons)
         * @param firstNeuron the start of the find to end
         * @param lastNeuron the end of the find to start
         * @return status of the finded
         */
        template <typename ListType = result_site>
        bool find(navigate_result<ListType> &navResult, INeuron *firstNeuron, INeuron *lastNeuron);

        /**
         * @brief find locations for selects
         * @param navResult the result of the selected (for the site result type or the neurons)
         * @param first the start of the find to end
         * @param last the end of the find to start
         * @return status of the finded
         */
        template <typename ListType = result_site>
        bool find(navigate_result<ListType> &navResult, const ISite &first, const ISite &last);

        /**
         * @brief load breakfast
         */
        inline void load(const brain_breakfast &);

        /**
         * @brief save to breakfast
         */
        inline void save(brain_breakfast *);

        /**
         * @brief cached size of the finded last result
         * @return size of the cache
         */
        inline std::size_t get_cached_size();

    protected:
        INeuron *neurons;
        weight_t (*__heuristic__)(weight_t dx, weight_t dy);
        std::uint32_t _seg_off;
        std::uint32_t _xsize, _ysize;

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

    typedef basic_brain_map<the_site, the_neuron> brain_map;

} // namespace across

#undef ACROSS_TEMPLATE
#undef ACROSS_DEFINE

#endif
