#include "ronin.h"

#ifdef OVERRIDE_LIBBRAINMAP
#include <brain_map.hpp>
#else
#include "brain_map.hpp"
#endif

using namespace RoninEngine::Exception;

namespace RoninEngine::AI
{
    using namespace RoninEngine::Runtime;

    class Sheduller : public brain::basic_brain_map<Vec2Int, Neuron>
    {
        friend class NavMesh;

    public:
        Sheduller(int width, int height) : basic_brain_map(width, height)
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

    void NavMesh::create_maze()
    {
        shedule->create_maze();
    }

    int NavMesh::width() const
    {
        return shedule->_xsize;
    }

    int NavMesh::height() const
    {
        return shedule->_ysize;
    }

    void NavMesh::clear(bool clearLocks)
    {
        shedule->clear(clearLocks);
    }

    void NavMesh::fill(bool fillLocks)
    {
        shedule->fill(fillLocks);
    }

    Neuron *NavMesh::get(int x, int y)
    {
        return shedule->get(x, y);
    }

    Neuron *NavMesh::get(const Vec2Int &range)
    {
        return shedule->get(range);
    }

    Neuron *NavMesh::get(const Vec2 &worldPoint)
    {
        Vec2Int p {
            Math::round(shedule->_xsize / 2 + worldPoint.x / worldScale.x), Math::round(shedule->_ysize / 2 - worldPoint.y / worldScale.y)};

        return get(p);
    }

    Neuron *NavMesh::get(const Vec2 &worldPoint, Vec2Int &outPoint)
    {
        outPoint.x = Math::round(shedule->_xsize / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        outPoint.y = Math::round(shedule->_ysize / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return get(outPoint);
    }

    const Vec2Int NavMesh::world_position_to_point(const Vec2 &worldPoint)
    {
        Vec2Int p;
        p.x = Math::round(shedule->_xsize / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        p.y = Math::round(shedule->_ysize / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return p;
    }

    std::uint32_t NavMesh::get_cache_size()
    {
        return shedule->get_cached_size();
    }

    bool NavMesh::find(NavResultSite &navResult, Neuron *firstNeuron, Neuron *lastNeuron)
    {
        return shedule->find(*reinterpret_cast<Sheduller::result_site *>(&navResult), firstNeuron, lastNeuron);
    }

    bool NavMesh::find(NavResultSite &navResult, const Runtime::Vec2Int &first, const Runtime::Vec2Int &last)
    {
        return find(navResult, get(first), get(last));
    }

    bool NavMesh::find(NavResultSite &navResult, const Runtime::Vec2 &worldPointFirst, const Runtime::Vec2 &worldPointLast)
    {
        return find(navResult, this->world_position_to_point(worldPointFirst), world_position_to_point(worldPointLast));
    }

    bool NavMesh::find(NavResultNeuron &navResult, Neuron *firstNeuron, Neuron *lastNeuron)
    {
        return shedule->find(*reinterpret_cast<Sheduller::result_neuron *>(&navResult), firstNeuron, lastNeuron);
    }

    bool NavMesh::find(NavResultNeuron &navResult, const Runtime::Vec2Int &first, const Runtime::Vec2Int &last)
    {
        return find(navResult, get(first), get(last));
    }

    bool NavMesh::find(NavResultNeuron &navResult, const Runtime::Vec2 &worldPointFirst, const Runtime::Vec2 &worldPointLast)
    {
        return find(navResult, this->world_position_to_point(worldPointFirst), world_position_to_point(worldPointLast));
    }

    const Vec2 NavMesh::point_to_world_position(const Vec2Int &range)
    {
        return point_to_world_position(range.x, range.y);
    }

    const Vec2 NavMesh::point_to_world_position(int x, int y)
    {
        Vec2 vec2(shedule->_xsize / 2.f, shedule->_ysize / 2.f);
        vec2.x = (x - vec2.x) * worldScale.x;
        vec2.y = -(y - vec2.y) * worldScale.y;
        return vec2 + worldOffset;
    }

    const Vec2 NavMesh::neuron_to_world_position(Neuron *neuron)
    {
        return point_to_world_position(get_point(neuron));
    }

    bool NavMesh::get_ncontains(const Vec2Int &range) const
    {
        return shedule->contains(range);
    }

    bool NavMesh::has_locked(const Vec2Int &range)
    {
        return shedule->has_lock(range);
    }

    bool NavMesh::has_locked(const Neuron *neuron)
    {
        return shedule->has_lock(neuron);
    }

    void NavMesh::set_lock(const Neuron *neuron, const bool state)
    {
        set_lock(get_point(neuron), state);
    }

    const Vec2Int NavMesh::get_point(const Neuron *neuron) const
    {
        return shedule->get_point(neuron);
    }

    HeuristicMethod NavMesh::get_heuristic_method()
    {
        return HeuristicMethod(shedule->get_heuristic());
    }

    bool NavMesh::set_heuristic_method(HeuristicMethod method)
    {
        return shedule->set_heuristic(brain::HeuristicMethod(method));
    }

    bool NavMesh::set_identity(NavIdentity newIdentity)
    {
        return shedule->set_identity(brain::MatrixIdentity(newIdentity));
    }

    NavIdentity NavMesh::get_identity()
    {
        return NavIdentity(shedule->get_identity());
    }

    NavListSite NavMesh::get_neighbours(NavIdentity identity, const Vec2Int &pick)
    {
        return shedule->get_neighbours<NavListSite>(brain::MatrixIdentity(identity), pick);
    }

    NavListNeuron NavMesh::get_neighbours(NavIdentity identity, const Neuron *pick)
    {
        return shedule->get_neighbours<NavListNeuron>(brain::MatrixIdentity(identity), pick);
    }

    void NavMesh::set_lock(const Vec2Int &range, const bool state)
    {
        shedule->set_lock(range, state);
    }

    void NavMesh::load(const NavMeshData &navmeshData)
    {
        shedule->load(*reinterpret_cast<const brain::brain_breakfast *>(&navmeshData));
    }

    void NavMesh::save(NavMeshData *navmeshData)
    {
        shedule->save(reinterpret_cast<brain::brain_breakfast *>(navmeshData));
    }
} // namespace RoninEngine::AI
