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
        extern World *preloadWorld;
        extern World *lastWorld;

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
                world->irs->requestUnloading = false;
                world->irs->objects = 0;
                RoninMemory::alloc_self(world->irs->gui, world);

                world->irs->metricPixelsPerPoint = Vec2::one * defaultPixelsPerPoint;
                // update internal loaded font
                font2d_update(world);

                loadedWorlds.insert(world);

                if(world->irs->mainObject.isNull())
                {
                    World *prev = currentWorld;
                    currentWorld = world;
                    world->irs->mainObject = create_empty_gameobject();
                    world->irs->mainObject->name("Main Object");
                    world->irs->mainObject->transform()->name("Root");
                    Matrix::matrix_remove(world->irs->mainObject->transform().ptr_);
                    currentWorld = (prev != nullptr) ? prev : world;
                }
            }
            else
            {
                font2d_update(world);
            }
        }

        bool internal_unload_world(World *world)
        {
            if(world == nullptr || world->irs == nullptr)
                return false;
            World *prevWorld = currentWorld;
            currentWorld = world;
            world->RequestUnload();
            world->OnUnloading();
            /////////////////////
            /// Free Game Objects
            /////////////////////
            if(!world->irs->mainObject.isNull())
            {
                sepuku_GameObject(world->irs->mainObject, nullptr);
                world->irs->mainObject = nullptr;
            }
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
            /////////////////////
            gid_resources_free(&world->irs->externalLocalResources);

            // Halt all channels
            Mix_HaltChannel(-1);
            // reallocate channels
            Mix_AllocateChannels(MIX_CHANNELS);
            // free native resources
            for(SDL_Surface *surface : world->irs->preloadeSurfaces)
            {
                if(surface)
                    SDL_FreeSurface(surface);
            }
            world->irs->preloadeSurfaces.clear();
            for(CameraResource *cam_res : world->irs->cameraResources)
            {
                RoninMemory::free(cam_res);
            }
            world->irs->cameraResources.clear();
            for(auto &pair : world->irs->renderCache)
            {
                if(pair.second.second)
                    SDL_DestroyTexture(pair.second.second);
            }
            world->irs->renderCache.clear();
            world->irs->renderCacheRefs.clear();
            if(world->irs->legacy_font_normal)
            {
                SDL_DestroyTexture(world->irs->legacy_font_normal);
                world->irs->legacy_font_normal = nullptr;
            }
            if(world->irs->legacy_font_hover)
            {
                SDL_DestroyTexture(world->irs->legacy_font_hover);
                world->irs->legacy_font_hover = nullptr;
            }
            for(auto &pair : world->irs->matrix)
            {
                delete pair.second;
            }
            world->irs->matrix.clear();
            RoninMemory::free(world->irs);
            world->irs = nullptr;
            loadedWorlds.erase(world);
            if(preloadWorld == world)
                preloadWorld = nullptr;
            if(lastWorld == world)
                lastWorld = nullptr;
            if(currentWorld == world)
                currentWorld = (prevWorld != world) ? prevWorld : nullptr;
            else
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
                Camera2D *cam2d = dynamic_cast<Camera2D *>(cam);
                if(cam2d)
                {
                    native_render_2D(cam2d);
                }
                else
                {
                    native_render_3D(cam);
                }
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
        internal_load_world(this);
        if(currentWorld == nullptr)
            currentWorld = this;
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
                if(mainCamera == target)
                {
                    mainCamera = nullptr;
                }
                cameraResources.remove(target->res);
                RoninMemory::free(target->res);
                target->res = nullptr;
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
        for(auto& pair : irs->matrix)
        {
            if(!pair.second)
                continue;
            std::set<Transform*> objects;
            pair.second->GetAllObjects(objects);
            for(Transform* obj : objects)
            {
                if(obj == nullptr || obj->isNull() || !obj->_owner || obj->_owner->m_layer != pair.first)
                {
                    damaged.emplace_back(obj);
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
            for (auto& pair : currentWorld->irs->matrix)
            {
                if (pair.second->Remove(dam))
                    break;
            }
            if (!dam->isNull())
            {
                Matrix::matrix_update(dam);
                ++restored;
            }
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
            return 0;
        int count = 0;
        for(auto &pair : irs->matrix)
        {
            if(pair.second)
                count += static_cast<int>(pair.second->CountAllObjects());
        }
        return count;
    }

    int World::MatrixCacheClear()
    {
        if(irs == nullptr)
            return 0;
        int cleared = 0;
        for(auto &pair : irs->matrix)
        {
            if(pair.second)
            {
                cleared += static_cast<int>(pair.second->CountAllObjects());
                pair.second->Clear();
            }
        }
        return cleared;
    }

    std::uint64_t World::StatCountRefLinks()
    {
        if(irs == nullptr || irs->mainObject.isNull())
            return 0;
        std::uint64_t count = 0;
        std::list<TransformRef> queue;
        queue.push_back(irs->mainObject->transform());
        while(!queue.empty())
        {
            TransformRef cur = queue.front();
            queue.pop_front();
            if(!cur) continue;
            count += cur->hierarchy.size();
            for(TransformRef &child : cur->hierarchy)
                queue.push_back(child);
        }
        return count;
    }

    std::uint64_t World::StatMaxRefDepth()
    {
        if(irs == nullptr || irs->mainObject.isNull())
            return 0;
        std::function<std::uint64_t(TransformRef)> getDepth = [&](TransformRef cur) -> std::uint64_t
        {
            if(!cur) return 0;
            std::uint64_t maxChild = 0;
            for(TransformRef &child : cur->hierarchy)
                maxChild = std::max(maxChild, getDepth(child));
            return 1 + maxChild;
        };
        return getDepth(irs->mainObject->transform());
    }

    void World::ObjectMoveTo(GameObject *target, World *newWorld)
    {
        if(!target || !newWorld || newWorld == this || !irs || !newWorld->irs || target->transform().isNull())
            return;

        TransformRef tr = target->transform();

        // 1. Remove from current matrix
        std::vector<TransformRef> allTransforms = tr->GetAllTransforms();
        for(TransformRef &t : allTransforms)
        {
            if(t && t->_owner && t->_owner->m_active)
            {
                Matrix::matrix_remove(t.ptr_);
            }
        }

        // 2. Unparent from old hierarchy and attach to newWorld root
        if(tr->m_parent)
        {
            hierarchy_child_remove(tr->m_parent.ptr_, tr.ptr_);
        }
        tr->setParent(newWorld->irs->mainObject->transform(), false);

        // 3. Move scripts from this world binders to new world
        for(TransformRef &t : allTransforms)
        {
            if(!t || !t->_owner)
                continue;
            for(ComponentRef &comp : t->_owner->m_components)
            {
                if(auto beh = dynamic_cast<Behaviour*>(comp.ptr_))
                {
                    for(auto &pair : irs->runtimeScriptBinders)
                    {
                        if(pair.second.erase(beh) > 0)
                        {
                            newWorld->irs->runtimeScriptBinders[pair.first].insert(beh);
                        }
                    }
                }
            }
            // 4. Update into new matrix
            if(t->_owner->m_active)
            {
                World *prevWorld = currentWorld;
                currentWorld = newWorld;
                Matrix::matrix_update(t.ptr_);
                currentWorld = prevWorld;
            }
        }

        // 5. Transfer object count between worlds (GameObjects + Components)
        int movedCount = 0;
        for(TransformRef &t : allTransforms)
        {
            if(t && t->_owner)
            {
                ++movedCount;
                movedCount += t->_owner->m_components.size();
            }
        }
        this->irs->objects -= movedCount;
        newWorld->irs->objects += movedCount;
    }

    std::string World::GetTreeOfHierarchy() const
    {
        if(irs == nullptr || irs->mainObject.isNull())
            return {};
        std::string result;
        std::function<void(TransformRef, int)> traverse = [&](TransformRef cur, int depth)
        {
            if(!cur || !cur->gameObject())
                return;
            result.append(depth * 2, ' ');
            result += cur->name();
            result += "\n";
            for(TransformRef &child : cur->hierarchy)
            {
                traverse(child, depth + 1);
            }
        };
        traverse(irs->mainObject->transform(), 0);
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
        if(irs->mainObject.isNull())
            return all_gobjects;

        std::list<TransformRef> queue;
        queue.push_back(irs->mainObject->transform());

        while(!queue.empty())
        {
            TransformRef cur = queue.front();
            queue.pop_front();
            if(cur.isNull() || cur->gameObject().isNull())
                continue;

            if(cur.ptr_ != irs->mainObject->transform().ptr_)
            {
                all_gobjects.push_back(cur->gameObject());
            }

            for(TransformRef &child : cur->hierarchy)
            {
                if(!child.isNull())
                    queue.push_back(child);
            }
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
