#pragma once

#include "begin.h"

// TODO: Добавить Storm алгоритм в NavMesh для поиска свободного места
// TODO: Добавить BiDirectional двунаправленный поиск

namespace RoninEngine::AIPathFinder
{

    class Neuron;
    class NavContainer;
    class NavMesh;

    enum class NavStatus { Undefined, Locked, Closed, Opened };

    enum class NavMethodRule { NavigationIntelegency, PlusMethod, SquareMethod, CrossMethod };

    struct NavResult {
        NavStatus status;
        Runtime::Vec2Int firstNeuron;
        Runtime::Vec2Int lastNeuron;
        std::list<Runtime::Vec2Int> RelativePaths;
        NavMesh* map;
    };

    struct Neuron {
        std::uint8_t flags;
        std::uint32_t h;
        std::uint32_t cost;
    };

    struct NavMeshData {
        std::uint32_t widthSpace;
        std::uint32_t heightSpace;
        void* neurons;
    };

    class RONIN_API NavMesh
    {
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

        int width();
        int height();

        Neuron* get_neuron(int x, int y);
        Neuron* get_neuron(const Runtime::Vec2Int& point);
        Neuron* get_neuron(const Runtime::Vec2& worldPoint);
        Neuron* get_neuron(const Runtime::Vec2& worldPoint, Runtime::Vec2Int& outPoint);

        bool neuronContains(const Runtime::Vec2Int& point);

        const Runtime::Vec2Int neuronGetPoint(const Neuron* neuron);

        // pointer with Point
        bool neuronLocked(const Runtime::Vec2Int& point);
        std::uint8_t& neuronGetFlag(const Runtime::Vec2Int& point);
        std::uint32_t& neuronGetCost(const Runtime::Vec2Int& point);
        std::uint32_t& neuronHeuristic(const Runtime::Vec2Int& point);
        const int neuronGetWeight(const Runtime::Vec2Int& point);
        const std::uint32_t neuronGetTotal(const Runtime::Vec2Int& point);
        const bool neuronEmpty(const Runtime::Vec2Int& point);
        void neuronLock(const Runtime::Vec2Int& point, const bool state);

        // pointer with pointer
        bool neuronLocked(const Neuron* neuron);
        std::uint8_t& neuronGetFlag(const Neuron* neuron);
        std::uint32_t& neuronGetCost(const Neuron* neuron);
        std::uint32_t& neuronHeuristic(const Neuron* neuron);
        const int neuronGetWeight(const Neuron* neuron);
        const std::uint32_t neuronGetTotal(const Neuron* neuron);
        const bool neuronEmpty(const Neuron* neuron);
        void neuronLock(const Neuron* neuron, const bool state);

        void find(NavResult& navResult, NavMethodRule method, Runtime::Vec2 worldPointFirst, Runtime::Vec2 worldPointLast);
        void find(NavResult& navResult, NavMethodRule method, Neuron* firstNeuron, Neuron* lastNeuron);
        void find(NavResult& navResult, NavMethodRule method, Runtime::Vec2Int first, Runtime::Vec2Int last);

        const Runtime::Vec2Int world_position_to_point(const Runtime::Vec2& worldPoint);
        const Runtime::Vec2 point_to_world_position(const Runtime::Vec2Int& point);
        const Runtime::Vec2 point_to_world_position(Neuron* neuron);
        const Runtime::Vec2 point_to_world_position(const int& x, const int& y);
        void load(const NavMeshData& navmeshData);
        void save(NavMeshData* navmeshData);

        std::uint32_t getCachedSize();
    };
} // namespace RoninEngine::AIPathFinder
