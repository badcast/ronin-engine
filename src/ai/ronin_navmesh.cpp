#include "ronin.h"

#ifdef OVERRIDE_LIBBRAINMAP
#include <brain_map.hpp>
#else
#include "../3rdparty/brainmap/brain_map.hpp"
#endif

using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

extern std::function<std::uint32_t(void)> internalRandom;

namespace RoninEngine::AI
{
    class Sheduller : public brain::basic_brain_map<Vec2Int, Neuron>
    {
        friend class NavMesh;

    public:
        Sheduller(int width, int height) : basic_brain_map(width, height)
        {
            this->set_random_function(internalRandom);
        }
    };

    NavMesh::NavMesh(std::uint32_t lwidth, std::uint32_t lheight)
    {
        worldScale = Vec2::one;
        RoninMemory::alloc_self(shedule, lwidth, lheight);
    }

    NavMesh::~NavMesh()
    {
        RoninMemory::free(shedule);
    }

    void NavMesh::Shuffle(int flagFilter)
    {
        shedule->randomize_hardware(flagFilter);
    }

    void NavMesh::GenerateMaze(MazeAlgorithm algorithm)
    {
        shedule->create_maze_ex(static_cast<brain::MazeAlgorithm>(algorithm), getPoint(shedule->front()), getPoint(shedule->back()));
    }

    std::uint32_t NavMesh::getWidth() const
    {
        return shedule->_xsize;
    }

    std::uint32_t NavMesh::getHeight() const
    {
        return shedule->_ysize;
    }

    void NavMesh::Clear(bool clearLocks)
    {
        shedule->clear(clearLocks);
    }

    void NavMesh::fillAll(bool fillLocks)
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
        Vec2Int p {Math::Number(static_cast<float>(shedule->_xsize) / 2 + worldPoint.x / worldScale.x), Math::Number(static_cast<float>(shedule->_ysize) / 2 - worldPoint.y / worldScale.y)};

        return get(p);
    }

    Neuron *NavMesh::get(const Vec2 &worldPoint, Vec2Int &outPoint)
    {
        outPoint.x = Math::Round(shedule->_xsize / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        outPoint.y = Math::Round(shedule->_ysize / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return get(outPoint);
    }

    const Vec2Int NavMesh::WorldPointToPoint(const Vec2 &worldPoint)
    {
        Vec2Int p;
        p.x = Math::Round(shedule->_xsize / 2 + (worldPoint.x + worldOffset.x) / worldScale.x);
        p.y = Math::Round(shedule->_ysize / 2 - (worldPoint.y + worldOffset.y) / worldScale.y);
        return p;
    }

    std::uint32_t NavMesh::getCached()
    {
        return shedule->get_cached_size();
    }

    bool NavMesh::Find(NavResultSite &navResult, Neuron *firstNeuron, Neuron *lastNeuron)
    {
        return shedule->find(*reinterpret_cast<Sheduller::result_site *>(&navResult), firstNeuron, lastNeuron);
    }

    bool NavMesh::Find(NavResultSite &navResult, const Runtime::Vec2Int &first, const Runtime::Vec2Int &last)
    {
        return Find(navResult, get(first), get(last));
    }

    bool NavMesh::Find(NavResultSite &navResult, const Runtime::Vec2 &worldPointFirst, const Runtime::Vec2 &worldPointLast)
    {
        return Find(navResult, this->WorldPointToPoint(worldPointFirst), WorldPointToPoint(worldPointLast));
    }

    bool NavMesh::Find(NavResultNeuron &navResult, Neuron *firstNeuron, Neuron *lastNeuron)
    {
        return shedule->find(*reinterpret_cast<Sheduller::result_neuron *>(&navResult), firstNeuron, lastNeuron);
    }

    bool NavMesh::Find(NavResultNeuron &navResult, const Runtime::Vec2Int &first, const Runtime::Vec2Int &last)
    {
        return Find(navResult, get(first), get(last));
    }

    bool NavMesh::Find(NavResultNeuron &navResult, const Runtime::Vec2 &worldPointFirst, const Runtime::Vec2 &worldPointLast)
    {
        return Find(navResult, this->WorldPointToPoint(worldPointFirst), WorldPointToPoint(worldPointLast));
    }

    const Vec2 NavMesh::PointToWorldPoint(const Vec2Int &range)
    {
        Vec2 vec2(shedule->_xsize / 2.f, shedule->_ysize / 2.f);
        vec2.x = (range.x - vec2.x) * worldScale.x;
        vec2.y = -(range.y - vec2.y) * worldScale.y;
        return vec2 + worldOffset;
    }

    const Vec2 NavMesh::PointToWorldPoint(int x, int y)
    {
        Vec2 vec2(shedule->_xsize / 2.f, shedule->_ysize / 2.f);
        vec2.x = (x - vec2.x) * worldScale.x;
        vec2.y = -(y - vec2.y) * worldScale.y;
        return vec2 + worldOffset;
    }

    const Vec2 NavMesh::NeuronToWorldPoint(Neuron *neuron)
    {
        return PointToWorldPoint(getPoint(neuron));
    }

    bool NavMesh::Contains(const Vec2Int &range) const
    {
        return shedule->contains(range);
    }

    bool NavMesh::hasLocked(const Vec2Int &range)
    {
        return shedule->has_lock(range);
    }

    bool NavMesh::hasLocked(const Neuron *neuron)
    {
        return shedule->has_lock(neuron);
    }

    void NavMesh::setLock(const Neuron *neuron, const bool state)
    {
        shedule->set_lock(neuron, state);
    }

    const Vec2Int NavMesh::getPoint(const Neuron *neuron) const
    {
        return shedule->get_point(neuron);
    }

    HeuristicMethod NavMesh::getHeuristic()
    {
        return HeuristicMethod(shedule->get_heuristic());
    }

    bool NavMesh::setHeuristic(HeuristicMethod method)
    {
        return shedule->set_heuristic(brain::HeuristicMethod(method));
    }

    bool NavMesh::setIdentity(NavIdentity newIdentity)
    {
        return shedule->set_identity(brain::MatrixIdentity(newIdentity));
    }

    NavIdentity NavMesh::getIdentity()
    {
        return NavIdentity(shedule->get_identity());
    }

    NavListSite NavMesh::GetNeighbours(NavIdentity identity, const Runtime::Vec2Int &pick)
    {
        return shedule->get_neighbours<NavListSite>(brain::MatrixIdentity(identity), pick);
    }

    NavListNeuron NavMesh::GetNeighbours(NavIdentity identity, const Neuron *pick)
    {
        return shedule->get_neighbours<NavListNeuron>(brain::MatrixIdentity(identity), pick);
    }

    void NavMesh::setLock(const Vec2Int &range, const bool state)
    {
        shedule->set_lock(range, state);
    }

    void NavMesh::LoadFrom(const NavMeshData &navmeshData)
    {
        shedule->load(*reinterpret_cast<const brain::brain_breakfast *>(&navmeshData));
    }

    void NavMesh::SaveTo(NavMeshData *navmeshData)
    {
        shedule->save(reinterpret_cast<brain::brain_breakfast *>(navmeshData));
    }
} // namespace RoninEngine::AI
