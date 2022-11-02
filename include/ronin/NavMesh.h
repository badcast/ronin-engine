#pragma once

#include "dependency.h"

// TODO: Добавить Storm алгоритм в NavMesh

namespace RoninEngine::AIPathFinder {

class NavMesh;

class Neuron;

enum NavStatus { Undefined, Locked, Closed, Opened };

// TODO: Дополнить список алгоритмов поиска путей
enum NavAlgorithm { AStar };

enum NavMethodRule { NavigationIntelegency, PlusMethod, SquareMethod, CrossMethod };

struct NavResult {
    NavStatus status;
    NavAlgorithm algorithm;
    Runtime::Vec2Int firstNeuron;
    Runtime::Vec2Int lastNeuron;
    std::list<Runtime::Vec2Int> RelativePaths;
    NavMesh *map;
};

struct Neuron {
    std::uint8_t flags;
    std::uint32_t h;
    std::uint32_t cost;
};

struct NavMeshData {
    std::uint32_t widthSpace;
    std::uint32_t heightSpace;
    void *neurons;
};

constexpr std::size_t NavMeshDataSizeMultiplier = sizeof(Neuron);

class NavMesh {
    void *neurons;
    int segmentOffset;

   protected:
    int widthSpace, heightSpace;

   public:
    Runtime::Vec2 worldScale;
    Runtime::Vec2 worldOffset;

    explicit NavMesh(int width, int height);

    ~NavMesh();

    void clear(bool clearLocks = false);
    void fill(bool fillLocks = false);
    void randomGenerate(int flagFilter = 0xffffff);
    void stress();

    int getWidth();
    int getHeight();

    Neuron *GetNeuron(const Runtime::Vec2Int &point);
    Neuron *GetNeuron(const Runtime::Vec2 &worldPoint);
    Neuron *GetNeuron(const Runtime::Vec2 &worldPoint, Runtime::Vec2Int &outPoint);

    inline bool neuronContains(const Runtime::Vec2Int &point);

    inline const Runtime::Vec2Int neuronGetPoint(const Neuron *neuron);

    //pointer with Point
    inline bool neuronLocked(const Runtime::Vec2Int &point);
    inline std::uint8_t &neuronGetFlag(const Runtime::Vec2Int &point);
    inline std::uint32_t &neuronGetCost(const Runtime::Vec2Int &point);
    inline std::uint32_t &neuronHeuristic(const Runtime::Vec2Int &point);
    inline const int neuronGetWeight(const Runtime::Vec2Int &point);
    inline const std::uint32_t neuronGetTotal(const Runtime::Vec2Int &point);
    inline const bool neuronEmpty(const Runtime::Vec2Int &point);
    void neuronLock(const Runtime::Vec2Int &point, const bool state);

    //pointer with pointer
    inline bool neuronLocked(const Neuron *neuron);
    inline std::uint8_t &neuronGetFlag(const Neuron *neuron);
    inline std::uint32_t &neuronGetCost(const Neuron *neuron);
    inline std::uint32_t &neuronHeuristic(const Neuron *neuron);
    inline const int neuronGetWeight(const Neuron *neuron);
    inline const std::uint32_t neuronGetTotal(const Neuron *neuron);
    inline const bool neuronEmpty(const Neuron *neuron);
    void neuronLock(const Neuron *neuron, const bool state);

    inline const Runtime::Vec2Int WorldPointToPoint(const RoninEngine::Runtime::Vec2 &worldPoint);

    void find(NavResult &navResult, NavMethodRule method, Runtime::Vec2 worldPointFirst, Runtime::Vec2 worldPointLast);

    void find(NavResult &navResult, NavMethodRule method, Runtime::Vec2Int firstNeuron, Runtime::Vec2Int lastNeuron,
              NavAlgorithm algorithm);

    const RoninEngine::Runtime::Vec2 PointToWorldPosition(const Runtime::Vec2Int &point);
    const RoninEngine::Runtime::Vec2 PointToWorldPosition(Neuron *neuron);
    const RoninEngine::Runtime::Vec2 PointToWorldPosition(const int &x, const int &y);
    void load(const NavMeshData &navmeshData);
    void save(NavMeshData *navmeshData);

    std::uint32_t getCachedSize();
};

class AlgorithmUtils {
   public:
    ///Определяет дистанцию точки от A до точки B
    ///Используется формула Пифагора "(a^2) + (b^2) = c^2"
    ///\par lhs Первоначальная точка
    ///\par rhs Конечная точка
    ///\return Сумма
    static inline int DistancePhf(const Runtime::Vec2Int &lhs, const Runtime::Vec2Int &rhs);

    ///Определяет дистанцию точки от A до точки B
    ///\par lhs Первоначальная точка
    ///\par rhs Конечная точка
    ///\return Сумма
    static inline int DistanceManht(const Runtime::Vec2Int &lhs, const Runtime::Vec2Int &rhs);

    /// Определяет, минимальную стоимость
    static auto GetMinCostPath(NavMesh &map, std::list<Runtime::Vec2Int> *paths) -> decltype(std::begin(*paths));

    /// Определяет, функцию пойска по направлениям. Таких как: left, up, right,
    static void AvailPoints(NavMesh &map, NavMethodRule method, Runtime::Vec2Int arrange, Runtime::Vec2Int target,
                            std::list<Runtime::Vec2Int> *pathTo, std::size_t maxCount = -1, int filterFlag = -1);
};

}  // namespace RoninEngine::AIPathFinder
