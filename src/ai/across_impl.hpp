// declarations Across Map (algorithm) part-file
#ifndef _ACROSS_IMPL_HPP_
#define _ACROSS_IMPL_HPP_

// TODO: Make Bi-Directional

#include "across.hpp"

#if not defined(ACROSS_TEMPLATE)
#error "main header not included"
#endif

enum
{
    ByteSize = 8
};

enum
{
    // Neutral Neuron
    NEURON_CAPTURE_RELEASED = 0,
    // Captured opened Neuron
    NEURON_CAPTURE_OPEN_LIST = 1,
    // Captured closed Neuron
    NEURON_CAPTURE_CLOSED_LIST = 2,
    // Tiled (Real path from-to)
    NUERON_CAPTURE_ROAD_LIST = 4
};

class brain_range_error : public std::range_error
{
public:
    brain_range_error() : std::range_error("range")
    {
    }
};

ACROSS_TEMPLATE
class immune_system
{
public:
    /**
     * Pythagorean distance.
     */
    static weight_t heuristic_pythagorean(weight_t dx, weight_t dy)
    {
        return dx * dx + dy * dy;
    }
    /**
     * Manhattan distance.
     */
    static weight_t heuristic_manhtattan(weight_t dx, weight_t dy)
    {
        return dx + dy;
    }
    /**
     * Chebyshev distance.
     */
    static weight_t heuristic_chebyshev(weight_t dx, weight_t dy)
    {
        return std::max(dx, dy);
    }
    /**
     * Octile distance.
     */
    static weight_t heuristic_octile(weight_t dx, weight_t dy)
    {
        static const float F = std::sqrt(2.f) - 1;
        return (dx < dy) ? F * dx + dy : F * dy + dx;
    }

    // main comparator for hash_list
    static bool compare_neuron(const INeuron *lhs, const INeuron *rhs)
    {
        return lhs->f < rhs->f;
    }

    template <typename IdentityWrapper>
    static int get_matrix(MatrixIdentity matrixIdentity, IdentityWrapper &place)
    {
        /*
         DIAGONAL

         * * *
         * x *
         * * *

        */
        static std::int8_t M_DIAGONAL_H_POINT[] {-1, 0, 1, -1, 1, -1, 0, 1};
        static std::int8_t M_DIAGONAL_V_POINT[] {-1, -1, -1, 0, 0, 1, 1, 1};
        static std::int8_t M_G_DIAGONAL_WEIGHT[] {2, 1, 2, 1, 1, 2, 1, 2};
        /*
         PLUS

           *
         * x *
           *

        */
        static std::int8_t M_PLUS_H_POINT[] {-1, 1, 0, 0};
        static std::int8_t M_PLUS_V_POINT[] {0, 0, -1, 1};
        static std::int8_t M_G_PLUS_WEIGHT[] {1, 1, 1, 1};
        /*
         CROSS

         *   *
           x
         *   *

        */
        static std::int8_t M_CROSS_H_POINT[] {-1, 1, -1, 1};
        static std::int8_t M_CROSS_V_POINT[] {-1, -1, 1, 1};

        switch(matrixIdentity)
        {
            case MatrixIdentity::DiagonalMethod:
                place.horizontal = M_DIAGONAL_H_POINT;
                place.vertical = M_DIAGONAL_V_POINT;
                place.g_weight = M_G_DIAGONAL_WEIGHT;
                return place.length = sizeof(M_DIAGONAL_H_POINT);
                break;
            case MatrixIdentity::PlusMethod:
                place.horizontal = M_PLUS_H_POINT;
                place.vertical = M_PLUS_V_POINT;
                place.g_weight = M_G_PLUS_WEIGHT;
                return place.length = sizeof(M_PLUS_H_POINT);
                break;
            case MatrixIdentity::CrossMethod:
                place.horizontal = M_CROSS_H_POINT;
                place.vertical = M_CROSS_V_POINT;
                place.g_weight = M_G_PLUS_WEIGHT;
                return place.length = sizeof(M_CROSS_H_POINT);
                break;
        }
        return 0;
    }
};

ACROSS_TEMPLATE
ACROSS_DEFINE::basic_brain_map(std::uint32_t xlength, std::uint32_t ylength)
{
    if(1 > xlength || 1 > ylength)
        throw std::runtime_error("dimensions is zero");

    this->_xsize = xlength;
    this->_ysize = ylength;

    // The Identity vector lines
    set_identity(MatrixIdentity::DiagonalMethod);
    // The Pythagorean heuristic is set as default
    set_heuristic(HeuristicMethod::Pythagorean);
    // Alloc
    _internal_realloc();
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::_internal_realloc()
{
    std::size_t xlength = _xsize, ylength = _ysize;

    if(this->neurons != nullptr)
    {
        std::free(this->neurons);
    }

    std::div_t lockedDiv = div(std::max<std::uint32_t>(ylength = (xlength * ylength), ByteSize), ByteSize); // add locked bits
    this->_seg_off = xlength = lockedDiv.quot + lockedDiv.rem;
    xlength = ylength * sizeof(INeuron) + xlength;
    this->neurons = static_cast<INeuron *>(std::malloc(xlength));

    if(this->neurons == nullptr)
        throw std::bad_alloc();

    // clear allocated data to ZERO
    clear(true);
}

ACROSS_TEMPLATE
ACROSS_DEFINE::~basic_brain_map()
{
    if(this->neurons != nullptr)
        std::free(this->neurons);
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::randomize_hardware(int flagFilter)
{
    std::uint32_t lhs, rhs = _seg_off;
    clear(true);
    do
    {
        lhs = static_cast<std::uint32_t>(rand() & flagFilter);
        memcpy(
            reinterpret_cast<void *>(reinterpret_cast<std::size_t>(neurons) + _seg_off - rhs),
            &lhs,
            std::min(rhs, (std::uint32_t) sizeof(long)));
        rhs -= std::min(rhs, static_cast<std::uint32_t>(sizeof(long)));
    } while(rhs > 0);
}

ACROSS_TEMPLATE
std::size_t ACROSS_DEFINE::size()
{
    return static_cast<std::size_t>(_xsize) * _ysize;
}

ACROSS_TEMPLATE
int ACROSS_DEFINE::reserved_bits()
{
    int bits = size() % ByteSize;
    if(bits)
        bits = ByteSize - bits;
    return bits;
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::set_identity(MatrixIdentity matrixIdentity)
{
    int len;
    len = immune_system::get_matrix(matrixIdentity, this->identity);
    return len > 0;
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::set_heuristic(HeuristicMethod method)
{
    bool status = true;
    switch(method)
    {
        case HeuristicMethod::Pythagorean:
            __heuristic__ = immune_system::heuristic_pythagorean;
            break;
        case HeuristicMethod::Manhattan:
            __heuristic__ = immune_system::heuristic_manhtattan;
            break;
        case HeuristicMethod::Chebyshev:
            __heuristic__ = immune_system::heuristic_chebyshev;
            break;
        case HeuristicMethod::Octile:
            __heuristic__ = immune_system::heuristic_octile;
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
    if(__heuristic__ == immune_system::heuristic_pythagorean)
        return HeuristicMethod::Pythagorean;
    else if(__heuristic__ == immune_system::heuristic_manhtattan)
        return HeuristicMethod::Manhattan;
    else if(__heuristic__ == immune_system::heuristic_chebyshev)
        return HeuristicMethod::Chebyshev;
    else if(__heuristic__ == immune_system::heuristic_octile)
        return HeuristicMethod::Octile;
    else
        return HeuristicMethod::Invalid;
}

ACROSS_TEMPLATE
std::uint32_t ACROSS_DEFINE::get_width()
{
    return _xsize;
}

ACROSS_TEMPLATE
std::uint32_t ACROSS_DEFINE::get_height()
{
    return _ysize;
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::clear(bool clearLocks)
{
    std::uint32_t length = _xsize * _ysize * sizeof(INeuron);
    std::uint32_t leftOffset;
    if(clearLocks)
    {
        leftOffset = 0;
        length += this->_seg_off;
    }
    else
    {
        leftOffset = this->_seg_off;
    }
    memset(reinterpret_cast<void *>(reinterpret_cast<std::size_t>(neurons) + leftOffset), 0, length);
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::fill(bool fillLocks)
{
    std::uint32_t length = _xsize * _ysize * sizeof(INeuron);
    std::uint32_t leftoffset;
    if(fillLocks)
    {
        leftoffset = 0;
        length += this->_seg_off;
    }
    else
    {
        leftoffset = this->_seg_off;
    }
    memset(reinterpret_cast<void *>(reinterpret_cast<std::size_t>(neurons) + leftoffset), 0xff, length);
}

ACROSS_TEMPLATE
INeuron *ACROSS_DEFINE::get(const ISite &range)
{
    INeuron *result = nullptr;
    if(contains(range))
        result =
            reinterpret_cast<INeuron *>(reinterpret_cast<std::size_t>(neurons) + _seg_off + (range.x * _ysize + range.y) * sizeof(INeuron));
    return result;
}

ACROSS_TEMPLATE
std::size_t ACROSS_DEFINE::get_cached_size()
{
    std::size_t cal = 0;

    INeuron *n = front();
    INeuron *ne = back() + 1;
    while(n < ne)
    {
        cal += n->f;
        ++n;
    }
    return cal;
}

ACROSS_TEMPLATE
INeuron *ACROSS_DEFINE::front()
{
    return reinterpret_cast<INeuron *>(reinterpret_cast<std::size_t>(neurons) + _seg_off);
}

ACROSS_TEMPLATE
INeuron *ACROSS_DEFINE::back()
{
    return front() + (size() - 1);
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::has_lock(const ISite &range)
{
    auto divide = std::div(range.x * std::size_t(_ysize) + range.y, ByteSize);
    auto pointer = reinterpret_cast<std::uint8_t *>(neurons) + divide.quot;
    return (*pointer) & (1 << divide.rem);
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::contains(const ISite &range)
{
    return (static_cast<std::uint32_t>(range.x) < _xsize && static_cast<std::uint32_t>(range.y) < _ysize && range.x > ~0 && range.y > ~0);
}

ACROSS_TEMPLATE
const ISite ACROSS_DEFINE::get_point(const INeuron *neuron)
{
    auto divide = std::div((std::size_t(neuron) - std::size_t(neurons) - _seg_off) / sizeof(INeuron), (int) _ysize);
    return {divide.quot, divide.rem};
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::set_lock(const ISite &range, const bool state)
{
    auto divide = std::div(range.x * std::size_t(_ysize) + range.y, ByteSize);
    auto pointer = (reinterpret_cast<std::uint8_t *>(neurons)) + divide.quot;
    divide.quot = (1 << divide.rem);
    // FIXME: Optimize to ~divide.quout (xor)
    // Reset bit
    (*pointer) ^= (*pointer) & (divide.quot);
    // Write bit
    (*pointer) |= divide.quot * (state == true);
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::set_lock(const INeuron *neuron, const bool state)
{
    auto divide = std::lldiv(static_cast<std::size_t>(neuron - front()), ByteSize);
    auto pointer = (reinterpret_cast<std::uint8_t *>(neurons)) + divide.quot;
    divide.quot = (1 << divide.rem);
    // FIXME: Optimize to ~divide.quout (xor)
    // Reset bit
    (*pointer) ^= (*pointer) & (divide.quot);
    // Write bit
    (*pointer) |= divide.quot * (state == true);
}

ACROSS_TEMPLATE
INeuron *ACROSS_DEFINE::get(int x, int y)
{
    return get({x, y});
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::contains(const INeuron *neuron)
{
    return contains(get_point(neuron));
}

ACROSS_TEMPLATE
bool ACROSS_DEFINE::has_lock(const INeuron *neuron)
{
    return has_lock(get_point(neuron));
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::load(const brain_breakfast &breakfast)
{
    if(!breakfast.widthSpace || !breakfast.heightSpace)
        throw std::runtime_error("Argument param, width or height is empty");

    if(breakfast.data == nullptr)
        throw std::runtime_error("pointer have null");

    if(this->neurons == breakfast.data)
        throw std::runtime_error("neurons == extrim, unflow effect");

    this->_xsize = breakfast.widthSpace;
    this->_ysize = breakfast.heightSpace;
    this->_seg_off = breakfast.len;

    _internal_realloc();

    std::memcpy(this->neurons, breakfast.data, breakfast.len);
}

ACROSS_TEMPLATE
void ACROSS_DEFINE::save(brain_breakfast *save_to)
{
    if(save_to == nullptr)
    {
        throw std::runtime_error("null pointer");
    }
    save_to->widthSpace = this->_xsize;
    save_to->heightSpace = this->_ysize;
    save_to->len = _seg_off;
    save_to->data = std::malloc(_seg_off);
    if(save_to->data == nullptr)
    {
        throw std::runtime_error("out of mem");
    }
    std::memcpy(save_to->data, this->neurons, _seg_off);
}

ACROSS_TEMPLATE
template <typename ListType>
bool ACROSS_DEFINE::find(navigate_result<ListType> &navigationResult, const ISite &first, const ISite &last)
{
    return this->find(navigationResult, this->get(first), this->get(last));
}

ACROSS_TEMPLATE
template <typename ListType>
bool ACROSS_DEFINE::find(navigate_result<ListType> &navigationResult, INeuron *firstNeuron, INeuron *lastNeuron)
{

    using navigate_result_t = navigate_result<ListType>;
    using target_type = typename ListType::value_type;
    // site state
#define BLOCK_SITE                                   \
    if constexpr(std::is_same_v<target_type, ISite>) \
    {
    // neuron state
#define BLOCK_NEURON \
    }                \
    else if constexpr(std::is_same_v<target_type, INeuron>) {
#define BLOCK_END }

    // OpenList for algorithm, auto sort, builds, and combines
    std::multiset<INeuron *, decltype(&immune_system::compare_neuron)> openList(&immune_system::compare_neuron);
    /*Алгоритм A* (A-Star) используется для поиска кратчайшего пути в графе с весами на ребрах. Он комбинирует эффективность алгоритма
    Дейкстры и эвристическую функцию для оценки оставшегося расстояния до цели. Вот пример псевдокода A*:

    ASTAR Pseudocode
    Функция AStar(start, goal):
    Открытый_список = пустой список с приоритетами (min-heap)
    Закрытый_список = пустой список
    Добавить start в Открытый_список с оценкой F(start) = G(start) + H(start)  // G(start) - расстояние от начальной вершины до текущей,
    H(start) - эвристическая оценка от текущей до цели
    while Открытый_список не пуст:
        Текущая_вершина = вершина с минимальной оценкой F в Открытом_списке
        Удалить Текущая_вершина из Открытый_список
        Добавить Текущая_вершина в Закрытый_список

        if Текущая_вершина = goal:
            // Путь найден
            Вернуть построенный путь

        for каждое соседнее_ребро от Текущая_вершина:
            if соседнее_ребро в Закрытый_список:
                Пропустить это соседнее_ребро

            if соседнее_ребро не в Открытый_список:
                Добавить соседнее_ребро в Открытый_список
                Установить родителя соседнего_ребра равным Текущая_вершина
                Рассчитать оценки G и H для соседнего_ребра
                Установить оценку F для соседнего_ребра: F(соседнее_ребро) = G(соседнее_ребро) + H(соседнее_ребро)
            else:
                // соседнее_ребро уже есть в Открытом_списке, проверяем, является ли новый путь лучше
                Если G(Текущая_вершина) + стоимость(Текущая_вершина, соседнее_ребро) < G(соседнее_ребро):
                    Обновить родителя соседнего_ребра равным Текущая_вершина
                    Рассчитать оценки G и H для соседнего_ребра
                    Установить оценку F для соседнего_ребра: F(соседнее_ребро) = G(соседнее_ребро) + H(соседнее_ребро)

    // Нет пути до цели
    Вернуть пустой список
    Заметки:

        G(вершина) представляет собой стоимость пути от начальной вершины до данной вершины.
        H(вершина) представляет собой эвристическую оценку расстояния от данной вершины до целевой вершины.
        F(вершина) = G(вершина) + H(вершина) - общая оценка вершины, которая учитывает как стоимость пути от начальной вершины, так и
    эвристическую оценку. Список с приоритетами (min-heap) в Открытом_списке используется для выбора вершины с наименьшей оценкой F. Это
    обеспечивает, что мы исследуем более перспективные пути в первую очередь.

    Примечание: Псевдокод предоставляет общую структуру алгоритма, и его точная реализация может отличаться в зависимости от языка
    программирования и используемых структур данных. Также необходимо определить функции оценки H и стоимости ребер для конкретной задачи.
    */

    if(firstNeuron == nullptr || lastNeuron == nullptr)
    {
        navigationResult.status = NavigationStatus::Undefined;
        return false;
    }
    if(has_lock(firstNeuron) || has_lock(lastNeuron))
    {
        navigationResult.status = NavigationStatus::Locked;
        return false;
    }

    INeuron *current, *self;
    typename decltype(openList)::iterator cur_iter;
    ISite first_site = get_point(firstNeuron), last_site = get_point(lastNeuron), self_site, current_site;

    navigationResult.status = NavigationStatus::Closed;

    BLOCK_SITE
    navigationResult.from = first_site;
    navigationResult.to = last_site;
    BLOCK_NEURON
    navigationResult.from = firstNeuron;
    navigationResult.to = lastNeuron;
    BLOCK_END

    firstNeuron->g = 0;
    firstNeuron->h = __heuristic__(first_site.x - last_site.x, first_site.y - last_site.y);
    firstNeuron->f = firstNeuron->h;
    firstNeuron->parent = nullptr;

    openList.insert(firstNeuron);

    // Перестройка
    while(openList.empty() == false)
    {
        cur_iter = openList.begin();
        current = *cur_iter;
        if(current == lastNeuron)
        {
            navigationResult.status = NavigationStatus::Opened;

            int size = 0;
            while(current && ++size)
                current = current->parent;

            current = *cur_iter;
            navigationResult.connections.resize(size);
            while(current)
            {
                --size;
                BLOCK_SITE
                navigationResult.connections[size] = std::move(get_point(current));
                BLOCK_NEURON
                navigationResult.connections[size] = std::move(current);
                BLOCK_END
                current = current->parent;
            }
            // close and combine finded connections
            break;
        }
        openList.erase(cur_iter);
        // change to closied list
        current->flags = NEURON_CAPTURE_CLOSED_LIST;

        current_site = get_point(current);

        for(int s = 0; s < identity.length; ++s)
        {
            // Get the closest neurons
            self_site.x = current_site.x + identity.horizontal[s];
            self_site.y = current_site.y + identity.vertical[s];
            self = get(self_site);
            // self is exits
            // self is not are closed list
            // self is not locked
            if(self == nullptr || self->flags == NEURON_CAPTURE_CLOSED_LIST || has_lock(self_site))
            {
                continue;
            }

            int newG = current->g + identity.g_weight[s]; // get weight
            if(self->flags == NEURON_CAPTURE_OPEN_LIST)
            {
                // Изменяем захваченный нейрон.
                if(newG < self->g)
                {
                    auto pair_range = openList.equal_range(self);
                    openList.erase(std::find(pair_range.first, pair_range.second, self));
                }
                else
                    continue;
            }
            else
                // Capture unresearched Neuron
                // Захватываем нейтральный нейрон и помещаем ее в открытый список
                self->flags = NEURON_CAPTURE_OPEN_LIST;

            self->parent = current;

            self->g = newG;
            // calculate heuristic
            self->h = __heuristic__(self_site.x - last_site.x, self_site.y - last_site.y);
            // compute total weight
            self->f = self->g + self->h;

            // Capture
            openList.insert(self);
        }
    }

    return navigationResult.status == NavigationStatus::Opened;
#undef BLOCK_SITE
#undef BLOCK_NEURON
#undef BLOCK_END
}

bool operator!=(const the_site &a, const the_site &b)
{
    return static_cast<bool>(std::memcmp(&a, &b, sizeof(the_site)));
}

bool operator==(const the_site &a, const the_site &b)
{
    return static_cast<bool>(!std::memcmp(&a, &b, sizeof(the_site)));
}

#endif
