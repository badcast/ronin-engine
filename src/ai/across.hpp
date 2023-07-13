/*******************************************************************************************************************************************
 *
 * ASTAR Algorithm composed to RoninEngine (Navigation AI)
 * creation date: 18.01.2023
 * author: badcast <lmecomposer@gmail.com>
 * C++11 minimum required
 * License: GPLv3
 *
 * ENG
 *
 * The fastest and most preferable type of heuristic can depend on the specific problem, input data, and requirements. Each heuristic
function has its own characteristics and can be effective in certain situations. However, in general, the heuristic that more accurately
estimates the remaining cost and better guides the search algorithm will be preferable.

Here are some general recommendations for choosing a heuristic function:

    Euclidean distance: This heuristic is often effective and preferable when movement between states is unconstrained and obstacle-free. It
is based on the geometric distance between points and can be a good choice for tasks where accuracy and execution speed are important.

    Manhattan distance: This heuristic works well in grid-based environments where movement is limited to horizontal and vertical
directions. It is usually straightforward to compute and can be effective for tasks where obstacles and constraints have geometric
limitations.

    Chebyshev distance: This heuristic uses the maximum difference between coordinates and works well when diagonal as well as
horizontal/vertical movements are allowed. It can be useful for tasks where diagonal movements are possible or when differences in the cost
of movements in different directions need to be considered.

    Other heuristics: Other heuristic functions such as Pythagorean distance or diagonal distance can also be effective in different
situations. Their effectiveness will depend on the specifics of the problem and state space.

It is important to perform testing and compare the performance of different heuristic functions on your specific data to choose the most
suitable one for your task.
 *
 * RUS
 *
 *Самый быстрый и предпочтительный тип эвристики может зависеть от конкретной задачи, входных данных и требований. Каждая эвристическая
функция имеет свои особенности и может быть эффективной в определенных ситуациях. Однако, в общем случае, эвристика, которая более точно
оценивает оставшуюся стоимость и лучше направляет алгоритм поиска, будет предпочтительной.

Вот некоторые общие рекомендации для выбора эвристической функции:

    Евклидово расстояние (Euclidean distance): Эта эвристика обычно является эффективной и предпочтительной, когда движение между
состояниями осуществляется без ограничений и без препятствий. Она основана на геометрическом расстоянии между точками и может быть хорошим
выбором для задач, где точность и скорость выполнения важны.

    Расстояние по Манхэттену (Manhattan distance): Эта эвристика хорошо работает в сеточных средах, где движение осуществляется только по
горизонтали и вертикали. Она обычно проста в вычислении и может быть эффективной для задач, где преграды и препятствия имеют геометрические
ограничения.

    Расстояние Чебышева (Chebyshev distance): Эта эвристика использует максимальную разность между координатами и хорошо работает в случаях,
когда разрешены перемещения по диагонали и горизонтали/вертикали. Она может быть полезной для задач, где возможны перемещения по диагонали
или когда нужно учесть различия в стоимости перемещений в разных направлениях.

    Другие эвристики: Другие эвристические функции, такие как расстояние Пифагора (Pythagorean distance) или диагональное расстояние
(Diagonal distance), также могут быть эффективными в различных ситуациях. Их эффективность будет зависеть от особенностей конкретной задачи
и пространства состояний.

Важно провести тестирование и сравнить производительность разных эвристических функций на ваших конкретных данных, чтобы выбрать наиболее
подходящую для вашей задачи.

 ** see HeuristicMethod
 ** basic_across_map::set_heuristic(HeuristicMethod)

*******************************************************************************************************************************************/

#ifndef _ACROSS_HPP_
#define _ACROSS_HPP_

#include <cstdint>
#include <cmath>
#include <deque>
#include <cstring>
#include <stdexcept>

namespace across
{
    enum class NavStatus
    {
        Undefined,
        Locked,
        Closed,
        Opened
    };

    enum class NavMethodRule
    {
        NavigationIntelegency,
        PlusMethod,
        SquareMethod,
        CrossMethod
    };

    enum class HeuristicMethod
    {
        Invalid = -1,
        Euclidean,
        Manhattan,
        Pythagorean,
        Chebyshev,
        Diagonal
    };

    struct NeuronPoint
    {
        int x;
        int y;
    };

    bool operator==(const NeuronPoint &a, const NeuronPoint &b);

    bool operator!=(const NeuronPoint &a, const NeuronPoint &b);

    struct NeuronMember
    {
        std::uint8_t flags;
        std::uint32_t h;
        std::uint32_t cost;
    };

    struct AcrossData
    {
        std::uint32_t widthSpace;
        std::uint32_t heightSpace;
        void *neurons;
    };

    template <typename AcrossMapType, typename ContainerType>
    struct NavResult
    {
        NavStatus status;
        typename AcrossMapType::TypePoint firstNeuron;
        typename AcrossMapType::TypePoint lastNeuron;
        ContainerType roads;
        const AcrossMapType *map;
    };

    template <typename TpNeuronPoint, typename TpNeuronMember, typename ContainerType>
    class basic_across_map
    {
    protected:
        int segmentOffset;
        TpNeuronMember *neurons;
        int widthSpace, heightSpace;
        std::uint32_t (*__heuristic__)(const TpNeuronPoint &, const TpNeuronPoint &);

    public:
        typedef TpNeuronMember TypeNeuron;
        typedef TpNeuronPoint TypePoint;
        typedef NavResult<basic_across_map, ContainerType> NavigationResult;

        explicit basic_across_map(int width, int height);

        ~basic_across_map();

        void clear(bool clearLocks = false);
        void fill(bool fillLocks = false);
        void randomize(int flagFilter = 0xdeadbef);
        void stress();

        bool set_heuristic(HeuristicMethod method);
        HeuristicMethod get_heuristic();

        int get_width();
        int get_height();

        TpNeuronMember *get(int x, int y);
        TpNeuronMember *get(const TpNeuronPoint &point);

        inline bool contains(const TpNeuronPoint &point);

        TpNeuronMember *front();
        TpNeuronMember *back();

        // pointer with Point
        inline bool get_nlocked(const TpNeuronPoint &point);
        inline std::uint8_t &get_nflag(const TpNeuronPoint &point);
        inline std::uint32_t &get_ncost(const TpNeuronPoint &point);
        inline std::uint32_t &get_nheuristic(const TpNeuronPoint &point);
        inline const int get_nweight(const TpNeuronPoint &point);
        inline const std::uint32_t get_ntotal(const TpNeuronPoint &point);
        inline const bool get_nempty(const TpNeuronPoint &point);
        void set_nlock(const TpNeuronPoint &point, const bool state);

        // pointer with pointer
        bool get_nlocked(const TpNeuronMember *neuron);
        std::uint8_t &get_nflag(const TpNeuronMember *neuron);
        std::uint32_t &get_ncost(const TpNeuronMember *neuron);
        std::uint32_t &get_nheuristic(const TpNeuronMember *neuron);
        const int get_nweight(const TpNeuronMember *neuron);
        const std::uint32_t get_ntotal(const TpNeuronMember *neuron);
        inline const bool get_nempty(const TpNeuronMember *neuron);
        void set_nlock(const TpNeuronMember *neuron, const bool state);

        inline const TpNeuronPoint get_npoint(const TpNeuronMember *neuron);

        void find(NavigationResult &navResult, NavMethodRule method, TpNeuronMember *firstNeuron, TpNeuronMember *lastNeuron);

        void find(NavigationResult &navResult, NavMethodRule method, TpNeuronPoint first, TpNeuronPoint last);

        void load(const AcrossData &AcrossData);

        void save(AcrossData *AcrossData);

        std::uint32_t get_cached_size();
    };

#include "across_impl.hpp"

    typedef basic_across_map<NeuronPoint, NeuronMember, std::deque<NeuronPoint>> AcrossMap;

} // namespace across

#endif
