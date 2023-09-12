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
        DiagonalMethod = 0,
        PlusMethod = 1,
        CrossMethod = 2
    };

    enum class HeuristicMethod
    {
        Invalid = -1,
        NavPythagorean = 0,
        NavManhattan = 1,
        NavChebyshev = 2,
        NavOctile = 3
    };

    enum class MazeAlgorithm
    {
        RecursiveBacktracer = 0,
        RecursiveDivison = 1,
        SelectiveStormChaos = 2,
        ForcedChaos = 3
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
        std::uint8_t flags;
        std::uint32_t g;
        std::uint32_t h;
        std::uint32_t f;
        Neuron *parent;
    };

    struct NavMeshData
    {
        std::uint32_t widthSpace;
        std::uint32_t heightSpace;
        int len;
        void *data;
        NavMeshData() : widthSpace(0), heightSpace(0), len(0), data(nullptr)
        {
        }
    };

    class RONIN_API NavMesh
    {
    private:
        Sheduller *shedule;

    public:
        Runtime::Vec2 worldScale;
        Runtime::Vec2 worldOffset;

        explicit NavMesh(std::uint32_t width, std::uint32_t height);

        ~NavMesh();

        void Clear(bool clearLocks = false);
        void fillAll(bool fillLocks = false);
        void Shuffle(int flagFilter = 0xffffff);

        void GenerateMaze(MazeAlgorithm algorithm = MazeAlgorithm::RecursiveBacktracer);

        std::uint32_t getWidth() const;
        std::uint32_t getHeight() const;

        Neuron *get(int x, int y);
        Neuron *get(const Runtime::Vec2Int &point);
        Neuron *get(const Runtime::Vec2 &worldPoint);
        Neuron *get(const Runtime::Vec2 &worldPoint, Runtime::Vec2Int &outPoint);

        bool Contains(const Runtime::Vec2Int &point) const;

        const Runtime::Vec2Int getPoint(const Neuron *neuron) const;

        HeuristicMethod getHeuristic();
        bool setHeuristic(HeuristicMethod method);

        bool setIdentity(NavIdentity newIdentity);
        NavIdentity getIdentity();

        NavListSite GetNeighbours(NavIdentity identity, const Runtime::Vec2Int &pick);
        NavListNeuron GetNeighbours(NavIdentity identity, const Neuron *pick);

        // pointer with Point
        bool hasLocked(const Runtime::Vec2Int &point);
        void setLock(const Runtime::Vec2Int &point, const bool state);

        // pointer with pointer
        bool hasLocked(const Neuron *neuron);
        void setLock(const Neuron *neuron, const bool state);

        bool Find(NavResultSite &navResult, Neuron *firstNeuron, Neuron *lastNeuron);
        bool Find(NavResultSite &navResult, const Runtime::Vec2Int &first, const Runtime::Vec2Int &last);
        bool Find(NavResultSite &navResult, const Runtime::Vec2 &worldPointFirst, const Runtime::Vec2 &worldPointLast);

        bool Find(NavResultNeuron &navResult, Neuron *firstNeuron, Neuron *lastNeuron);
        bool Find(NavResultNeuron &navResult, const Runtime::Vec2Int &first, const Runtime::Vec2Int &last);
        bool Find(NavResultNeuron &navResult, const Runtime::Vec2 &worldPointFirst, const Runtime::Vec2 &worldPointLast);

        const Runtime::Vec2Int WorldPointToPoint(const Runtime::Vec2 &worldPoint);
        const Runtime::Vec2 PointToWorldPoint(const Runtime::Vec2Int &point);
        const Runtime::Vec2 PointToWorldPoint(int x, int y);
        const Runtime::Vec2 NeuronToWorldPoint(Neuron *neuron);

        void LoadFrom(const NavMeshData &navmeshData);
        void SaveTo(NavMeshData *navmeshData);

        std::uint32_t getCached();

        // TODO: add operator for Vec2Int -> get Neuron
    };
} // namespace RoninEngine::AI
