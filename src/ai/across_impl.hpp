// declarations Across Map (algorithm) part-file
#ifndef _ACROSS_IMPL_HPP_
#define _ACROSS_IMPL_HPP_

#include "across.hpp"

#define ACROSS_TEMPLATE template <typename Tp, typename TpNeuron, typename ContainerType>
#define ACROSS_DEFINE basic_across_map<Tp, TpNeuron, ContainerType>

ACROSS_TEMPLATE
class PointerUtils
{
public:
    struct radar_t
    {
        ACROSS_DEFINE *map;
        NavMethodRule method;
        Tp center;
        Tp target;
        ContainerType *detected;
        std::size_t maxCount = -1;
        int filterFlag = -1;
    };

    static std::uint32_t heuristic_euclidean(const Tp &lhs, const Tp &rhs)
    {
        // double dx = x2 - x1;
        // double dy = y2 - y1;
        // return std::sqrt(dx * dx + dy * dy);
        int dx = rhs.x - lhs.x;
        int dy = rhs.y - lhs.y;
        return static_cast<std::uint32_t>(dx * dx + dy * dy);
    }

    static std::uint32_t heuristic_manhtatten(const Tp &lhs, const Tp &rhs)
    {
        return static_cast<std::uint32_t>(abs(rhs.x - lhs.x) + abs(rhs.y - lhs.y));
    }

    static std::uint32_t heuristic_pythagorean(const Tp &lhs, const Tp &rhs)
    {
        int dx = rhs.x - lhs.x;
        int dy = rhs.y - lhs.y;
        return static_cast<std::uint32_t>(dx * dx + dy * dy);
    }

    static std::uint32_t heuristic_chebyshev(const Tp &lhs, const Tp &rhs)
    {
        std::uint32_t dx = std::abs(rhs.x - lhs.x);
        std::uint32_t dy = std::abs(rhs.y - lhs.y);
        return std::max(dx, dy);
    }

    static std::uint32_t heuristic_diagonal(const Tp &lhs, const Tp &rhs)
    {
        int dx = std::abs(rhs.x - lhs.x);
        int dy = std::abs(rhs.y - lhs.y);
        int minDelta = std::min(dx, dy);
        int maxDelta = std::max(dx, dy);
        return static_cast<std::uint32_t>(minDelta * minDelta + (maxDelta - minDelta) * (maxDelta - minDelta));
    }

    static auto get_minimum(ACROSS_DEFINE &map, ContainerType *paths) -> decltype(std::begin(*paths))
    {
        int min = INTMAX_MAX;
        auto result = begin(*paths);
        for(auto i = result; i != std::end(*paths); ++i)
        {
            int g = map.get_ntotal(*i);
            if(g <= min)
            {
                min = g;
                result = i;
            }
        }
        return result;
    }

    static int get_matrix(NavMethodRule method, std::int8_t **matrixH, std::int8_t **matrixV)
    {
        static std::int8_t PLUS_H_POINT[] {-1, 1, 0, 0};
        static std::int8_t PLUS_V_POINT[] {0, 0, -1, 1};

        static std::int8_t M_SQUARE_H_POINT[] {-1, 0, 1, -1, 1, -1, 0, 1};
        static std::int8_t M_SQUARE_V_POINT[] {-1, -1, -1, 0, 0, 1, 1, 1};

        static std::int8_t M_CROSS_H_POINT[] {-1, 1, -1, 1};
        static std::int8_t M_CROSS_V_POINT[] {-1, -1, 1, 1};

        switch(method)
        {
            case NavMethodRule::PlusMethod:
                *matrixH = PLUS_H_POINT;
                *matrixV = PLUS_V_POINT;
                return sizeof(PLUS_H_POINT);
                break;
            case NavMethodRule::SquareMethod:
                *matrixH = M_SQUARE_H_POINT;
                *matrixV = M_SQUARE_V_POINT;
                return sizeof(M_SQUARE_H_POINT);
                break;
            case NavMethodRule::CrossMethod:
                *matrixH = M_CROSS_H_POINT;
                *matrixV = M_CROSS_V_POINT;
                return sizeof(M_CROSS_H_POINT);
                break;
        }
        return 0;
    }

    // radar detector free path's
    static void detect_to(radar_t &radar)
    {
        TpNeuron *it = nullptr;
        Tp point;
        int i = 0, c;
        std::int8_t *matrixH;
        std::int8_t *matrixV;
        switch(radar.method)
        {
            case NavMethodRule::NavigationIntelegency:
            {
                // TODO: Написать интелектуальный пойск путей для достижения лучших
                // результатов.
                // TODO: Приводить вектор направление для лучшего достижения.
                // TODO: Выводить оптимальный результат, чтобы было меньше итерации
                // TODO: Вывести итог и анализ скорости.
                c = get_matrix(NavMethodRule::SquareMethod, &matrixH, &matrixV);
                do
                {
                    point.x = radar.center.x + matrixH[i];
                    point.y = radar.center.y + matrixV[i];
                    it = radar.map->get(point);
                    if(it && !(radar.map)->get_nlocked(point) && (radar.filterFlag == ~0 || it->flags & radar.filterFlag))
                    {
                        if(point.x == radar.target.x || point.y == radar.target.y)
                        {
                            if(point == radar.target)
                            {
                                i = c;
                                radar.detected->clear();
                            }
                            radar.detected->emplace_front(point);
                        }
                        else
                            radar.detected->emplace_back(point);
                    }
                    // next step
                } while(radar.maxCount != radar.detected->size() && i++ != c);
            }
            break;
            default:

                c = get_matrix(radar.method, &matrixH, &matrixV);
                for(; i != c; ++i)
                {
                    point.x = radar.center.x + matrixH[i];
                    point.y = radar.center.y + matrixV[i];
                    it = radar.map->get(point);
                    if(it && !(radar.map)->get_nlocked(point) && (radar.filterFlag == ~0 || it->flags & radar.filterFlag))
                    {
                        radar.detected->emplace_back(point);
                        if(radar.maxCount == radar.detected->size() || point == radar.target)
                            break;
                    }
                }

                break;
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

    std::div_t lockedDiv = div(std::max(lheight = (lwidth * lheight), 8), 8); // add locked bits
    this->segmentOffset = lwidth = lockedDiv.quot + lockedDiv.rem;
    lwidth = lheight * sizeof(TpNeuron) + lwidth + 100;
    this->neurons = static_cast<TpNeuron *>(std::malloc(lwidth));
}
ACROSS_TEMPLATE
ACROSS_DEFINE::~basic_across_map()
{
    std::free(this->neurons);
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::randomize(int flagFilter)
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
bool ACROSS_DEFINE::set_heuristic(HeuristicMethod method)
{
    using PointerUtils = PointerUtils<Tp, TpNeuron, ContainerType>;

    bool status = true;
    switch(method)
    {
        case HeuristicMethod::Euclidean:
            __heuristic__ = PointerUtils::heuristic_euclidean;
            break;
        case HeuristicMethod::Manhattan:
            __heuristic__ = PointerUtils::heuristic_manhtatten;
            break;
        case HeuristicMethod::Pythagorean:
            __heuristic__ = PointerUtils::heuristic_pythagorean;
            break;
        case HeuristicMethod::Chebyshev:
            __heuristic__ = PointerUtils::heuristic_chebyshev;
            break;
        case HeuristicMethod::Diagonal:
            __heuristic__ = PointerUtils::heuristic_diagonal;
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
    using PointerUtils = PointerUtils<Tp, TpNeuron, ContainerType>;

    if(__heuristic__ == PointerUtils::heuristic_euclidean)
        return HeuristicMethod::Euclidean;
    else if(__heuristic__ == PointerUtils::heuristic_manhtatten)
        return HeuristicMethod::Manhattan;
    else if(__heuristic__ == PointerUtils::heuristic_pythagorean)
        return HeuristicMethod::Pythagorean;
    else if(__heuristic__ == PointerUtils::heuristic_chebyshev)
        return HeuristicMethod::Chebyshev;
    else if(__heuristic__ == PointerUtils::heuristic_diagonal)
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
    std::uint32_t length = widthSpace * heightSpace * sizeof(TpNeuron);
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
    std::uint32_t length = widthSpace * heightSpace * sizeof(TpNeuron);
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
TpNeuron *ACROSS_DEFINE::get(int x, int y)
{
    return get({x, y});
}
ACROSS_TEMPLATE
TpNeuron *ACROSS_DEFINE::get(const Tp &range)
{
    TpNeuron *result = nullptr;
    if(contains(range))
        result = reinterpret_cast<TpNeuron *>(
            (reinterpret_cast<std::size_t>(neurons)) + ((range.x * heightSpace + range.y) * sizeof(TpNeuron)) + segmentOffset);
    return result;
}

ACROSS_TEMPLATE
std::uint32_t ACROSS_DEFINE::get_cached_size()
{
    std::uint32_t cal = 0;

    TpNeuron *n = reinterpret_cast<TpNeuron *>(reinterpret_cast<std::size_t>(neurons) + segmentOffset);
    TpNeuron *nM = n + widthSpace * heightSpace;
    while(n < nM)
    {
        cal += n->cost + n->h;
        ++n;
    }
    return cal;
}
ACROSS_TEMPLATE
TpNeuron *ACROSS_DEFINE::front()
{
    return get(0, 0);
}
ACROSS_TEMPLATE
TpNeuron *ACROSS_DEFINE::back()
{
    return get(widthSpace - 1, heightSpace - 1);
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::get_nlocked(const Tp &range)
{
    if(!contains(range))
        throw std::out_of_range("range");
    auto divide = std::div(range.x * heightSpace + range.y, 8);
    auto pointer = reinterpret_cast<std::uint8_t *>(neurons) + divide.quot;
    return (*pointer) & (1 << divide.rem);
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::contains(const Tp &range)
{
    return (range.x < widthSpace && range.y < heightSpace && range.x > ~0 && range.y > ~0);
}

ACROSS_TEMPLATE
std::uint8_t &ACROSS_DEFINE::get_nflag(const Tp &range)
{
    TpNeuron *n = get(range);
    if(!n)
        throw std::out_of_range("range");
    return n->flags;
}
ACROSS_TEMPLATE
std::uint32_t &ACROSS_DEFINE::get_ncost(const Tp &range)
{
    TpNeuron *n = get(range);
    if(!n)
        throw std::out_of_range("range");
    return n->cost;
}
ACROSS_TEMPLATE
std::uint32_t &ACROSS_DEFINE::get_nheuristic(const Tp &range)
{
    TpNeuron *n = get(range);
    if(!n)
        throw std::out_of_range("range");
    return n->h;
}
ACROSS_TEMPLATE
const int ACROSS_DEFINE::get_nweight(const Tp &range)
{
    if(!contains(range))
        throw std::out_of_range("range");
    return range.x * range.y + range.y * range.y;
}
ACROSS_TEMPLATE
const std::uint32_t ACROSS_DEFINE::get_ntotal(const Tp &range)
{
    TpNeuron *n = get(range);
    if(!n)
        throw std::out_of_range("range");
    return n->cost + n->h;
}
ACROSS_TEMPLATE
const bool ACROSS_DEFINE::get_nempty(const Tp &range)
{
    return !neuronGetTotal(range);
}
ACROSS_TEMPLATE
bool ACROSS_DEFINE::get_nlocked(const TpNeuron *neuron)
{
    return neuronLocked(get_point(neuron));
}
ACROSS_TEMPLATE
std::uint8_t &ACROSS_DEFINE::get_nflag(const TpNeuron *neuron)
{
    return neuronGetFlag(get_point(neuron));
}
ACROSS_TEMPLATE
std::uint32_t &ACROSS_DEFINE::get_ncost(const TpNeuron *neuron)
{
    return neuronGetCost(get_point(neuron));
}
ACROSS_TEMPLATE
std::uint32_t &ACROSS_DEFINE::get_nheuristic(const TpNeuron *neuron)
{
    return neuronHeuristic(get_point(neuron));
}
ACROSS_TEMPLATE
const int ACROSS_DEFINE::get_nweight(const TpNeuron *neuron)
{
    return neuronGetWeight(get_point(neuron));
}
ACROSS_TEMPLATE
const std::uint32_t ACROSS_DEFINE::get_ntotal(const TpNeuron *neuron)
{
    return neuronEmpty(get_point(neuron));
}
ACROSS_TEMPLATE
const bool ACROSS_DEFINE::get_nempty(const TpNeuron *neuron)
{
    return neuronEmpty(get_point(neuron));
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::set_nlock(const TpNeuron *neuron, const bool state)
{
    neuronLock(get_point(neuron), state);
}

ACROSS_TEMPLATE
const Tp ACROSS_DEFINE::get_npoint(const TpNeuron *neuron)
{
    auto divide = std::div((std::size_t(neuron) - std::size_t(neurons) - segmentOffset) / sizeof(TpNeuron), heightSpace);
    return {divide.quot, divide.rem};
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::set_nlock(const Tp &range, const bool state)
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
    this->neurons = static_cast<TpNeuron *>(std::malloc(widthSpace * heightSpace * sizeof(TpNeuron)));

    memcpy(this->neurons, AcrossData.neurons, widthSpace * heightSpace * sizeof(TpNeuron));
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
void ACROSS_DEFINE::find(NavigationResult &navResult, NavMethodRule method, TpNeuron *firstNeuron, TpNeuron *lastNeuron)
{
    this->find(navResult, method, this->get_npoint(firstNeuron), this->get_npoint(lastNeuron));
}
ACROSS_TEMPLATE
void ACROSS_DEFINE::find(NavigationResult &navigationResult, NavMethodRule method, Tp first, Tp last)
{
    enum : std::uint8_t
    {
        FLAG_CLEAN = 0,
        FLAG_OPEN_LIST = 1,
        FLAG_CLOSED_LIST = 2,
        FLAG_TILED_LIST = 4
    };

    using PointerUtils = PointerUtils<Tp, TpNeuron, ContainerType>;

    TpNeuron *current, *firstNeuron, *lastNeuron, *select;

    if((firstNeuron = get(first)) == nullptr || (lastNeuron = get(last)) == nullptr)
    {
        navigationResult.status = NavStatus::Undefined;
        return;
    }
    if(get_nlocked(first) || get_nlocked(last))
    {
        navigationResult.status = NavStatus::Locked;
        return;
    }
    typename PointerUtils::radar_t radar_info;
    typename ContainerType::iterator iter, p1, p2;
    ContainerType detected;

    radar_info.map = this;
    radar_info.method = method;
    radar_info.detected = &detected;
    radar_info.maxCount = -1;
    radar_info.filterFlag = -1;
    radar_info.target = last;

    navigationResult.map = this;
    navigationResult.firstNeuron = first;
    navigationResult.lastNeuron = last;
    navigationResult.roads.clear();
    navigationResult.roads.emplace_back(first);

    firstNeuron->h = __heuristic__(first, last);
    // Перестройка
    navigationResult.status = NavStatus::Opened;
    while(!navigationResult.roads.empty())
    {
        current = get(navigationResult.roads.front()); // get the best Neuron
        if(current == lastNeuron)
        {
            break; // close and combine finded roads
        }

        current->flags = FLAG_CLOSED_LIST; // change to closing list
        navigationResult.roads.pop_front();

        // Avail
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

    TpNeuron *lastSelect = nullptr;
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
            navigationResult.status = NavStatus::Closed;
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
}

bool operator!=(const NeuronPoint &a, const NeuronPoint &b)
{
    return static_cast<bool>(std::memcmp(&a, &b, sizeof(NeuronPoint)));
}

bool operator==(const NeuronPoint &a, const NeuronPoint &b)
{
    return static_cast<bool>(!std::memcmp(&a, &b, sizeof(NeuronPoint)));
}

#undef ACROSS_TEMPLATE
#undef ACROSS_DEFINE

#endif
