#include "ronin.h"

#include "across.hpp"

namespace RoninEngine::AIPathFinder
{
    class NavContainer : public across::basic_across_map<RoninEngine::Runtime::Vec2Int, Neuron>
    {
        friend class NavMesh;

    public:
        NavContainer(int width, int height)
            : basic_across_map(width, height)
        {
        }
    };

    NavMesh::NavMesh(int lwidth, int lheight)
    {
        worldScale = Vec2::one;
        container = GC::gc_alloc<NavContainer>(lwidth, lheight);
    }

    NavMesh::~NavMesh() { GC::gc_unalloc(container); }

    void NavMesh::Randomize(int flagFilter) { container->randomGenerate(flagFilter); }

    void NavMesh::stress() { }

    int NavMesh::Width() { return container->widthSpace; }

    int NavMesh::Height() { return container->heightSpace; }

    void NavMesh::Clear(bool clearLocks) { container->clear(clearLocks); }

    void NavMesh::Fill(bool fillLocks) { container->fill(fillLocks); }

    Neuron* NavMesh::GetNeuron(int x, int y) { container->GetNeuron(x, y); }

    Neuron* NavMesh::GetNeuron(const Runtime::Vec2Int& range) { return container->GetNeuron(range); }

    Neuron* NavMesh::GetNeuron(const Vec2& worldPoint)
    {
        Vec2Int p;
        p.x = Math::ceil(container->widthSpace / 2 + worldPoint.x / worldScale.x);
        p.y = Math::ceil(container->heightSpace / 2 - worldPoint.y / worldScale.y);
        return GetNeuron(p);
    }

    Neuron* NavMesh::GetNeuron(const Runtime::Vec2& worldPoint, Runtime::Vec2Int& outPoint)
    {
        outPoint.x = Math::ceil(container->widthSpace / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        outPoint.y = Math::ceil(container->heightSpace / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return GetNeuron(outPoint);
    }

    const Vec2Int NavMesh::WorldPointToPoint(const Vec2& worldPoint)
    {
        Vec2Int p;
        p.x = Math::ceil(container->widthSpace / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        p.y = Math::ceil(container->heightSpace / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return p;
    }

    std::uint32_t NavMesh::getCachedSize() { return container->getCachedSize(); }

    void NavMesh::Find(NavResult& navResult, NavMethodRule method, Runtime::Vec2 worldPointFirst, Runtime::Vec2 worldPointLast)
    {
        NavContainer::NavigateionResult _nr;

        container->find(_nr, across::NavMethodRule(method), this->WorldPointToPoint(worldPointFirst), WorldPointToPoint(worldPointLast));
    }

    const Runtime::Vec2 NavMesh::PointToWorldPosition(const Runtime::Vec2Int& range) { return PointToWorldPosition(range.x, range.y); }

    const Runtime::Vec2 NavMesh::PointToWorldPosition(Neuron* neuron) { return PointToWorldPosition(neuronGetPoint(neuron)); }

    const Runtime::Vec2 NavMesh::PointToWorldPosition(const int& x, const int& y)
    {
        Vec2 vec2(container->widthSpace / 2.f, container->heightSpace / 2.f);
        vec2.x = (x - vec2.x) * worldScale.x;
        vec2.y = -(y - vec2.y) * worldScale.y;
        return vec2 + worldOffset;
    }

    bool RoninEngine::AIPathFinder::NavMesh::neuronLocked(const Runtime::Vec2Int& range) { return container->neuronLocked(range); }

    bool RoninEngine::AIPathFinder::NavMesh::neuronContains(const Runtime::Vec2Int& range) { return container->neuronContains(range); }

    std::uint8_t& RoninEngine::AIPathFinder::NavMesh::neuronGetFlag(const Runtime::Vec2Int& range)
    {
        Neuron* n = GetNeuron(range);
        if (!n)
            throw std::out_of_range("range");
        return n->flags;
    }

    std::uint32_t& RoninEngine::AIPathFinder::NavMesh::neuronGetCost(const Runtime::Vec2Int& range)
    {
        Neuron* n = GetNeuron(range);
        if (!n)
            throw std::out_of_range("range");
        return n->cost;
    }

    std::uint32_t& RoninEngine::AIPathFinder::NavMesh::neuronHeuristic(const Runtime::Vec2Int& range)
    {
        Neuron* n = GetNeuron(range);
        if (!n)
            throw std::out_of_range("range");
        return n->h;
    }

    const int RoninEngine::AIPathFinder::NavMesh::neuronGetWeight(const Runtime::Vec2Int& range)
    {
        if (!neuronContains(range))
            throw std::out_of_range("range");
        return range.x * range.y + range.y * range.y;
    }

    const std::uint32_t RoninEngine::AIPathFinder::NavMesh::neuronGetTotal(const Runtime::Vec2Int& range)
    {
        Neuron* n = GetNeuron(range);
        if (!n)
            throw std::out_of_range("range");
        return n->cost + n->h;
    }

    const bool RoninEngine::AIPathFinder::NavMesh::neuronEmpty(const Runtime::Vec2Int& range) { return !neuronGetTotal(range); }

    bool NavMesh::neuronLocked(const Neuron* neuron) { return neuronLocked(neuronGetPoint(neuron)); }

    std::uint8_t& NavMesh::neuronGetFlag(const Neuron* neuron) { return neuronGetFlag(neuronGetPoint(neuron)); }

    std::uint32_t& NavMesh::neuronGetCost(const Neuron* neuron) { return neuronGetCost(neuronGetPoint(neuron)); }

    std::uint32_t& NavMesh::neuronHeuristic(const Neuron* neuron) { return neuronHeuristic(neuronGetPoint(neuron)); }

    const int NavMesh::neuronGetWeight(const Neuron* neuron) { return neuronGetWeight(neuronGetPoint(neuron)); }

    const std::uint32_t NavMesh::neuronGetTotal(const Neuron* neuron) { return neuronEmpty(neuronGetPoint(neuron)); }

    const bool NavMesh::neuronEmpty(const Neuron* neuron) { return neuronEmpty(neuronGetPoint(neuron)); }

    void NavMesh::neuronLock(const Neuron* neuron, const bool state) { neuronLock(neuronGetPoint(neuron), state); }

    const Vec2Int NavMesh::neuronGetPoint(const Neuron* neuron) { return container->neuronGetPoint(neuron); }

    void RoninEngine::AIPathFinder::NavMesh::neuronLock(const Runtime::Vec2Int& range, const bool state) { container->neuronLock(range, state); }

    void NavMesh::load(const NavMeshData& navmeshData) { container->load(*reinterpret_cast<const across::AcrossData*>(&navmeshData)); }

    void NavMesh::save(NavMeshData* navmeshData) { container->save(reinterpret_cast<across::AcrossData*>(navmeshData)); }

    void NavMesh::Find(NavResult& navResult, NavMethodRule method, Neuron* firstNeuron, Neuron* lastNeuron) { Find(navResult, method, neuronGetPoint(firstNeuron), neuronGetPoint(lastNeuron)); }

    void NavMesh::Find(NavResult& navResult, NavMethodRule method, Vec2Int first, Vec2Int last)
    {
        NavContainer::NavigateionResult _rt;
        container->find(_rt, static_cast<across::NavMethodRule>(method), first, last);
        navResult.firstNeuron = _rt.firstNeuron;
        navResult.lastNeuron = _rt.lastNeuron;
        navResult.map = this;
        navResult.RelativePaths = std::move(_rt.RelativePaths);
        navResult.status = static_cast<NavStatus>(_rt.status);
    }

} // namespace RoninEngine::AIPathFinder
