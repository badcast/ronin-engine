#include "ronin.h"
#include "ronin_matrix.h"
#include "ronin_debug.h"

using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

namespace RoninEngine
{
    namespace Runtime
    {
        extern std::set<World *> loadedWorlds;
        extern std::set<World *> privateWorlds;

        void internal_load_world(World *world)
        {
            if(world == nullptr)
            {
                throw ronin_null_error();
            }
            if(world->irs == nullptr)
            {
                // reallocate channels
                Mix_AllocateChannels(MIX_CHANNELS);

                RoninMemory::alloc_self(world->irs);
                RoninMemory::alloc_self(world->irs->gui, world);

                       // set state load
                world->irs->requestUnloading = false;
                // update internal loaded font
                font2d_update(world);

                loadedWorlds.insert(world);
            }
        }

        bool internal_unload_world(World *world)
        {
            if(world->irs == nullptr)
                return false;
            World *prevWorld = currentWorld;
            currentWorld = world;
            world->RequestUnload();
            world->OnUnloading();
            /////////////////////
            /// Free Game Objects
            /////////////////////
            TransformRef target = world->irs->mainObject->transform();
            std::list<TransformRef> stacks;
            while(target)
            {
                stacks.merge(target->hierarchy);
                sepuku_GameObject(target->_owner, nullptr);
                if(stacks.empty())
                {
                    target = nullptr;
                }
                else
                {
                    target = stacks.front();
                    stacks.pop_front();
                }
            }
            world->irs->mainObject = nullptr;
            if(world->irs->objects != 0)
            {
                ronin_log(("World is have leak objects: " + std::to_string(world->irs->objects)).c_str());
            }
            if(world->irs->runtimeCollectors)
            {
                RoninMemory::free(world->irs->runtimeCollectors);
                world->irs->runtimeCollectors = nullptr;
            }
            /////////////////////
            /// free GUI objects
            /////////////////////
            if(world->irs->gui)
            {
                RoninMemory::free(world->irs->gui);
                world->irs->gui = nullptr;
            }
            /////////////////////
            /// NOTE: Free Local Resources
            /// world->irs->external_local_resources = nullptr;
            /////////////////////
            gid_resources_free(&world->irs->externalLocalResources);

                   // Halt all channels
            Mix_HaltChannel(-1);
            // reallocate channels
            Mix_AllocateChannels(MIX_CHANNELS);
            // free native resources
            for(SDL_Surface *surface : world->irs->preloadeSurfaces)
            {
                SDL_FreeSurface(surface);
            }
            for(CameraResource *cam_res : world->irs->cameraResources)
            {
                RoninMemory::free(cam_res);
            }
            RoninMemory::free(world->irs);
            world->irs = nullptr;
            loadedWorlds.erase(world);
            currentWorld = prevWorld;
            return true;
        }

        void level_render_world()
        {
            Time::BeginWatch();
            // set default color
            RenderUtility::SetColor(Color::white);
            // Set scale to default
            SDL_RenderSetScale(gscope.renderer, 1.f, 1.f);
            scripts_start();
            scripts_update();
            gscope.queueWatcher.delayExecScripts = Time::EndWatch();

            Time::BeginWatch();
            // Render on main camera
            Camera* cam = Camera::mainCamera().ptr_; // Draw level from active camera (main)
            if(!currentWorld->irs->requestUnloading && cam && cam->enable())
            {
                // draw world in world size
                native_render_2D(reinterpret_cast<Camera2D *>(cam));
                       // Set scale to default
                SDL_RenderSetScale(gscope.renderer, 1.f, 1.f);
            }
            gscope.queueWatcher.delayPresent = Time::EndWatch();

            Time::BeginWatch();
            scripts_lateUpdate();
            gscope.queueWatcher.delayExecScripts+=Time::EndWatch();

            Time::BeginWatch();
            if(!currentWorld->irs->requestUnloading && cam)
            {
                // Reset Color
                RenderUtility::SetColor(Color::white);

                       // Draw gizmos
                currentWorld->OnGizmos();
                scripts_gizmos();
            }
            gscope.queueWatcher.delayRenderGizmos = Time::EndWatch();
        }

    } // namespace Runtime

    World::World(const std::string &name) : m_name(name), irs(nullptr)
    {
    }

    World::~World()
    {
        // unloading
        if(this->irs)
            internal_unload_world(this);
    }

    void WorldResources::event_camera_changed(CameraRef target, CameraEvent state)
    {
        switch(state)
        {
            case CameraEvent::CAM_DELETED:
                // TODO: find free Camera and set as Main
                if(mainCamera == target)
                    mainCamera = nullptr;
                // BUG: MEMORY LEAK CAM RESOURCES
                cameraResources.remove(target->res);

                RoninMemory::free(target->res);
                break;
            case CameraEvent::CAM_TARGET:
                mainCamera = target;
                break;
        }
    }

           // NOTE: Check game hierarchy
    std::list<Transform *> World::MatrixCheckDamage()
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();
        std::list<Transform *> damaged;
        for(auto x = std::begin(currentWorld->irs->matrix); x != end(currentWorld->irs->matrix); ++x)
        {
            // unordered_map<Vec2Int,... <Transform*>>
            for(auto &layerObject : x->second)
            {
                // set<Transform*>
                for(auto &y : layerObject.second)
                {
                    if(y->isNull() || Matrix::matrix_get_key(y->_position) != layerObject.first || x->first != y->_owner->m_layer)
                    {
                        damaged.emplace_back(y);
                    }
                }
            }
        }
        return damaged;
    }

    int World::MatrixRestore()
    {
        auto damaged = MatrixCheckDamage();
        return MatrixRestore(damaged);
    }

           // try restore damaged matrix element's
    int World::MatrixRestore(const std::list<Runtime::Transform *> &damaged_content)
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();
        int restored = 0;
        for(Transform *dam : damaged_content)
        {
            Matrix::matrix_key_t key = Matrix::matrix_get_key(dam->_position);
            // unordered_map<int, ...>
            for(auto &findIter : currentWorld->irs->matrix)
            {
                // unordered_map<Vec2Int,...>
                for(auto &layer : findIter.second)
                    // set<Transform*>
                    for(auto &set : layer.second)
                    {
                        if(set != dam)
                            continue;
                        if(set->_owner->m_layer != findIter.first || key != layer.first)
                        {
                            // Remove damaged transform
                            layer.second.erase(dam);
                            if(!dam->isNull())
                            {
                                // Restore
                                currentWorld->irs->matrix[dam->_owner->m_layer][key].insert(dam);
                            }
                            ++restored;
                            goto next;
                        }
                    }
            }
        next:
            continue;
        }

        return restored;
    }

    int World::GetCulled()
    {
        if(irs == nullptr || irs->mainCamera == nullptr || irs->mainCamera->res == nullptr)
            return -1;
        return irs->mainCamera->res->culled;
    }

    int World::MatrixCacheCount()
    {
        if(irs == nullptr)
            return -1;
        int cached = 0;
        for(auto &matrix : this->irs->matrix)
            cached += static_cast<int>(std::count_if(
                matrix.second.begin(),
                matrix.second.end(),
                [](auto iobject)
                {
                    // predicate
                    return iobject.second.empty();
                }));
        return cached;
    }

    int World::MatrixCacheClear()
    {
        if(irs == nullptr)
            return -1;
        int cleans = 0;
        std::vector<typename std::unordered_map<Vec2Int, std::set<Transform *>>::iterator> cached;
        for(auto &matrix : this->irs->matrix)
        {
            for(auto iter = std::begin(matrix.second); iter != std::end(matrix.second); ++iter)
            {
                if(iter->second.empty())
                {
                    cached.emplace_back(iter);
                }
            }
            for(auto &iter_ref : cached)
            {
                matrix.second.erase(iter_ref);
            }
            cleans += static_cast<int>(cached.size());
            cached.clear();
        }

        return cleans;
    }

    std::uint64_t World::StatCountRefLinks(){
        std::uint64_t retval = 0;
        if(irs)
        {
            retval = irs->refPointers.size();
        }
        return retval;
    }

    std::uint64_t World::StatMaxRefDepth(){
        std::uint64_t retval = 0;
        if(irs)
        {
            for(auto & refkey : this->irs->refPointers)
            {
                if(refkey.second.ref_count_)
                    retval = Math::Max(retval, *refkey.second.ref_count_);
            }
        }
        return retval;
    }

    std::string World::GetTreeOfHierarchy() const
    {
        if(irs == nullptr)
            return {};
        static char delim = 0x32;
        std::string delims;
        std::string result;
        std::list<TransformRef> stack;
        TransformRef target = irs->mainObject->transform();
        while(target)
        {
            for(TransformRef& current : target->hierarchy)
            {
                stack.emplace_back(current);
            }
            result += delims;
            result += target->name();
            result += "\n";
            if(!stack.empty())
            {
                target = stack.front();
                stack.pop_front();
            }
            else
                target = nullptr;
        }
        return result;
    }

    bool World::isHierarchy()
    {
        if(irs != nullptr)
            return !irs->mainObject.isNull();
        return false;
    }

    std::string &World::name()
    {
        return m_name;
    }

    UI::GUI *World::GetGUI()
    {
        UI::GUI * retval;
        if(irs != nullptr)
            retval = irs->gui;
        else
            retval = nullptr;
        return retval;
    }

    void World::RequestUnload()
    {
        if(irs)
            irs->requestUnloading = true;
    }

    int World::GetDestroyedFrames()
    {
        if(irs == nullptr)
            return -1;
        return irs->_destroyedGameObject;
    }

    std::list<GameObjectRef> World::GetAllGameObjects()
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();
        std::list<GameObjectRef> all_gobjects;
        GameObjectRef next = irs->mainObject;
        while(next)
        {
            for(TransformRef& e : next->transform()->hierarchy)
            {
                all_gobjects.emplace_front(e->gameObject());
            }
            if(next == all_gobjects.front())
                next = nullptr;
            else
                next = all_gobjects.front();
        }
        return all_gobjects;
    }

    std::list<ComponentRef> World::GetAllComponents()
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();
        std::list<ComponentRef> components;
        std::list<GameObjectRef> all_objects = GetAllGameObjects();
        for(GameObjectRef& curObject : all_objects)
        {
            for(ComponentRef & self_component : curObject->m_components)
            {
                components.emplace_back(self_component);
            }
        }
        return components;
    }

    const bool World::CancelObjectDestruction(GameObjectRef obj)
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

        if(obj && irs->runtimeCollectors)
        {
            // std::pair<const float, std::set<GameObject *>>
            for(auto mapIter = std::begin(*irs->runtimeCollectors); mapIter != std::end(*irs->runtimeCollectors); ++mapIter)
            {
                std::set<GameObject*>::iterator iter = mapIter->second.find(obj.ptr_);
                if(iter != std::end(mapIter->second))
                {
                    mapIter->second.erase(iter);
                    if(mapIter->second.empty())
                        irs->runtimeCollectors->erase(mapIter); // erase empty list
                    return true;
                }
            }
        }
        return false;
    }

    const int World::CostObjectDestruction(GameObjectRef obj)
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();
        if(obj && irs->runtimeCollectors)
        {
            int x = 0;
            for(std::pair<const float, std::set<GameObject*>> &mapIter : *irs->runtimeCollectors)
            {
                if(mapIter.second.find(obj.ptr_) != std::end(mapIter.second))
                {
                    return x;
                }
                ++x;
            }
        }

        return -1;
    }

    const bool World::StateObjectDestruction(GameObjectRef obj)
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();
        return this->CostObjectDestruction(obj) > ~0;
    }

    const int World::CountObjectDestruction()
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();
        int x = 0;
        if(irs->runtimeCollectors)
        {
            for(std::pair<const float, std::set<GameObject*>> &mapIter : *irs->runtimeCollectors)
            {
                x += mapIter.second.size();
            }
        }
        return x;
    }

    void World::OnAwake()
    {
    }

    void World::OnStart()
    {
    }

    void World::OnUpdate()
    {
    }

    void World::OnGizmos()
    {
    }

    void World::OnUnloading()
    {
    }

    World *World::GetCurrentWorld()
    {
        return currentWorld;
    }
} // namespace RoninEngine
