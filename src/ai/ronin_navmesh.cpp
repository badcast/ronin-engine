﻿#include "ronin.h"

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
        shedule = RoninMemory::alloc<NavContainer>(lwidth, lheight);
    }

    NavMesh::~NavMesh() { RoninMemory::free(shedule); }

    void NavMesh::randomize(int flagFilter) { shedule->randomize(flagFilter); }

    void NavMesh::stress() { }

    int NavMesh::width() { return shedule->widthSpace; }

    int NavMesh::height() { return shedule->heightSpace; }

    void NavMesh::clear(bool clearLocks) { shedule->clear(clearLocks); }

    void NavMesh::fill(bool fillLocks) { shedule->fill(fillLocks); }

    Neuron* NavMesh::get_neuron(int x, int y) { shedule->get(x, y); }

    Neuron* NavMesh::get_neuron(const Runtime::Vec2Int& range) { return shedule->get(range); }

    Neuron* NavMesh::get_neuron(const Vec2& worldPoint)
    {
        Vec2Int p;
        p.x = Math::ceil(shedule->widthSpace / 2 + worldPoint.x / worldScale.x);
        p.y = Math::ceil(shedule->heightSpace / 2 - worldPoint.y / worldScale.y);
        return get_neuron(p);
    }

    Neuron* NavMesh::get_neuron(const Runtime::Vec2& worldPoint, Runtime::Vec2Int& outPoint)
    {
        outPoint.x = Math::ceil(shedule->widthSpace / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        outPoint.y = Math::ceil(shedule->heightSpace / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return get_neuron(outPoint);
    }

    const Vec2Int NavMesh::world_position_to_point(const Vec2& worldPoint)
    {
        Vec2Int p;
        p.x = Math::ceil(shedule->widthSpace / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        p.y = Math::ceil(shedule->heightSpace / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return p;
    }

    std::uint32_t NavMesh::get_cache_size() { return shedule->get_cached_size(); }

    void NavMesh::find(NavResult& navResult, NavMethodRule method, Runtime::Vec2 worldPointFirst, Runtime::Vec2 worldPointLast)
    {
        NavContainer::NavigateionResult _nr;

        shedule->find(_nr, across::NavMethodRule(method), this->world_position_to_point(worldPointFirst), world_position_to_point(worldPointLast));
    }

    const Runtime::Vec2 NavMesh::point_to_world_position(const Runtime::Vec2Int& range) { return point_to_world_position(range.x, range.y); }

    const Runtime::Vec2 NavMesh::point_to_world_position(Neuron* neuron) { return point_to_world_position(neuronGetPoint(neuron)); }

    const Runtime::Vec2 NavMesh::point_to_world_position(const int& x, const int& y)
    {
        Vec2 vec2(shedule->widthSpace / 2.f, shedule->heightSpace / 2.f);
        vec2.x = (x - vec2.x) * worldScale.x;
        vec2.y = -(y - vec2.y) * worldScale.y;
        return vec2 + worldOffset;
    }

    bool NavMesh::get_nlocked(const Runtime::Vec2Int& range) { return shedule->get_nlocked(range); }

    bool NavMesh::neuronContains(const Runtime::Vec2Int& range) { return shedule->contains(range); }

    std::uint8_t& NavMesh::get_nflag(const Runtime::Vec2Int& range)
    {
        Neuron* n = get_neuron(range);
        if (!n)
            throw std::out_of_range("range");
        return n->flags;
    }

    std::uint32_t& RoninEngine::AIPathFinder::NavMesh::get_ncost(const Runtime::Vec2Int& range)
    {
        Neuron* n = get_neuron(range);
        if (!n)
            throw std::out_of_range("range");
        return n->cost;
    }

    std::uint32_t& RoninEngine::AIPathFinder::NavMesh::get_nheuristic(const Runtime::Vec2Int& range)
    {
        Neuron* n = get_neuron(range);
        if (!n)
            throw std::out_of_range("range");
        return n->h;
    }

    const int RoninEngine::AIPathFinder::NavMesh::get_nweight(const Runtime::Vec2Int& range)
    {
        if (!neuronContains(range))
            throw std::out_of_range("range");
        return range.x * range.y + range.y * range.y;
    }

    const std::uint32_t RoninEngine::AIPathFinder::NavMesh::get_ntotal(const Runtime::Vec2Int& range)
    {
        Neuron* n = get_neuron(range);
        if (!n)
            throw std::out_of_range("range");
        return n->cost + n->h;
    }

    const bool NavMesh::get_nempty(const Runtime::Vec2Int& range) { return !get_ntotal(range); }

    bool NavMesh::get_nlocked(const Neuron* neuron) { return get_nlocked(neuronGetPoint(neuron)); }

    std::uint8_t& NavMesh::get_nflag(const Neuron* neuron) { return get_nflag(neuronGetPoint(neuron)); }

    std::uint32_t& NavMesh::get_ncost(const Neuron* neuron) { return get_ncost(neuronGetPoint(neuron)); }

    std::uint32_t& NavMesh::get_nheuristic(const Neuron* neuron) { return get_nheuristic(neuronGetPoint(neuron)); }

    const int NavMesh::get_nweight(const Neuron* neuron) { return get_nweight(neuronGetPoint(neuron)); }

    const std::uint32_t NavMesh::get_ntotal(const Neuron* neuron) { return get_nempty(neuronGetPoint(neuron)); }

    const bool NavMesh::get_nempty(const Neuron* neuron) { return get_nempty(neuronGetPoint(neuron)); }

    void NavMesh::get_nlock(const Neuron* neuron, const bool state) { get_nlock(neuronGetPoint(neuron), state); }

    const Vec2Int NavMesh::neuronGetPoint(const Neuron* neuron) { return shedule->get_npoint(neuron); }

    void NavMesh::get_nlock(const Runtime::Vec2Int& range, const bool state) { shedule->set_nlock(range, state); }

    void NavMesh::load(const NavMeshData& navmeshData) { shedule->load(*reinterpret_cast<const across::AcrossData*>(&navmeshData)); }

    void NavMesh::save(NavMeshData* navmeshData) { shedule->save(reinterpret_cast<across::AcrossData*>(navmeshData)); }

    void NavMesh::find(NavResult& navResult, NavMethodRule method, Neuron* firstNeuron, Neuron* lastNeuron) { find(navResult, method, neuronGetPoint(firstNeuron), neuronGetPoint(lastNeuron)); }

    void NavMesh::find(NavResult& navResult, NavMethodRule method, Vec2Int first, Vec2Int last)
    {
        NavContainer::NavigateionResult _rt;
        shedule->find(_rt, static_cast<across::NavMethodRule>(method), first, last);
        navResult.firstNeuron = _rt.firstNeuron;
        navResult.lastNeuron = _rt.lastNeuron;
        navResult.map = this;
        navResult.RelativePaths = std::move(_rt.RelativePaths);
        navResult.status = static_cast<NavStatus>(_rt.status);
    }

} // namespace RoninEngine::AIPathFinder
