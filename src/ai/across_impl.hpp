// declarations Across Map (algorithm) part-file
#ifndef _ACROSS_IMPL_HPP_
#define _ACROSS_IMPL_HPP_

#include "across.hpp"

class across_range_error : public std::range_error
{
public:
    across_range_error() : std::range_error("range")
    {
    }
};

ACROSS_TEMPLATE
class PointerUtils
{
public:
    struct radar_t
    {
        ACROSS_DEFINE *map;
        TpNeuronPoint center;
        TpNeuronPoint target;
        ContainerType *detected;
        std::size_t maxCount = -1;
        int filterFlag = -1;
    };

    static std::uint32_t heuristic_euclidean(const TpNeuronPoint &lhs, const TpNeuronPoint &rhs)
    {
        int dx = rhs.x - lhs.x;
        int dy = rhs.y - lhs.y;
        return static_cast<std::uint32_t>(dx * dx + dy * dy);
    }

    static std::uint32_t heuristic_manhtatten(const TpNeuronPoint &lhs, const TpNeuronPoint &rhs)
    {
        return static_cast<std::uint32_t>(abs(rhs.x - lhs.x) + abs(rhs.y - lhs.y));
    }

    static std::uint32_t heuristic_pythagorean(const TpNeuronPoint &lhs, const TpNeuronPoint &rhs)
    {
        int dx = rhs.x - lhs.x;
        int dy = rhs.y - lhs.y;
        return static_cast<std::uint32_t>(dx * dx + dy * dy);
    }

    static std::uint32_t heuristic_chebyshev(const TpNeuronPoint &lhs, const TpNeuronPoint &rhs)
    {
        std::uint32_t dx = std::abs(rhs.x - lhs.x);
        std::uint32_t dy = std::abs(rhs.y - lhs.y);
        return std::max(dx, dy);
    }

    static std::uint32_t heuristic_diagonal(const TpNeuronPoint &lhs, const TpNeuronPoint &rhs)
    {
        int dx = std::abs(rhs.x - lhs.x);
        int dy = std::abs(rhs.y - lhs.y);
        int minDelta = std::min(dx, dy);
        int maxDelta = std::max(dx, dy);
        return static_cast<std::uint32_t>(minDelta * minDelta + (maxDelta - minDelta) * (maxDelta - minDelta));
    }

    static TpNeuronPoint get_minimum(ACROSS_DEFINE &map, ContainerType &paths)
    {
        int min = INTMAX_MAX;
        auto result = begin(paths);
        for(auto i = result; i != std::end(paths); ++i)
        {
            int g = map.get_ntotal(*i);
            if(g <= min)
            {
                min = g;
                result = i;
            }
        }
        return *result;
    }

    template <typename IdentityWrapper>
    static int get_matrix(MatrixIdentity identity, IdentityWrapper &place)
    {
        static std::int8_t M_SQUARE_H_POINT[] {-1, 0, 1, -1, 1, -1, 0, 1};
        static std::int8_t M_SQUARE_V_POINT[] {-1, -1, -1, 0, 0, 1, 1, 1};

        static std::int8_t PLUS_H_POINT[] {-1, 1, 0, 0};
        static std::int8_t PLUS_V_POINT[] {0, 0, -1, 1};

        static std::int8_t M_CROSS_H_POINT[] {-1, 1, -1, 1};
        static std::int8_t M_CROSS_V_POINT[] {-1, -1, 1, 1};

        switch(identity)
        {
            case MatrixIdentity::SquareMethod:
                place.horizontal = M_SQUARE_H_POINT;
                place.vertical = M_SQUARE_V_POINT;
                return place.length = sizeof(M_SQUARE_H_POINT);
                break;
            case MatrixIdentity::PlusMethod:
                place.horizontal = PLUS_H_POINT;
                place.vertical = PLUS_V_POINT;
                return place.length = sizeof(PLUS_H_POINT);
                break;
            case MatrixIdentity::CrossMethod:
                place.horizontal = M_CROSS_H_POINT;
                place.vertical = M_CROSS_V_POINT;
                return place.length = sizeof(M_CROSS_H_POINT);
                break;
        }
        return 0;
    }

    // radar detector free path's
    static void detect_to(radar_t &radar_info)
    {
        TpNeuronMember *it = nullptr;
        TpNeuronPoint point;
        int i;
        //    // TODO: Написать интелектуальный пойск путей для достижения лучших
        //    // результатов.
        //    // TODO: Приводить вектор направление для лучшего достижения.
        //    // TODO: Выводить оптимальный результат, чтобы было меньше итерации
        //    // TODO: Вывести итог и анализ скорости.
        //    c = get_matrix(NavMethodRule::SquareMethod, &matrixH, &matrixV);
        //    do
        //    {
        //        point.x = radar.center.x + matrixH[i];
        //        point.y = radar.center.y + matrixV[i];
        //        it = radar.map->get(point);
        //        if(it && !(radar.map)->get_nlocked(point) && (radar.filterFlag == ~0 || it->flags & radar.filterFlag))
        //        {
        //            if(point.x == radar.target.x || point.y == radar.target.y)
        //            {
        //                if(point == radar.target)
        //                {
        //                    i = c;
        //                    radar.detected->clear();
        //                }
        //                radar.detected->emplace_front(point);
        //            }
        //            else
        //                radar.detected->emplace_back(point);
        //        }
        //        // next step
        //    } while(radar.maxCount != radar.detected->size() && i++ != c);
        //}
        // break;

        for(i = 0; i != radar_info.map->identity.length; ++i)
        {
            point.x = radar_info.center.x + radar_info.map->identity.horizontal[i];
            point.y = radar_info.center.y + radar_info.map->identity.vertical[i];
            it = radar_info.map->get(point);
            if(it && !(radar_info.map)->get_nlocked(point) && (radar_info.filterFlag == ~0 || it->flags & radar_info.filterFlag))
            {
                radar_info.detected->emplace_back(point);
                if(radar_info.maxCount == radar_info.detected->size() || point == radar_info.target)
                    break;
            }
        }
    }
};
ACROSS_TEMPLATE
ACROSS_DEFINE::basic_across_map(int lwidth, int lheight)
{
    if(!lwidth || !lheight)
        throw std::runtime_error("Width or Height is zero!");

    this->widthSpace = lwidth;
    this->heightSpace = lheight;

    // The Euclidean heuristic is set as default
    set_heuristic(HeuristicMethod::Euclidean);
    // The Identity vector lines
    set_identity(MatrixIdentity::SquareMethod);

    std::div_t lockedDiv = div(std::max(lheight = (lwidth * lheight), 8), 8); // add locked bits
    this->segmentOffset = lwidth = lockedDiv.quot + lockedDiv.rem;
    lwidth = lheight * sizeof(TpNeuronMember) + lwidth + 100;
    this->neurons = static_cast<TpNeuronMember *>(std::malloc(lwidth));
}
ACROSS_TEMPLATE
ACROSS_DEFINE::~basic_across_map()
{
    std::free(this->neurons);
}
ACROSS_TEMPLATE
void ACROSS_DEFINE::randomize_hardware(int flagFilter)
{
    std::uint32_t lhs, rhs = segmentOffset;
    clear(true);
    do
    {
        lhs = static_cast<std::uint32_t>(rand() & flagFilter);
        memcpy(
            reinterpret_cast<void *>(reinterpret_cast<std::size_t>(neurons) + segmentOffset - rhs),
            &lhs,
            std::min(rhs, (std::uint32_t) sizeof(long)));
        rhs -= std::min(rhs, static_cast<std::uint32_t>(sizeof(long)));
    } while(rhs > 0);
}
ACROSS_TEMPLATE
void ACROSS_DEFINE::stress()
{
    // TODO: next a strees
}
ACROSS_TEMPLATE
bool ACROSS_DEFINE::set_identity(MatrixIdentity identity)
{
    int len;
    len = PointerUtilsInstance::get_matrix(identity, this->identity);
    return len > 0;
}
ACROSS_TEMPLATE bool ACROSS_DEFINE::set_heuristic(HeuristicMethod method)
{
    bool status = true;
    switch(method)
    {
        case HeuristicMethod::Euclidean:
            __heuristic__ = PointerUtilsInstance::heuristic_euclidean;
            break;
        case HeuristicMethod::Manhattan:
            __heuristic__ = PointerUtilsInstance::heuristic_manhtatten;
            break;
        case HeuristicMethod::Pythagorean:
            __heuristic__ = PointerUtilsInstance::heuristic_pythagorean;
            break;
        case HeuristicMethod::Chebyshev:
            __heuristic__ = PointerUtilsInstance::heuristic_chebyshev;
            break;
        case HeuristicMethod::Diagonal:
            __heuristic__ = PointerUtilsInstance::heuristic_diagonal;
            break;
        default:
            status = false;
            break;
    }
    return status;
}
ACROSS_TEMPLATE
HeuristicMethod ACROSS_DEFINE::get_heuristic()
{
    if(__heuristic__ == PointerUtilsInstance::heuristic_euclidean)
        return HeuristicMethod::Euclidean;
    else if(__heuristic__ == PointerUtilsInstance::heuristic_manhtatten)
        return HeuristicMethod::Manhattan;
    else if(__heuristic__ == PointerUtilsInstance::heuristic_pythagorean)
        return HeuristicMethod::Pythagorean;
    else if(__heuristic__ == PointerUtilsInstance::heuristic_chebyshev)
        return HeuristicMethod::Chebyshev;
    else if(__heuristic__ == PointerUtilsInstance::heuristic_diagonal)
        return HeuristicMethod::Diagonal;
    else
        return HeuristicMethod::Invalid;
}
ACROSS_TEMPLATE
int ACROSS_DEFINE::get_width()
{
    return widthSpace;
}
ACROSS_TEMPLATE
int ACROSS_DEFINE::get_height()
{
    return heightSpace;
}
ACROSS_TEMPLATE
void ACROSS_DEFINE::clear(bool clearLocks)
{
    std::uint32_t length = widthSpace * heightSpace * sizeof(TpNeuronMember);
    std::uint32_t leftOffset;
    if(clearLocks)
    {
        leftOffset = 0;
        length += this->segmentOffset;
    }
    else
    {
        leftOffset = this->segmentOffset;
    }
    memset(reinterpret_cast<void *>(reinterpret_cast<std::size_t>(neurons) + leftOffset), 0, length);
}
ACROSS_TEMPLATE
void ACROSS_DEFINE::fill(bool fillLocks)
{
    std::uint32_t length = widthSpace * heightSpace * sizeof(TpNeuronMember);
    std::uint32_t leftoffset;
    if(!fillLocks)
    {
        leftoffset = this->segmentOffset;
    }
    else
    {
        leftoffset = 0;
        length += this->segmentOffset;
    }
    memset(reinterpret_cast<void *>(reinterpret_cast<std::size_t>(neurons) + leftoffset), 0xff, length);
}
ACROSS_TEMPLATE
TpNeuronMember *ACROSS_DEFINE::get(int x, int y)
{
    return get({x, y});
}
ACROSS_TEMPLATE
TpNeuronMember *ACROSS_DEFINE::get(const TpNeuronPoint &range)
{
    TpNeuronMember *result = nullptr;
    if(contains(range))
        result = reinterpret_cast<TpNeuronMember *>(
            (reinterpret_cast<std::size_t>(neurons)) + ((range.x * heightSpace + range.y) * sizeof(TpNeuronMember)) + segmentOffset);
    return result;
}

ACROSS_TEMPLATE
std::uint32_t ACROSS_DEFINE::get_cached_size()
{
    std::uint32_t cal = 0;

    TpNeuronMember *n = reinterpret_cast<TpNeuronMember *>(reinterpret_cast<std::size_t>(neurons) + segmentOffset);
    TpNeuronMember *nM = n + widthSpace * heightSpace;
    while(n < nM)
    {
        cal += n->cost + n->h;
        ++n;
    }
    return cal;
}
ACROSS_TEMPLATE
TpNeuronMember *ACROSS_DEFINE::front()
{
    return get(0, 0);
}
ACROSS_TEMPLATE
TpNeuronMember *ACROSS_DEFINE::back()
{
    return get(widthSpace - 1, heightSpace - 1);
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::get_nlocked(const TpNeuronPoint &range)
{
    if(!contains(range))
        throw across_range_error();
    auto divide = std::div(range.x * heightSpace + range.y, 8);
    auto pointer = reinterpret_cast<std::uint8_t *>(neurons) + divide.quot;
    return (*pointer) & (1 << divide.rem);
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::contains(const TpNeuronPoint &range)
{
    return (range.x < widthSpace && range.y < heightSpace && range.x > ~0 && range.y > ~0);
}

ACROSS_TEMPLATE
std::uint8_t &ACROSS_DEFINE::get_nflag(const TpNeuronPoint &range)
{
    TpNeuronMember *n = get(range);
    if(!n)
        throw across_range_error();
    return n->flags;
}
ACROSS_TEMPLATE
std::uint32_t &ACROSS_DEFINE::get_ncost(const TpNeuronPoint &range)
{
    TpNeuronMember *n = get(range);
    if(!n)
        throw across_range_error();
    return n->cost;
}
ACROSS_TEMPLATE
std::uint32_t &ACROSS_DEFINE::get_nheuristic(const TpNeuronPoint &range)
{
    TpNeuronMember *n = get(range);
    if(!n)
        throw across_range_error();
    return n->h;
}
ACROSS_TEMPLATE
const int ACROSS_DEFINE::get_nweight(const TpNeuronPoint &range)
{
    if(!contains(range))
        throw across_range_error();
    return range.x * range.y + range.y * range.y;
}
ACROSS_TEMPLATE
const std::uint32_t ACROSS_DEFINE::get_ntotal(const TpNeuronPoint &range)
{
    TpNeuronMember *n = get(range);
    if(!n)
        throw across_range_error();
    return n->cost + n->h;
}
ACROSS_TEMPLATE
const bool ACROSS_DEFINE::get_nempty(const TpNeuronPoint &range)
{
    return get_ntotal(range) == 0;
}
ACROSS_TEMPLATE
bool ACROSS_DEFINE::get_nlocked(const TpNeuronMember *neuron)
{
    return get_nlocked(get_npoint(neuron));
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::set_nlock(const TpNeuronMember *neuron, const bool state)
{
    neuronLock(get_point(neuron), state);
}

ACROSS_TEMPLATE
const TpNeuronPoint ACROSS_DEFINE::get_npoint(const TpNeuronMember *neuron)
{
    auto divide = std::div((std::size_t(neuron) - std::size_t(neurons) - segmentOffset) / sizeof(TpNeuronMember), heightSpace);
    return {divide.quot, divide.rem};
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::set_nlock(const TpNeuronPoint &range, const bool state)
{
    auto divide = std::div(range.x * heightSpace + range.y, 8);
    auto pointer = (reinterpret_cast<std::uint8_t *>(neurons) + segmentOffset) + divide.quot;
    divide.quot = (1 << divide.rem);
    // TODO: Optimized to ~divide.quout (xor)
    (*pointer) ^= (*pointer) & (divide.quot);
    (*pointer) |= divide.quot * (state == true);
}
ACROSS_TEMPLATE
void ACROSS_DEFINE::load(const AcrossData &AcrossData)
{
    if(!AcrossData.widthSpace || !AcrossData.heightSpace)
        throw std::runtime_error("Argument param, width or height is empty");

    if(AcrossData.neurons == nullptr)
        throw std::runtime_error("Data is null");

    if(this->neurons == AcrossData.neurons)
        throw std::runtime_error("neurons == Across data, unflow effect");

    if(this->neurons != nullptr)
        std::free(this->neurons);

    this->widthSpace = AcrossData.widthSpace;
    this->heightSpace = AcrossData.heightSpace;
    this->neurons = static_cast<TpNeuronMember *>(std::malloc(widthSpace * heightSpace * sizeof(TpNeuronMember)));

    memcpy(this->neurons, AcrossData.neurons, widthSpace * heightSpace * sizeof(TpNeuronMember));
}
ACROSS_TEMPLATE
void ACROSS_DEFINE::save(AcrossData *AcrossData)
{
    if(AcrossData == nullptr)
        return;

    AcrossData->widthSpace = this->widthSpace;
    AcrossData->heightSpace = this->heightSpace;
    AcrossData->neurons = this->neurons;
}
ACROSS_TEMPLATE
bool ACROSS_DEFINE::find(NavigationResult &navigationResult, TpNeuronPoint first, TpNeuronPoint last)
{
    return this->find(navigationResult, this->get(first), this->get(last));
}
ACROSS_TEMPLATE
bool ACROSS_DEFINE::find(NavigationResult &navigationResult, TpNeuronMember *firstNeuron, TpNeuronMember *lastNeuron)
{
    /*
    ASTAR Pseudocode
    function AStar(start, goal)
    openSet := {start} // множество вершин, ожидающих рассмотрения
    closedSet := {} // множество уже рассмотренных вершин
    cameFrom := {} // словарь для отслеживания пути
    gScore := map with default value of Infinity // дистанции от стартовой вершины до каждой вершины, изначально бесконечность
    gScore[start] := 0 // дистанция от стартовой вершины до самой себя равна 0
    fScore := map with default value of Infinity // оценка общей стоимости для каждой вершины, изначально бесконечность
    fScore[start] := heuristic_cost_estimate(start, goal) // эвристическая оценка стоимости пути от стартовой до целевой вершины

    while openSet is not empty
        current := node in openSet with the lowest fScore // выбираем вершину с наименьшей оценкой fScore
        if current is goal
            return reconstruct_path(cameFrom, current)

        remove current from openSet
        add current to closedSet

        for each neighbor of current
            if neighbor is in closedSet
                continue // пропускаем уже рассмотренные вершины
            tentative_gScore := gScore[current] + dist_between(current, neighbor) // предварительная дистанция от старт. верш. до соседней
            if tentative_gScore < gScore[neighbor]
                cameFrom[neighbor] := current
                gScore[neighbor] := tentative_gScore
                fScore[neighbor] := gScore[neighbor] + heuristic_cost_estimate(neighbor, goal)
                if neighbor not in openSet
                    add neighbor to openSet

    return failure // если не найден путь

    function reconstruct_path(cameFrom, current)
        totalPath := [current]
        while current in cameFrom.Keys
            current := cameFrom[current]
            add current to totalPath
        return totalPath

    */
    enum : std::uint8_t
    {
        FLAG_CLEAN = 0,
        FLAG_OPEN_LIST = 1,
        FLAG_CLOSED_LIST = 2,
        FLAG_TILED_LIST = 4
    };

    using PointerUtils = PointerUtils<TpNeuronPoint, TpNeuronMember, ContainerType>;

    TpNeuronMember *current, *select;

    if(firstNeuron == nullptr || lastNeuron == nullptr)
    {
        navigationResult.status = NavigationStatus::Undefined;
        return false;
    }
    if(get_nlocked(firstNeuron) || get_nlocked(lastNeuron))
    {
        navigationResult.status = NavigationStatus::Locked;
        return false;
    }
    typename PointerUtils::radar_t radar_info;
    typename ContainerType::iterator iter, p1, p2;
    ContainerType detected;
    TpNeuronPoint first = get_npoint(firstNeuron), last = get_npoint(lastNeuron);

    radar_info.map = this;
    radar_info.detected = &detected;
    radar_info.maxCount = -1;
    radar_info.filterFlag = -1;
    radar_info.target = last;

    navigationResult.firstNeuron = first;
    navigationResult.lastNeuron = last;
    navigationResult.roads.clear();
    navigationResult.roads.emplace_back(first);
    navigationResult.status = NavigationStatus::Opened;

    firstNeuron->h = __heuristic__(first, last);
    // Перестройка
    while(!navigationResult.roads.empty())
    {
        current = get(navigationResult.roads.front()); // get the best Neuron get(PointerUtils::get_minimum(*this, navigationResult.roads));
        if(current == lastNeuron)
        {
            break; // close and combine finded roads
        }

        current->flags = FLAG_CLOSED_LIST; // change to closing list
        navigationResult.roads.pop_front();

        // Detect free points
        radar_info.center = this->get_npoint(current);
        PointerUtils::detect_to(radar_info);
        for(iter = std::begin(detected); iter != std::end(detected); ++iter)
        {
            select = get(*iter);
            if(select->flags == FLAG_CLEAN) // free path
            {
                select->flags = FLAG_OPEN_LIST;
                select->cost = current->cost + 1;
                select->h = __heuristic__((*iter), last);

                navigationResult.roads.emplace_back((*iter));
                p1 = std::begin(navigationResult.roads);
                p2 = std::end(navigationResult.roads);

                for(; p1 != p2;)
                {
                    if(get_ntotal(*p1) > get_ntotal(*iter))
                    {
                        navigationResult.roads.emplace(p1, (*iter));
                        break;
                    }
                    ++p1;
                }
                navigationResult.roads.emplace_back((*iter));
            }
        }

        detected.clear(); // clear
    }

    TpNeuronMember *lastSelect = nullptr;
    current = lastNeuron;
    navigationResult.roads.clear();
    navigationResult.roads.emplace_back(last);

    radar_info.maxCount = -1;
    radar_info.target = first;
    radar_info.filterFlag = FLAG_OPEN_LIST | FLAG_CLOSED_LIST;
    while(current != firstNeuron)
    {
        if(current == lastSelect)
        {
            navigationResult.status = NavigationStatus::Closed;
            break;
        }
        lastSelect = current;

        radar_info.center = get_npoint(current);
        PointerUtils::detect_to(radar_info);
        for(iter = std::begin(detected); iter != std::end(detected); ++iter)
        {
            select = get(*iter);
            if((select->cost && select->cost < current->cost) || select == firstNeuron)
            {
                current = select;
                navigationResult.roads.emplace_front(*iter);
                current->flags = FLAG_TILED_LIST;
            }
        }

        detected.clear(); // clear
    }
    return navigationResult.status == NavigationStatus::Opened;
}

bool operator!=(const NeuronPoint &a, const NeuronPoint &b)
{
    return static_cast<bool>(std::memcmp(&a, &b, sizeof(NeuronPoint)));
}

bool operator==(const NeuronPoint &a, const NeuronPoint &b)
{
    return static_cast<bool>(!std::memcmp(&a, &b, sizeof(NeuronPoint)));
}

#endif
