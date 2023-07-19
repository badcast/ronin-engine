#pragma once

#include "begin.h"

// TODO: Добавить Storm алгоритм в NavMesh для поиска свободного места
// TODO: Добавить BiDirectional двунаправленный поиск

namespace RoninEngine::AI
{
    class Neuron;
    typedef std::vector<Runtime::Vec2Int> NavListSite;
    typedef std::vector<Neuron *> NavListNeuron;
    class Sheduller;
    class NavMesh;

    enum class NavStatus
    {
        Undefined,
        Locked,
        Closed,
        Opened
    };

    enum class NavIdentity
    {
        SquareMethod,
        PlusMethod,
        CrossMethod
    };

    enum class HeuristicMethod
    {
        Invalid = -1,
        NavPythagorean,
        NavManhattan,
        NavChebyshev
    };

    template <typename ResultList>
    struct NavResult
    {
        NavStatus status;
        typename ResultList::value_type from;
        typename ResultList::value_type to;
        ResultList connections;
    };

    typedef NavResult<NavListSite> NavResultSite;
    typedef NavResult<NavListNeuron> NavResultNeuron;

    struct Neuron
    {
        std::uint8_t flags; // Флаги (Содержит OpenList, ClosedList)
        std::uint32_t g;    // - стоимость пути от начальной_точки до текущей_точки
        std::uint32_t h;    // - оценка эвристики (обычно расстояние до конечной_точки)
        std::uint32_t f;    // общая оценка стоимости (f = g + h)
    };

    struct NavMeshData
    {
        std::uint32_t widthSpace;
        std::uint32_t heightSpace;
        void *neurons;
    };

    class RONIN_API NavMesh
    {
    private:
        Sheduller *shedule;

    public:
        Runtime::Vec2 worldScale;
        Runtime::Vec2 worldOffset;

        explicit NavMesh(int width, int height);

        ~NavMesh();

        void clear(bool clearLocks = false);
        void fill(bool fillLocks = false);
        void randomize(int flagFilter = 0xffffff);
        void stress();

        int width() const;
        int height() const;

        Neuron *get(int x, int y);
        Neuron *get(const Runtime::Vec2Int &point);
        Neuron *get(const Runtime::Vec2 &worldPoint);
        Neuron *get(const Runtime::Vec2 &worldPoint, Runtime::Vec2Int &outPoint);

        bool get_ncontains(const Runtime::Vec2Int &point) const;

        const Runtime::Vec2Int get_point(const Neuron *neuron) const;

        HeuristicMethod get_heuristic_method();
        bool set_heuristic_method(HeuristicMethod method);

        bool set_identity(NavIdentity newIdentity);

        // pointer with Point
        bool has_locked(const Runtime::Vec2Int &point);
        void set_lock(const Runtime::Vec2Int &point, const bool state);

        // pointer with pointer
        bool has_locked(const Neuron *neuron);
        void set_lock(const Neuron *neuron, const bool state);

        template <typename ResultList = NavResultSite>
        bool find(NavResult<ResultList> &navResult, Neuron *firstNeuron, Neuron *lastNeuron);
        template <typename ResultList = NavResultSite>
        bool find(NavResult<ResultList> &navResult, const Runtime::Vec2Int &first, const Runtime::Vec2Int &last);
        template <typename ResultList = NavResultSite>
        bool find(NavResult<ResultList> &navResult, const Runtime::Vec2 &worldPointFirst, const Runtime::Vec2 &worldPointLast);

        const Runtime::Vec2Int world_position_to_point(const Runtime::Vec2 &worldPoint);
        const Runtime::Vec2 point_to_world_position(const Runtime::Vec2Int &point);
        const Runtime::Vec2 point_to_world_position(int x, int y);
        const Runtime::Vec2 neuron_to_world_position(Neuron* neuron);

        void load(const NavMeshData &navmeshData);
        void save(NavMeshData *navmeshData);

        std::uint32_t get_cache_size();

        // TODO: add operator for Vec2Int -> get Neuron
    };
} // namespace RoninEngine::AI
