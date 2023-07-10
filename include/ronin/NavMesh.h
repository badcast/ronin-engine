#pragma once

#include "begin.h"

// TODO: Добавить Storm алгоритм в NavMesh для поиска свободного места
// TODO: Добавить BiDirectional двунаправленный поиск

namespace RoninEngine::AI
{

    class Neuron;
    class NavContainer;
    class NavMesh;

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

    struct NavResult
    {
        NavStatus status;
        Runtime::Vec2Int firstNeuron;
        Runtime::Vec2Int lastNeuron;
        std::list<Runtime::Vec2Int> RelativePaths;
        NavMesh* map;
    };

    struct Neuron
    {
        std::uint8_t flags;
        std::uint32_t h;
        std::uint32_t cost;
    };

    struct NavMeshData
    {
        std::uint32_t widthSpace;
        std::uint32_t heightSpace;
        void* neurons;
    };

    class RONIN_API NavMesh
    {
    private:
        NavContainer* shedule;

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

        Neuron* get_neuron(int x, int y);
        Neuron* get_neuron(const Runtime::Vec2Int& point);
        Neuron* get_neuron(const Runtime::Vec2& worldPoint);
        Neuron* get_neuron(const Runtime::Vec2& worldPoint, Runtime::Vec2Int& outPoint);

        bool get_ncontains(const Runtime::Vec2Int& point) const;

        const Runtime::Vec2Int get_npoint(const Neuron* neuron) const;

        // pointer with Point
        bool get_nlocked(const Runtime::Vec2Int& point);
        std::uint8_t& get_nflag(const Runtime::Vec2Int& point);
        std::uint32_t& get_ncost(const Runtime::Vec2Int& point);
        std::uint32_t& get_nheuristic(const Runtime::Vec2Int& point);
        const int get_nweight(const Runtime::Vec2Int& point);
        const std::uint32_t get_ntotal(const Runtime::Vec2Int& point);
        const bool get_nempty(const Runtime::Vec2Int& point);
        void get_nlock(const Runtime::Vec2Int& point, const bool state);

        // pointer with pointer
        bool get_nlocked(const Neuron* neuron);
        std::uint8_t& get_nflag(const Neuron* neuron);
        std::uint32_t& get_ncost(const Neuron* neuron);
        std::uint32_t& get_nheuristic(const Neuron* neuron);
        const int get_nweight(const Neuron* neuron);
        const std::uint32_t get_ntotal(const Neuron* neuron);
        const bool get_nempty(const Neuron* neuron);
        void get_nlock(const Neuron* neuron, const bool state);

        void find(NavResult& navResult, NavMethodRule method, Runtime::Vec2 worldPointFirst, Runtime::Vec2 worldPointLast);
        void find(NavResult& navResult, NavMethodRule method, Neuron* firstNeuron, Neuron* lastNeuron);
        void find(NavResult& navResult, NavMethodRule method, Runtime::Vec2Int first, Runtime::Vec2Int last);

        const Runtime::Vec2Int world_position_to_point(const Runtime::Vec2& worldPoint);
        const Runtime::Vec2 point_to_world_position(const Runtime::Vec2Int& point);
        const Runtime::Vec2 point_to_world_position(Neuron* neuron);
        const Runtime::Vec2 point_to_world_position(const int& x, const int& y);
        void load(const NavMeshData& navmeshData);
        void save(NavMeshData* navmeshData);

        std::uint32_t get_cache_size();
    };
} // namespace RoninEngine::AIPathFinder
