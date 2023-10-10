#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

namespace RoninEngine
{
    namespace Runtime
    {
        extern std::set<World *> pinned_worlds;

        void WorldResources::event_camera_changed(Camera *target, CameraEvent state)
        {
            switch(state)
            {
                case CameraEvent::CAM_DELETED:
                    // TODO: find free Camera and set as Main
                    if(main_camera == target)
                        main_camera = nullptr;
                    camera_resources.remove(target->camera_resource);
                    RoninMemory::free(target->camera_resource);
                    break;
                case CameraEvent::CAM_TARGET:
                    main_camera = target;
                    break;
            }
        }
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
                world->irs->request_unloading = false;

                // update internal loaded font
                RoninEngine::UI::refresh_legacy_font(world);

                pinned_worlds.insert(world);
            }
        }

        bool internal_unload_world(World *world)
        {
            if(world == nullptr)
            {
                throw ronin_null_error();
            }

            if(world->irs == nullptr)
                return false;

            World *lastWorld = switched_world;
            switched_world = world;

            world->RequestUnload();

            // unloading owner
            world->OnUnloading();

            // Free Game Objects
            Transform *target = world->irs->main_object->transform();
            std::list<Transform *> stacks;
            while(target)
            {
                stacks.merge(target->hierarchy);

                // destroy
                harakiri_GameObject(target->_owner);

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
            world->irs->main_object = nullptr;

            if(world->irs->objects != 0)
            {
                RoninSimulator::Log("World is have leak objects: " + std::to_string(world->irs->objects));
            }

            if(world->irs->_firstRunScripts)
            {
                RoninMemory::free(world->irs->_firstRunScripts);
                world->irs->_firstRunScripts = nullptr;
            }
            if(world->irs->runtimeScripts)
            {
                RoninMemory::free(world->irs->runtimeScripts);
                world->irs->runtimeScripts = nullptr;
            }
            if(world->irs->runtimeCollectors)
            {
                RoninMemory::free(world->irs->runtimeCollectors);
                world->irs->runtimeCollectors = nullptr;
            }

            // free GUI objects
            if(world->irs->gui)
            {
                RoninMemory::free(world->irs->gui);
                world->irs->gui = nullptr;
            }

            // NOTE: Free Local Resources
            gid_resources_free(world->irs->external_local_resources);
            world->irs->external_local_resources = nullptr;

            // Halt all channels
            Mix_HaltChannel(-1);

            // reallocate channels
            Mix_AllocateChannels(MIX_CHANNELS);

            // free native resources
            for(Sprite *sprite : world->irs->offload_sprites)
            {
                RoninMemory::free(sprite);
            }

            for(SDL_Surface *surface : world->irs->offload_surfaces)
            {
                SDL_FreeSurface(surface);
            }

            for(CameraResource *cam_res : world->irs->camera_resources)
            {
                RoninMemory::free(cam_res);
            }

            RoninMemory::free(world->irs);
            world->irs = nullptr;

            pinned_worlds.erase(world);

            switched_world = lastWorld;
            return true;
        }

        void internal_bind_script(Behaviour *script)
        {
            if(!switched_world->irs->_firstRunScripts)
                RoninMemory::alloc_self(switched_world->irs->_firstRunScripts);

            if(std::find_if(
                   begin(*(switched_world->irs->_firstRunScripts)),
                   end(*(switched_world->irs->_firstRunScripts)),
                   std::bind(std::equal_to<Behaviour *>(), std::placeholders::_1, script)) == end(*(switched_world->irs->_firstRunScripts)))
            {
                if(switched_world->irs->runtimeScripts &&
                   std::find_if(
                       begin(*(switched_world->irs->runtimeScripts)),
                       end(*(switched_world->irs->runtimeScripts)),
                       std::bind(std::equal_to<Behaviour *>(), std::placeholders::_1, script)) !=
                       end(*(switched_world->irs->runtimeScripts)))
                    return;

                switched_world->irs->_firstRunScripts->emplace_back(script);
            }
        }

        void level_render_world()
        {
            // set default color
            Gizmos::SetColor(Color::white);

            TimeEngine::BeginWatch();

            if(switched_world->irs->_firstRunScripts)
            {
                if(!switched_world->irs->runtimeScripts)
                {
                    RoninMemory::alloc_self(switched_world->irs->runtimeScripts);
                }

                for(Behaviour *exec : *(switched_world->irs->_firstRunScripts))
                {
                    if(exec->gameObject()->m_active)
                        exec->OnStart(); // go start (first draw)
                    switched_world->irs->runtimeScripts->emplace_back(exec);
                }
                RoninMemory::free(switched_world->irs->_firstRunScripts);
                switched_world->irs->_firstRunScripts = nullptr;
            }

            if(switched_world->irs->runtimeScripts)
            {
                for(auto exec = std::begin(*switched_world->irs->runtimeScripts); exec != std::end(*switched_world->irs->runtimeScripts);
                    ++exec)
                {
                    if((*exec)->exists() && (*exec)->gameObject()->m_active)
                        (*exec)->OnUpdate();
                };
            }
            queue_watcher.ms_wait_exec_scripts = TimeEngine::EndWatch();
            // end watcher

            TimeEngine::BeginWatch();
            // Render on main camera
            Camera *cam = Camera::mainCamera(); // Draw level from active camera (main)
            if(!switched_world->irs->request_unloading && cam)
            {
                // draw world in world size
                RoninEngine::Runtime::native_render_2D(reinterpret_cast<Camera2D *>(cam));
            }
            queue_watcher.ms_wait_render_world = TimeEngine::EndWatch();

            // begin watcher
            TimeEngine::BeginWatch();
            if(!switched_world->irs->request_unloading && cam)
            {
                switched_world->OnGizmos(); // Draw gizmos

                if(switched_world->irs->runtimeScripts)
                {
                    for(auto exec : *(switched_world->irs->runtimeScripts))
                    {
                        if(exec->gameObject()->m_active)
                            exec->OnGizmos();
                    };
                }
            }

            if(ronin_debug_mode)
            {
                constexpr int font_height = 12;

                static struct
                {
                    const char *label;

                    std::uint32_t value;
                    std::string format;
                    Color format_color;
                } elements[] = {{"Render Frame", 0}, {"GUI", 0}, {"Scripts", 0}, {"Render", 0}, {"Gizmos", 0}, {"Memory", 0}};

                static std::uint32_t max_elements = sizeof(elements) / sizeof(elements[0]);
                static std::uint32_t max;
                static std::uint32_t averrage;

                Vec2 g_size = Vec2 {138, static_cast<float>(font_height * (max_elements + 2))};
                Vec2Int screen_point = Vec2::RoundToInt({g_size.x, static_cast<float>(active_resolution.height)});
                Vec2 g_pos = Camera::ScreenToWorldPoint({screen_point.x / 2.f, screen_point.y - g_size.y / 2});
                int x;

                if(TimeEngine::frame() % 10 == 0)
                {
                    TimingWatcher stat = RoninSimulator::GetTimingWatches();
                    // Update data
                    elements[0].value = stat.ms_wait_frame;
                    elements[1].value = stat.ms_wait_render_gui;
                    elements[2].value = stat.ms_wait_exec_scripts + stat.ms_wait_exec_world;
                    elements[3].value = stat.ms_wait_render_world;
                    elements[4].value = stat.ms_wait_render_gizmos;
                    elements[5].value = get_process_sizeMemory() / 1024 / 1024;

                    // calculate averrage and max
                    max = 10;
                    averrage = 0;
                    for(x = 1; x < max_elements - 1; ++x)
                    {
                        max = std::max(elements[x].value, max);
                        averrage += elements[x].value;
                    }
                    averrage /= std::max(1, x);

                    // format text
                    for(x = 0; x < max_elements; ++x)
                    {
                        if(x != 0)
                            elements[x].format = " > ";
                        else
                            elements[x].format.clear();
                        elements[x].format += elements[x].label;
                        elements[x].format += ": " + std::to_string(elements[x].value) + ' ';

                        // format color
                        if(max && elements[x].value == max)
                            elements[x].format_color = Color::red;
                        else if(averrage && elements[x].value >= averrage)
                            elements[x].format_color = Color::yellow;
                        else
                            elements[x].format_color = Color::white;
                    }

                    // write ISO
                    // ms
                    for(x = 0; x < max_elements - 1; ++x)
                        elements[x].format += "ms";
                    elements[x].format += "MiB";
                    elements[x].format_color = Color::white;
                }

                // Set background color
                Gizmos::SetColor(Color(0, 0, 0, 100));

                // Draw box
                Gizmos::DrawFillRectRounded(g_pos, g_size.x / pixelsPerPoint, g_size.y / pixelsPerPoint, 8);

                screen_point.x = 10;
                screen_point.y -= static_cast<int>(g_size.y) - font_height / 2;
                Gizmos::SetColor(Color::white);
                Gizmos::DrawTextToScreen(screen_point, elements[0].format, font_height);
                for(x = 1; x < max_elements; ++x)
                {
                    Gizmos::SetColor(elements[x].format_color);

                    screen_point.y += font_height + 1;
                    Gizmos::DrawTextToScreen(screen_point, elements[x].format, font_height);
                }
            }

            // end watcher
            queue_watcher.ms_wait_render_gizmos = TimeEngine::EndWatch();

            // Run Collector
            TimeEngine::BeginWatch();
            if(false && !switched_world->irs->request_unloading)
                RuntimeCollector();
            // end watcher
            queue_watcher.ms_wait_destructions = TimeEngine::EndWatch();
        }

        void level_render_world_late()
        {
            if(switched_world->irs->runtimeScripts)
            {
                for(auto exec : *(switched_world->irs->runtimeScripts))
                {
                    if(exec->gameObject()->m_active)
                        exec->OnLateUpdate();
                }
            }
        }

    } // namespace Runtime

    World::World(const std::string &name) : m_name(name), irs(nullptr)
    {
    }

    World::~World()
    {
        // unloading
        internal_unload_world(this);
    }

    // NOTE: Check game hierarchy
    std::list<Transform *> World::MatrixCheckDamage()
    {
        std::list<Transform *> damaged;

        for(auto x = std::begin(switched_world->irs->matrix); x != end(switched_world->irs->matrix); ++x)
        {
            // unordered_map<Vec2Int,... <Transform*>>
            for(auto &layerObject : x->second)
            {
                // set<Transform*>
                for(auto &y : layerObject.second)
                {
                    if(!object_instanced(y) || Matrix::matrix_get_key(y->_position) != layerObject.first || x->first != y->_owner->m_layer)
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
        int restored = 0;
        for(Transform *dam : damaged_content)
        {
            MatrixKey key = Matrix::matrix_get_key(dam->_position);
            // unordered_map<int, ...>
            for(auto &findIter : switched_world->irs->matrix)
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

                            if(object_instanced(dam))
                            {
                                // Restore
                                switched_world->irs->matrix[dam->_owner->m_layer][key].insert(dam);
                            }
                            ++restored;
                            goto next;
                        }
                    }

        next:
            continue;
        }

        return restored;
    }

    int World::GetCulled()
    {
        if(irs == nullptr || irs->main_camera == nullptr)
            return -1;

        return irs->main_camera->camera_resource->culled;
    }

    int World::MatrixCacheCount()
    {
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
        std::vector<typename std::unordered_map<Vec2Int, std::set<Transform *>>::iterator> cached;
        int cleans = 0;
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

    std::string World::GetTreeOfHierarchy() const
    {
        static char delim = 0x32;
        std::string delims;
        std::string result;
        std::list<Runtime::Transform *> stack;
        Transform *target = this->irs->main_object->transform();

        while(target)
        {
            for(auto c : target->hierarchy)
            {
                stack.emplace_back(c);
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
        return this->irs->main_object != nullptr;
    }

    std::string &World::name()
    {
        return this->m_name;
    }

    UI::GUI *World::getGUI()
    {
        return this->irs->gui;
    }

    void World::RequestUnload()
    {
        this->irs->request_unloading = true;
    }

    bool World::SetCursor(Cursor *cursor)
    {
        if(cursor == nullptr)
            return false;

        SDL_SetCursor(cursor);
        SDL_ShowCursor(SDL_ENABLE);
        return true;
    }

    int World::GetDestroyedFrames()
    {
        return irs->_destroyed;
    }

    std::list<GameObject *> World::GetAllGameObjects()
    {
        std::list<GameObject *> all_gobjects;
        GameObject *next = irs->main_object;

        while(next)
        {
            for(Transform *e : next->transform()->hierarchy)
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

    std::list<Component *> World::GetAllComponents()
    {
        std::list<Component *> components;
        std::list<GameObject *> all_objects = GetAllGameObjects();

        for(GameObject *curObject : all_objects)
        {
            for(Component *self_component : curObject->m_components)
            {
                components.emplace_back(self_component);
            }
        }
        return components;
    }

    const bool World::CancelObjectDestruction(GameObject *obj)
    {
        if(!obj || !obj->exists())
        {
            RoninSimulator::BreakSimulate();
        }
        else if(World::self()->irs->runtimeCollectors)
        {
            for(std::pair<const float, std::set<GameObject *>> &mapIter : *World::self()->irs->runtimeCollectors)
            {
                auto iter = mapIter.second.find(obj);
                if(iter != std::end(mapIter.second))
                {
                    mapIter.second.erase(iter);
                    return true; // canceled
                }
            }
        }
        return false;
    }

    const int World::CostObjectDestruction(GameObject *obj)
    {
        int x;
        if(!obj || !obj->exists())
        {
            RoninSimulator::BreakSimulate();
            return false;
        }
        if(World::self()->irs->runtimeCollectors)
        {
            x = 0;
            for(std::pair<const float, std::set<GameObject *>> &mapIter : *World::self()->irs->runtimeCollectors)
            {
                if(mapIter.second.find(obj) != std::end(mapIter.second))
                {
                    return x;
                }
                ++x;
            }
        }

        return -1;
    }

    const bool World::StateObjectDestruction(GameObject *obj)
    {
        return this->CostObjectDestruction(obj) > ~0;
    }

    const int World::CountObjectDestruction()
    {
        int x = 0;
        if(World::self()->irs->runtimeCollectors)
        {
            for(std::pair<const float, std::set<GameObject *>> &mapIter : *World::self()->irs->runtimeCollectors)
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

    World *World::self()
    {
        return switched_world;
    }
} // namespace RoninEngine
