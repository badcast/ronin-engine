#include "ronin.h"

#include "across.hpp"

using namespace RoninEngine::Exception;

namespace RoninEngine::AI
{
    using namespace RoninEngine::Runtime;

    class NavContainer : public across::basic_across_map<Vec2Int, Neuron, std::deque<Vec2Int>>
    {
        friend class NavMesh;

    public:
        NavContainer(int width, int height) : basic_across_map(width, height)
        {
        }
    };

    NavMesh::NavMesh(int lwidth, int lheight)
    {
        worldScale = Vec2::one;
        RoninMemory::alloc_self(shedule, lwidth, lheight);
    }

    NavMesh::~NavMesh()
    {
        RoninMemory::free(shedule);
    }

    void NavMesh::randomize(int flagFilter)
    {
        shedule->randomize_hardware(flagFilter);
    }

    void NavMesh::stress()
    {
    }

    int NavMesh::width() const
    {
        return shedule->widthSpace;
    }

    int NavMesh::height() const
    {
        return shedule->heightSpace;
    }

    void NavMesh::clear(bool clearLocks)
    {
        shedule->clear(clearLocks);
    }

    void NavMesh::fill(bool fillLocks)
    {
        shedule->fill(fillLocks);
    }

    Neuron *NavMesh::get_neuron(int x, int y)
    {
        return shedule->get(x, y);
    }

    Neuron *NavMesh::get_neuron(const Vec2Int &range)
    {
        return shedule->get(range);
    }

    Neuron *NavMesh::get_neuron(const Vec2 &worldPoint)
    {
        Vec2Int p {
            Math::ceil(shedule->widthSpace / 2 + worldPoint.x / worldScale.x),
            Math::ceil(shedule->heightSpace / 2 - worldPoint.y / worldScale.y)};
        return get_neuron(p);
    }

    Neuron *NavMesh::get_neuron(const Vec2 &worldPoint, Vec2Int &outPoint)
    {
        outPoint.x = Math::ceil(shedule->widthSpace / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        outPoint.y = Math::ceil(shedule->heightSpace / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return get_neuron(outPoint);
    }

    const Vec2Int NavMesh::world_position_to_point(const Vec2 &worldPoint)
    {
        Vec2Int p;
        p.x = Math::ceil(shedule->widthSpace / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        p.y = Math::ceil(shedule->heightSpace / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return p;
    }

    std::uint32_t NavMesh::get_cache_size()
    {
        return shedule->get_cached_size();
    }

    bool NavMesh::find(NavResult &navResult, Neuron *firstNeuron, Neuron *lastNeuron)
    {
        return shedule->find(*reinterpret_cast<NavContainer::NavigationResult *>(&navResult), firstNeuron, lastNeuron);
    }

    bool NavMesh::find(NavResult &navResult, Vec2Int first, Vec2Int last)
    {
        return find(navResult, get_neuron(first), get_neuron(last));
    }

    bool NavMesh::find(NavResult &navResult, Vec2 worldPointFirst, Vec2 worldPointLast)
    {
        return find(navResult, this->world_position_to_point(worldPointFirst), world_position_to_point(worldPointLast));
    }

    const Vec2 NavMesh::point_to_world_position(const Vec2Int &range)
    {
        return point_to_world_position(range.x, range.y);
    }

    const Vec2 NavMesh::point_to_world_position(Neuron *neuron)
    {
        return point_to_world_position(get_npoint(neuron));
    }

    const Vec2 NavMesh::point_to_world_position(const int &x, const int &y)
    {
        Vec2 vec2(shedule->widthSpace / 2.f, shedule->heightSpace / 2.f);
        vec2.x = (x - vec2.x) * worldScale.x;
        vec2.y = -(y - vec2.y) * worldScale.y;
        return vec2 + worldOffset;
    }

    bool NavMesh::get_nlocked(const Vec2Int &range)
    {
        return shedule->get_nlocked(range);
    }

    bool NavMesh::get_ncontains(const Vec2Int &range) const
    {
        return shedule->contains(range);
    }

    std::uint8_t &NavMesh::get_nflag(const Vec2Int &range)
    {
        Neuron *n = get_neuron(range);
        if(!n)
            throw ronin_invlaid_range_error();
        return n->flags;
    }

    std::uint32_t &RoninEngine::AI::NavMesh::get_ncost(const Vec2Int &range)
    {
        Neuron *n = get_neuron(range);
        if(!n)
            throw ronin_invlaid_range_error();
        return n->cost;
    }

    std::uint32_t &RoninEngine::AI::NavMesh::get_nheuristic(const Vec2Int &range)
    {
        Neuron *n = get_neuron(range);
        if(!n)
            throw ronin_invlaid_range_error();
        return n->h;
    }

    const int RoninEngine::AI::NavMesh::get_nweight(const Vec2Int &range)
    {
        if(!get_ncontains(range))
            throw ronin_invlaid_range_error();
        return range.x * range.y + range.y * range.y;
    }

    const std::uint32_t RoninEngine::AI::NavMesh::get_ntotal(const Vec2Int &range)
    {
        Neuron *n = get_neuron(range);
        if(!n)
            throw ronin_invlaid_range_error();
        return n->cost + n->h;
    }

    const bool NavMesh::get_nempty(const Vec2Int &range)
    {
        return !get_ntotal(range);
    }

    bool NavMesh::get_nlocked(const Neuron *neuron)
    {
        return get_nlocked(get_npoint(neuron));
    }

    std::uint8_t &NavMesh::get_nflag(const Neuron *neuron)
    {
        return get_nflag(get_npoint(neuron));
    }

    std::uint32_t &NavMesh::get_ncost(const Neuron *neuron)
    {
        return get_ncost(get_npoint(neuron));
    }

    std::uint32_t &NavMesh::get_nheuristic(const Neuron *neuron)
    {
        return get_nheuristic(get_npoint(neuron));
    }

    const int NavMesh::get_nweight(const Neuron *neuron)
    {
        return get_nweight(get_npoint(neuron));
    }

    const std::uint32_t NavMesh::get_ntotal(const Neuron *neuron)
    {
        return get_nempty(get_npoint(neuron));
    }

    const bool NavMesh::get_nempty(const Neuron *neuron)
    {
        return get_nempty(get_npoint(neuron));
    }

    void NavMesh::set_nlock(const Neuron *neuron, const bool state)
    {
        set_nlock(get_npoint(neuron), state);
    }

    const Vec2Int NavMesh::get_npoint(const Neuron *neuron) const
    {
        return shedule->get_npoint(neuron);
    }

    HeuristicMethod NavMesh::get_heuristic_method()
    {
        return HeuristicMethod(shedule->get_heuristic());
    }

    bool NavMesh::set_heuristic_method(HeuristicMethod method)
    {
        return shedule->set_heuristic(across::HeuristicMethod(method));
    }

    bool NavMesh::set_identity(NavIdentity newIdentity)
    {
        return shedule->set_identity(across::MatrixIdentity(newIdentity));
    }

    void NavMesh::set_nlock(const Vec2Int &range, const bool state)
    {
        shedule->set_nlock(range, state);
    }

    void NavMesh::load(const NavMeshData &navmeshData)
    {
        shedule->load(*reinterpret_cast<const across::AcrossData *>(&navmeshData));
    }

    void NavMesh::save(NavMeshData *navmeshData)
    {
        shedule->save(reinterpret_cast<across::AcrossData *>(navmeshData));
    }
} // namespace RoninEngine::AI
