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
                harakiri_GameObject(target->_owner, nullptr);

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
                RoninSimulator::Log(("World is have leak objects: " + std::to_string(world->irs->objects)).c_str());
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
            // world->irs->external_local_resources = nullptr;
            gid_resources_free(&world->irs->external_local_resources);

            // Halt all channels
            Mix_HaltChannel(-1);

            // reallocate channels
            Mix_AllocateChannels(MIX_CHANNELS);

            // free native resources
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

        void level_render_world()
        {
            // set default color
            RenderUtility::SetColor(Color::white);

            TimeEngine::BeginWatch();

            scripts_start();

            scripts_update();

            scripts_lateUpdate();

            // end watcher
            env.queue_watcher.delayExecScripts = TimeEngine::EndWatch();

            TimeEngine::BeginWatch();
            // Render on main camera
            Camera *cam = Camera::mainCamera(); // Draw level from active camera (main)
            if(!switched_world->irs->request_unloading && cam && cam->enable())
            {
                // draw world in world size
                native_render_2D(reinterpret_cast<Camera2D *>(cam));
            }
            env.queue_watcher.delayRenderWorld = TimeEngine::EndWatch();

            // begin watcher
            TimeEngine::BeginWatch();
            if(!switched_world->irs->request_unloading && cam)
            {
                // Reset Color
                RenderUtility::SetColor(Color::white);
                switched_world->OnGizmos(); // Draw gizmos

                scripts_gizmos();

                if(env.ronin_debug_mode)
                {
                    constexpr int font_height = 11;
                    static char buffer[32];
                    static struct __INFOLABEL__
                    {
                        int labelLen;

                        std::uint32_t value;
                        std::string format;
                        Color format_color;

                        __INFOLABEL__(const char *label, std::uint32_t value) : format(label)
                        {
                            this->labelLen = this->format.length();
                            this->value = value;
                        }
                    } labels[] = {
                        {"FPS: ", 0}, {" > GUI: ", 0}, {" > Scripts: ", 0}, {" > Render: ", 0}, {" > Gizmos: ", 0}, {" > Memory: ", 0}};

                    static std::uint32_t max_elements = sizeof(labels) / sizeof(labels[0]);
                    static std::uint32_t max;
                    static std::uint32_t averrage;

                    Vec2 g_size {138, static_cast<float>(font_height * (max_elements + 2))};
                    Vec2Int screen_point = Vec2::RoundToInt({g_size.x, static_cast<float>(env.active_resolution.height)});
                    Vec2 g_pos = Camera::ScreenToWorldPoint({screen_point.x / 2.f, screen_point.y - g_size.y / 2});
                    int x;

                    if(TimeEngine::frame() % 10 == 0)
                    {
                        // Update data
                        labels[0].value = env.last_watcher.delayFrameRate;
                        labels[1].value = env.last_watcher.delayRenderGUI;
                        labels[2].value = env.last_watcher.delayExecScripts + env.last_watcher.delayExecWorld;
                        labels[3].value = env.last_watcher.delayRenderWorld;
                        labels[4].value = env.last_watcher.delayRenderGizmos;
                        labels[5].value = Perfomances::GetMemoryUsed() / 1024 / 1024; // convert Bytes to MB

                        // calculate averrage and max
                        max = 10;
                        averrage = 0;
                        for(x = 1; x < max_elements - 1; ++x)
                        {
                            max = std::max(labels[x].value, max);
                            averrage += labels[x].value;
                        }
                        averrage /= std::max(1, x);

                        // Calculate FPS and timing
                        labels[0].format.resize(labels[0].labelLen);
                        snprintf(buffer, sizeof(buffer), "%d (%d ms)", static_cast<int>(1 / TimeEngine::deltaTime()), labels[0].value);
                        labels[0].format += buffer;

                        // format text
                        for(x = 1; x < max_elements; ++x)
                        {
                            labels[x].format.resize(labels[x].labelLen);
                            snprintf(buffer, sizeof(buffer), "%d ", labels[x].value);
                            labels[x].format += buffer;

                            // format color
                            if(max && labels[x].value == max)
                                labels[x].format_color = Color::red;
                            else if(averrage && labels[x].value >= averrage)
                                labels[x].format_color = Color::yellow;
                            else
                                labels[x].format_color = Color::white;
                        }

                        for(x = 1; x < max_elements - 1; ++x)
                            labels[x].format += "ms";
                        labels[x].format += "MiB";
                        labels[x].format_color = Color::white;
                    }

                    // Set background color
                    RenderUtility::SetColor(Color(0, 0, 0, 100));

                    // Draw box
                    RenderUtility::DrawFillRect(g_pos, g_size.x / pixelsPerPoint, g_size.y / pixelsPerPoint);

                    screen_point.x = 10;
                    screen_point.y -= static_cast<int>(g_size.y) - font_height / 2;
                    RenderUtility::SetColor(Color::white);
                    RenderUtility::DrawTextToScreen(screen_point, labels[0].format, font_height);
                    for(x = 1; x < max_elements; ++x)
                    {
                        RenderUtility::SetColor(labels[x].format_color);

                        screen_point.y += font_height + 1;
                        RenderUtility::DrawTextToScreen(screen_point, labels[x].format, font_height);
                    }
                }
            }

            // end watcher
            env.queue_watcher.delayRenderGizmos = TimeEngine::EndWatch();
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
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

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
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

        int restored = 0;
        for(Transform *dam : damaged_content)
        {
            matrix_key_t key = Matrix::matrix_get_key(dam->_position);
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
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

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
            throw ronin_world_notloaded_error();

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
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

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
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

        return this->irs->main_object != nullptr;
    }

    std::string &World::name()
    {
        return this->m_name;
    }

    UI::GUI *World::GetGUI()
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

        return this->irs->gui;
    }

    void World::RequestUnload()
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

        this->irs->request_unloading = true;
    }

    int World::GetDestroyedFrames()
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

        return irs->_destroyedGameObject;
    }

    std::list<GameObject *> World::GetAllGameObjects()
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

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
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

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
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

        if(obj && irs->runtimeCollectors)
        {
            // std::pair<const float, std::set<GameObject *>>
            for(auto mapIter = std::begin(*irs->runtimeCollectors); mapIter != std::end(*irs->runtimeCollectors); ++mapIter)
            {
                std::set<GameObject *>::iterator iter = mapIter->second.find(obj);
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

    const int World::CostObjectDestruction(GameObject *obj)
    {
        if(irs == nullptr)
            throw ronin_world_notloaded_error();

        int x;
        if(!obj)
        {
            RoninSimulator::BreakSimulate();
            return false;
        }
        if(irs->runtimeCollectors)
        {
            x = 0;
            for(std::pair<const float, std::set<GameObject *>> &mapIter : *irs->runtimeCollectors)
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
            for(std::pair<const float, std::set<GameObject *>> &mapIter : *irs->runtimeCollectors)
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
