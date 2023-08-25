#include "ronin.h"
#include "ronin_matrix.h"
using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

// TODO: optimize MatrixSelection USE TRANSFORM::LAYER component for selection (as INDEX)

namespace RoninEngine
{
    namespace Runtime
    {
        void WorldResources::event_camera_changed(Camera *target, CameraEvent state)
        {
            switch(state)
            {
                case CameraEvent::CAM_DELETED:
                    // TODO: find free Camera and set as Main
                    if(main_camera == target)
                        main_camera = nullptr;
                    break;
                case CameraEvent::CAM_TARGET:
                    main_camera = target;
                    break;
            }
        }
        void load_world(World *world)
        {
            if(world == nullptr)
            {
                throw ronin_null_error();
            }

            if(world->internal_resources == nullptr)
            {
                RoninMemory::alloc_self(world->internal_resources);
                RoninMemory::alloc_self(world->internal_resources->gui, world);
            }
        }

        bool unload_world(World *world)
        {
            World *lastLevel;
            GameObject *target; // first;

            if(world == nullptr)
            {
                throw ronin_null_error();
            }

            if(world->internal_resources == nullptr)
                return false;

            lastLevel = switched_world;
            switched_world = world;

            world->RequestUnload();

            // unloading owner
            world->OnUnloading();

            if(world->internal_resources->_firstRunScripts)
            {
                RoninMemory::free(world->internal_resources->_firstRunScripts);
                world->internal_resources->_firstRunScripts = nullptr;
            }
            if(world->internal_resources->_realtimeScripts)
            {
                RoninMemory::free(world->internal_resources->_realtimeScripts);
                world->internal_resources->_realtimeScripts = nullptr;
            }
            if(world->internal_resources->_destructTasks)
            {
                RoninMemory::free(world->internal_resources->_destructTasks);
                world->internal_resources->_destructTasks = nullptr;
            }

            // free GUI objects
            if(world->internal_resources->gui)
            {
                RoninMemory::free(world->internal_resources->gui);
                world->internal_resources->gui = nullptr;
            }

            // Halt all channels
            Mix_HaltChannel(-1);

            // reallocate channels
            Mix_AllocateChannels(MIX_CHANNELS);

            // free Audio objects
            for(AudioClip &ac : world->internal_resources->offload_audioclips)
            {
                Mix_FreeChunk(ac.mix_chunk);
            }

            // NOTE: Free Local Resources
            gid_resources_free(world->internal_resources->external_local_resources);
            world->internal_resources->external_local_resources = nullptr;

            target = world->internal_resources->main_object;
            std::list<GameObject *> stacks;
            // free objects
            while(target)
            {
                for(Transform *e : target->transform()->hierarchy)
                {
                    stacks.emplace_front(e->gameObject());
                }

                // destroy
                destroy_immediate(target);

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

            world->internal_resources->main_object = nullptr;

            if(!world->internal_resources->world_objects.empty())
            {
                throw ronin_cant_release_error();
            }

            // free native resources
            for(auto sprite : world->internal_resources->offload_sprites)
            {
                RoninMemory::free(sprite);
            }

            for(auto surface : world->internal_resources->offload_surfaces)
            {
                SDL_FreeSurface(surface);
            }

            for(CameraResource *cam_res : world->internal_resources->camera_resources)
            {
                RoninMemory::free(cam_res);
            }

            RoninMemory::free(world->internal_resources);
            world->internal_resources = nullptr;

            switched_world = lastLevel;
            return true;
        }
    } // namespace Runtime

    World::World(const std::string &name) : m_name(name)
    {
    }

    World::~World()
    {
        // unloading
        unload_world(this);
    }

    // NOTE: Check game hierarchy
    std::list<Transform *> World::matrix_check_damaged()
    {
        std::list<Transform *> damaged;

        for(auto x = std::begin(switched_world->internal_resources->matrix); x != end(switched_world->internal_resources->matrix); ++x)
        {
            for(auto y = begin(x->second); y != end(x->second); ++y)
            {
                if(Matrix::matrix_get_key((*y)->position()) != x->first)
                {
                    damaged.emplace_back(*y);
                }
            }
        }
        return damaged;
    }

    int World::matrix_restore()
    {
        auto damaged = matrix_check_damaged();
        return matrix_restore(damaged);
    }

    // try restore damaged matrix element's
    int World::matrix_restore(const std::list<Runtime::Transform *> &damaged_content)
    {
        int restored = 0;

        for(Transform *dam : damaged_content)
        {
            Vec2Int find = Matrix::matrix_get_key(dam->position());
            auto vertList = switched_world->internal_resources->matrix.find(find);

            if(vertList == std::end(switched_world->internal_resources->matrix))
                continue;

            // FIXME: Hash function is collision
            //  if (vertList->first != find)

            auto fl = vertList->second.find(dam);

            if(fl == end(vertList->second))
            {
                // restore
                vertList->second.emplace(dam);
                ++restored;
            }
        }

        return restored;
    }

    int World::GetCulled()
    {
        if(internal_resources == nullptr || internal_resources->main_camera == nullptr)
            return -1;

        return internal_resources->main_camera->camera_resources->renders.size();
    }

    int World::matrix_count_cache()
    {
        int cached = 0;
        auto &matrix = this->internal_resources->matrix;
        cached = static_cast<int>(std::count_if(
            matrix.begin(),
            matrix.end(),
            [](auto iobject)
            {
                // predicate return
                return (bool) (iobject.second.empty());
            }));
        return cached;
    }

    int World::matrix_clear_cache()
    {
        std::list<typename decltype(this->internal_resources->matrix)::iterator> cached {};
        auto matrix = &this->internal_resources->matrix;
        for(auto iter = std::begin(internal_resources->matrix), _end = std::end(internal_resources->matrix); iter != _end; ++iter)
        {
            if(iter->second.empty())
            {
                cached.emplace_back(iter);
            }
        }

        for(auto &iter_ref : cached)
        {
            internal_resources->matrix.erase(iter_ref);
        }

        return cached.size();
    }

    std::string World::get_hierarchy_as_tree() const
    {
        static char delim = 0x32;
        std::string delims;
        std::string result;
        std::list<Runtime::Transform *> stack;
        Transform *target = this->internal_resources->main_object->transform();

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

    void World::push_light_object(Light *light)
    {
        internal_resources->_assoc_lightings.emplace_front(light);
    }

    void World::push_object(Object *obj)
    {
        internal_resources->world_objects.insert(obj);
    }

    void World::internal_bind_script(Behaviour *script)
    {
        if(!internal_resources->_firstRunScripts)
            RoninMemory::alloc_self(internal_resources->_firstRunScripts);

        if(std::find_if(
               begin(*(this->internal_resources->_firstRunScripts)),
               end(*(this->internal_resources->_firstRunScripts)),
               std::bind2nd(std::equal_to<Behaviour *>(), script)) == end(*(internal_resources->_firstRunScripts)))
        {
            if(internal_resources->_realtimeScripts &&
               std::find_if(
                   begin(*(internal_resources->_realtimeScripts)),
                   end(*(internal_resources->_realtimeScripts)),
                   std::bind2nd(std::equal_to<Behaviour *>(), script)) != end(*(internal_resources->_realtimeScripts)))
                return;

            internal_resources->_firstRunScripts->emplace_back(script);
        }
    }

    void World::runtime_destructs()
    {
        internal_resources->_destroyed = 0;
        // Destroy queue objects
        if(internal_resources->_destructTasks && internal_resources->_destroy_delay_time < TimeEngine::time())
        {
            float time = TimeEngine::time();
            std::map<float, std::set<GameObject *>>::iterator xiter = std::end(*(internal_resources->_destructTasks)),
                                                              yiter = std::end(*(internal_resources->_destructTasks));
            for(std::pair<const float, std::set<GameObject *>> &pair : *(internal_resources->_destructTasks))
            {
                // The time for the destruction of the object has not yet come
                if(pair.first > time)
                {
                    internal_resources->_destroy_delay_time = pair.first;
                    break;
                }
                if(xiter == std::end(*(internal_resources->_destructTasks)))
                    xiter = yiter = std::begin(*(internal_resources->_destructTasks));

                // destroy
                for(auto target : pair.second)
                {
                    if(target->exists())
                    {
                        // run destructor
                        destroy_immediate(target);
                        ++(internal_resources->_destroyed);
                    }
                }
                ++yiter;
            }

            if(xiter != yiter)
                internal_resources->_destructTasks->erase(xiter, yiter);

            if(internal_resources->_destructTasks->empty())
            {
                RoninMemory::free(internal_resources->_destructTasks);
                internal_resources->_destructTasks = nullptr;
            }
        }
    }

    void World::level_render_world(ScoreWatcher *watcher)
    {
        // set default color
        Gizmos::set_color(Color::white);

        if(watcher == nullptr)
            throw ronin_null_error();

        TimeEngine::begin_watch();

        if(internal_resources->_firstRunScripts)
        {
            if(!internal_resources->_realtimeScripts)
            {
                RoninMemory::alloc_self(internal_resources->_realtimeScripts);
            }

            for(Behaviour *exec : *(internal_resources->_firstRunScripts))
            {
                if(exec->gameObject()->m_active)
                    exec->OnStart(); // go start (first draw)
                internal_resources->_realtimeScripts->emplace_back(exec);
            }
            RoninMemory::free(internal_resources->_firstRunScripts);
            internal_resources->_firstRunScripts = nullptr;
        }

        if(internal_resources->_realtimeScripts)
        {
            for(Behaviour *exec : *(internal_resources->_realtimeScripts))
            {
                if(exec->gameObject()->m_active)
                    exec->OnUpdate();
            };
        }
        watcher->ms_wait_exec_scripts = TimeEngine::end_watch();
        // end watcher

        TimeEngine::begin_watch();
        // Render on main camera
        Camera *cam = Camera::main_camera(); // Draw level from active camera (main)
        if(!switched_world->internal_resources->request_unloading && cam)
        {
            // FlushCache last result
            if(cam->camera_resources->targetClear)
                cam->camera_resources->renders.clear();

            // draw world in world size
            RoninEngine::Runtime::native_render_2D(reinterpret_cast<Camera2D *>(cam));
        }
        watcher->ms_wait_render_world = TimeEngine::end_watch();

        // begin watcher
        TimeEngine::begin_watch();
        if(!switched_world->internal_resources->request_unloading && cam)
        {
            this->OnGizmos(); // Draw gizmos

            if(internal_resources->_realtimeScripts)
            {
                for(auto exec : *(internal_resources->_realtimeScripts))
                {
                    if(exec->gameObject()->m_active)
                        exec->OnGizmos();
                };
            }
        }
        if(ronin_debug_mode)
        {
            using Gizmos = RoninEngine::Runtime::Gizmos;
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
            Vec2Int screen_point = {g_size.x, static_cast<float>(active_resolution.height)};
            Vec2 g_pos = Camera::ScreenToWorldPoint({screen_point.x / 2.f, screen_point.y - g_size.y / 2});
            int x;

            if(TimeEngine::frame() % 10 == 0)
            {
                ScoreWatcher stat = RoninSimulator::get_watches();
                // Update data
                elements[0].value = stat.ms_wait_frame;
                elements[1].value = stat.ms_wait_render_gui;
                elements[2].value = stat.ms_wait_exec_scripts + stat.ms_wait_exec_world;
                elements[3].value = stat.ms_wait_render_world;
                elements[4].value = stat.ms_wait_render_gizmos;
                elements[5].value = get_process_sizeMemory() / 1024 / 1024;

                // calculate averrage and max
                max = 0;
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
            Gizmos::set_color(Color(0, 0, 0, 100));

            // Draw box
            Gizmos::draw_fill_rect_rounded(g_pos, g_size.x / pixelsPerPoint, g_size.y / pixelsPerPoint, 8);

            screen_point.x = 10;
            screen_point.y -= static_cast<int>(g_size.y) - font_height / 2;
            Gizmos::set_color(Color::white);
            Gizmos::draw_text_to_screen(screen_point, elements[0].format, font_height);
            for(x = 1; x < max_elements; ++x)
            {
                Gizmos::set_color(elements[x].format_color);

                screen_point.y += font_height + 1;
                Gizmos::draw_text_to_screen(screen_point, elements[x].format, font_height);
            }
        }
        watcher->ms_wait_render_gizmos = TimeEngine::end_watch();
        // end watcher

        TimeEngine::begin_watch();
        if(!switched_world->internal_resources->request_unloading)
            runtime_destructs();
        watcher->ms_wait_destructions = TimeEngine::end_watch();
    }

    void World::level_render_world_late()
    {
        if(internal_resources->_realtimeScripts)
        {
            for(auto exec : *(internal_resources->_realtimeScripts))
            {
                if(exec->gameObject()->m_active)
                    exec->OnLateUpdate();
            }
        }
    }
    bool World::isHierarchy()
    {
        return this->internal_resources->main_object != nullptr;
    }

    std::string &World::name()
    {
        return this->m_name;
    }

    UI::GUI *World::getGUI()
    {
        return this->internal_resources->gui;
    }

    void World::RequestUnload()
    {
        this->internal_resources->request_unloading = true;
    }

    int World::GetDestroyedFrames()
    {
        return internal_resources->_destroyed;
    }

    std::list<GameObject *> World::GetAllGameObjects()
    {
        std::list<GameObject *> all_gobjects;
        GameObject *next = internal_resources->main_object;

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
            RoninSimulator::fail_oom_kill();
        }
        else if(World::self()->internal_resources->_destructTasks)
        {
            for(std::pair<const float, std::set<GameObject *>> &mapIter : *World::self()->internal_resources->_destructTasks)
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
            RoninSimulator::fail_oom_kill();
            return false;
        }
        if(World::self()->internal_resources->_destructTasks)
        {
            x = 0;
            for(std::pair<const float, std::set<GameObject *>> &mapIter : *World::self()->internal_resources->_destructTasks)
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
        if(World::self()->internal_resources->_destructTasks)
        {
            for(std::pair<const float, std::set<GameObject *>> &mapIter : *World::self()->internal_resources->_destructTasks)
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
