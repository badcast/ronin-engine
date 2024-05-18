/************************************************************************   *** *** *** *** *** *** *** *** ***  ***   *********************
 *                                                                          ***                                  ***
 * ASTAR Algorithm composed to RoninEngine (Navigation AI)                  ***         badcast for cast         ***
 * creation date: 18.01.2023                                                ***                                  ***
 * author: badcast <lmecomposer@gmail.com>                                  *** *** *** *** *** *** *** *** ***  ***
 * C++17 minimum required
 * License: GPLv3
 *
 *  ** see HeuristicMethod
 *  ** basic_brain_map::set_heuristic(HeuristicMethod)
 *
 *******************************************************************************************************************************************/

#ifndef _BRAIN_MAP_HPP_
#define _BRAIN_MAP_HPP_

#include <cstdint>
#include <cmath>
#include <vector>
#include <set>
#include <limits>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <functional>

#define BRAIN_TEMPLATE template <typename ISite, typename INeuron>
#define BRAIN_DEFINE basic_brain_map<ISite, INeuron>

namespace brain
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

    enum class MazeAlgorithm
    {
        RecursiveBacktracer = 0,
        RecursiveDivison = 1,
        SelectiveStormChaos = 2,
        ForcedChaos = 3
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

    struct brain_identity
    {
        int length;
        std::int8_t *horizontal;
        std::int8_t *vertical;
        std::int8_t *g_weight;
    };

    bool operator==(const the_site &lhs, const the_site &rhs);

    bool operator!=(const the_site &lhs, const the_site &rhs);

    struct brain_breakfast
    {
        /**
         * @brief Horizontal length of data
         */
        std::uint32_t widthSpace;
        /**
         * @brief Vertical length of data
         */
        std::uint32_t heightSpace;
        /**
         * @brief Length of data
         */
        int len;
        /**
         * @brief The bits of locks (raw data)
         */
        void *data;
        /**
         * @brief Default constructor
         */
        brain_breakfast() : widthSpace(0), heightSpace(0), len(0), data(nullptr)
        {
        }
    };

    BRAIN_TEMPLATE
    class basic_brain_map;

    template <typename IList>
    struct navigate_result
    {
        NavigationStatus status;
        typename IList::value_type from;
        typename IList::value_type to;
        IList connections;
    };

    BRAIN_TEMPLATE
    class basic_brain_map
    {
    public:
        template <typename T>
        using basic_list = std::vector<T>;

        using site_type = ISite;
        using neuron_type = INeuron;
        using list_site = basic_list<ISite>;
        using list_neuron = basic_list<INeuron *>;
        using result_site = navigate_result<list_site>;
        using result_neuron = navigate_result<list_neuron>;
        using randomize_function = std::function<int(void)>;

        explicit basic_brain_map(std::uint32_t xlength, std::uint32_t ylength);

        ~basic_brain_map();

        /**
         * @brief get the raw brain data (with locks)
         * @return raw data of the brain map (with locks)
         */
        inline void *data() const;

        /**
         * @brief clear neurons
         * @clearLocks also clear locked data
         */
        inline void clear(bool clearLocks = false);

        /**
         * @brief fill data
         * @param fillLocks can fill locks
         */
        inline void fill(bool fillLocks = false);

        /**
         * @brief fill lock data
         * @param state fill with
         */
        inline void fill_locks(bool state);

        /**
         * @brief randomize_hardware the function for randomized data
         * @param flagFilter the filter only source
         */
        inline void randomize_hardware(int flagFilter = 0xdeadbeff);

        /**
         * @brief set the custom randomize functions
         * @param randomizer The candidate for set (default std::rand())
         */
        inline void set_random_function(const randomize_function &randomizer);

        /**
         * @brief create maze for map
         */
        inline void create_maze();

        /**
         * @brief create maze for map with extensions
         * @param mazeAlgorithm The Algoritm for generate maze system
         * @param startOf The candidate for start generation
         * @param endOf The candidate for end generation
         */
        inline void create_maze_ex(MazeAlgorithm mazeAlgoritm, ISite startOf, ISite endOf);

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
         * @brief get the identity method
         * @see MatrixIdentity
         * @return MatrixIdentity value
         */
        inline MatrixIdentity get_identity();

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
        template <typename list_type = result_site>
        bool find(navigate_result<list_type> &navResult, INeuron *firstNeuron, INeuron *lastNeuron);

        /**
         * @brief find locations for selects
         * @param navResult the result of the selected (for the site result type or the neurons)
         * @param first the start of the find to end
         * @param last the end of the find to start
         * @return status of the finded
         */
        template <typename list_type = result_site>
        bool find(navigate_result<list_type> &navResult, const ISite &first, const ISite &last);

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

        /**
         * @brief Get Neighbours offset selective
         * @param identity method
         * @param offset selective
         * @return the result of the selected (for the site result type or the neurons)
         */
        template <typename list_type = list_site, typename target_type = typename list_type::value_type>
        inline list_type get_neighbours(MatrixIdentity matrixIdentity, const target_type &from);

        /**
         * @brief Get the random value of min to max
         * @param min the minimum
         * @param max the maximum
         * @return unique randomized value
         */
        inline int random_number(int min, int max);

    protected:
        INeuron *neurons;
        randomize_function _rand_ = std::rand;
        weight_t (*__heuristic__)(weight_t dx, weight_t dy);
        std::uint32_t _seg_off;
        std::uint32_t _xsize, _ysize;
        brain_identity identity;

        void _internal_realloc();
    };

#include "brain_impl.hpp"
#include "brain_maze.hpp"

    typedef basic_brain_map<the_site, the_neuron> brain_map;

} // namespace brain

#undef BRAIN_TEMPLATE
#undef BRAIN_DEFINE

#endif
