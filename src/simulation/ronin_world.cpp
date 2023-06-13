#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

// TODO: optimize MatrixSelection USE TRANSFORM::LAYER component for selection (as INDEX)

namespace RoninEngine::Runtime
{
    extern World* switched_world;

    void load_world(World* world)
    {
        if (world == nullptr) {
            throw std::runtime_error("arg is null");
        }

        if (world->internal_resources == nullptr) {
            RoninMemory::alloc_self(world->internal_resources);
            RoninMemory::alloc_self(world->internal_resources->gui, world);
        }
        // world->internal_resources->_firstRunScripts = nullptr;
        // world->internal_resources->_realtimeScripts = nullptr;
        // world->internal_resources->_destructTasks = nullptr;
        // world->internal_resources->_destroyed = 0;
        // world->internal_resources->_level_ids_ = 0;
        // world->internal_resources->_destroy_delay_time = 0;
        // world->internal_resources->request_unloading = false;
    }

    bool unload_world(World* world)
    {
        World* lastLevel;
        GameObject* target; // first;

        if (world == nullptr) {
            throw std::runtime_error("arg is null");
        }

        if (world->internal_resources == nullptr)
            return false;

        target = world->internal_resources->main_object;

        lastLevel = switched_world;
        switched_world = world;

        world->request_unload();

        // unloading owner
        world->on_unloading();

        if (world->internal_resources->_firstRunScripts) {
            RoninMemory::free(world->internal_resources->_firstRunScripts);
            world->internal_resources->_firstRunScripts = nullptr;
        }
        if (world->internal_resources->_realtimeScripts) {
            RoninMemory::free(world->internal_resources->_realtimeScripts);
            world->internal_resources->_realtimeScripts = nullptr;
        }
        if (world->internal_resources->_destructTasks) {
            RoninMemory::free(world->internal_resources->_destructTasks);
            world->internal_resources->_destructTasks = nullptr;
        }

        // free GUI objects
        if (world->internal_resources->gui) {
            RoninMemory::free(world->internal_resources->gui);
            world->internal_resources->gui = nullptr;
        }

        // TODO: Not released world (Prev-unload), freed audio channels too...

        // Halt all channels
        Mix_HaltChannel(-1);

        // reallocate channels
        Mix_AllocateChannels(MIX_CHANNELS);

        // free Audio objects
        for (AudioClip& ac : world->internal_resources->offload_audioclips) {
            Mix_FreeChunk(ac.mix_chunk);
        }

        std::list<GameObject*> stacks;
        // free objects
        while (target) {
            for (Transform* e : *World::get_hierarchy(target->transform())) {
                stacks.emplace_front(e->game_object());
            }

            // destroy
            destroy_immediate(target);

            if (stacks.empty()) {
                target = nullptr;
            } else {
                target = stacks.front();
                stacks.pop_front();
            }
        }

        world->internal_resources->main_object = nullptr;

        if (!world->internal_resources->world_objects.empty()) {
            throw std::runtime_error("that's objects isn't release");
        }

        // free native resources
        for (auto sprite : world->internal_resources->offload_sprites) {
            RoninMemory::free(sprite);
        }

        for (auto surface : world->internal_resources->offload_surfaces) {
            SDL_FreeSurface(surface);
        }

        RoninMemory::free(world->internal_resources);
        world->internal_resources = nullptr;

        switched_world = lastLevel;
        return true;
    }
}

World::World(const std::string& name)
    : m_name(name)
{
}

World::~World()
{
    // unloading
    unload_world(this);
}

// NOTE: Check game hierarchy
std::list<Transform*> World::matrix_check_damaged()
{
    std::list<Transform*> damaged;

    for (auto x = std::begin(switched_world->internal_resources->matrixWorld); x != end(switched_world->internal_resources->matrixWorld); ++x) {
        for (auto y = begin(x->second); y != end(x->second); ++y) {
            if (Vec2::round_to_int((*y)->position()) != x->first) {
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
int World::matrix_restore(const std::list<Runtime::Transform*>& damaged_content)
{
    int restored = 0;

    for (Runtime::Transform* dam : damaged_content) {
        Vec2Int find = Vec2::round_to_int(dam->p);
        auto vertList = switched_world->internal_resources->matrixWorld.find(find);

        if (vertList == std::end(switched_world->internal_resources->matrixWorld))
            continue;

        if (vertList->first != find)
            static_assert(true, "Hash function is collision");

        auto fl = vertList->second.find(dam);

        if (fl == end(vertList->second)) {
            // restore
            vertList->second.emplace(dam);
            ++restored;
        }
    }

    return restored;
}

void World::get_render_info(int* culled, int* fullobjects)
{
    Camera* curCamera = Camera::main_camera();

    if (!curCamera)
        return;

    if (culled) {
        (*culled) = -1;
    }

    if (fullobjects) {
        (*fullobjects) = -1;
    }
}

void World::matrix_nature(Transform* target, Vec2Int lastPoint) { matrix_nature(target, Vec2::round_to_int(target->position()), lastPoint); }

// THIS is matrix get from world space
void World::matrix_nature(Runtime::Transform* target, const RoninEngine::Runtime::Vec2Int& newPoint, const RoninEngine::Runtime::Vec2Int& lastPoint)
{
    if (newPoint == lastPoint)
        return;

    // 1. delete last point source
    auto iter = internal_resources->matrixWorld.find(lastPoint);

    if (std::end(internal_resources->matrixWorld) != iter) {
        iter->second.erase(target);

        // 2. erase empty matrix element
        if (iter->second.empty())
            internal_resources->matrixWorld.erase(iter);
    }

    // 3. add new point source
    internal_resources->matrixWorld[newPoint].insert(target);
}

void World::matrix_nature_pickup(Runtime::Transform* target)
{
    auto iter = internal_resources->matrixWorld.find(Vec2::round_to_int(target->position()));

    if (std::end(internal_resources->matrixWorld) != iter) {
        iter->second.erase(target);
        // erase empty matrix element
        if (iter->second.empty())
            internal_resources->matrixWorld.erase(iter);
    }
}

void World::push_light_object(Light* light) { internal_resources->_assoc_lightings.emplace_front(light); }

void World::push_object(Object* obj) { internal_resources->world_objects.insert(obj); }

std::list<Transform*>* World::get_hierarchy(Runtime::Transform* parent)
{
    if (!parent) {
        Application::fail("Argument is null");
    }
    return &parent->hierarchy;
}

void World::internal_bind_script(Behaviour* script)
{
    if (!internal_resources->_firstRunScripts)
        RoninMemory::alloc_self(internal_resources->_firstRunScripts);

    if (std::find_if(begin(*(this->internal_resources->_firstRunScripts)), end(*(this->internal_resources->_firstRunScripts)), std::bind2nd(std::equal_to<Behaviour*>(), script)) == end(*(internal_resources->_firstRunScripts))) {
        if (internal_resources->_realtimeScripts
            && std::find_if(begin(*(internal_resources->_realtimeScripts)), end(*(internal_resources->_realtimeScripts)), std::bind2nd(std::equal_to<Behaviour*>(), script)) != end(*(internal_resources->_realtimeScripts)))
            return;

        internal_resources->_firstRunScripts->emplace_back(script);
    }
}

void World::runtime_destructs()
{
    internal_resources->_destroyed = 0;
    // Destroy queue objects
    if (internal_resources->_destructTasks && internal_resources->_destroy_delay_time < TimeEngine::time()) {
        float time = TimeEngine::time();
        std::map<float, std::set<GameObject*>>::iterator xiter = std::end(*(internal_resources->_destructTasks)), yiter = std::end(*(internal_resources->_destructTasks));
        for (std::pair<const float, std::set<GameObject*>>& pair : *(internal_resources->_destructTasks)) {
            // The time for the destruction of the object has not yet come
            if (pair.first > time) {
                internal_resources->_destroy_delay_time = pair.first;
                break;
            }
            if (xiter == std::end(*(internal_resources->_destructTasks)))
                xiter = yiter = std::begin(*(internal_resources->_destructTasks));

            // destroy
            for (auto target : pair.second) {
                if (target->exists()) {
                    // run destructor
                    destroy_immediate(target);
                    ++(internal_resources->_destroyed);
                }
            }
            ++yiter;
        }

        if (xiter != yiter)
            internal_resources->_destructTasks->erase(xiter, yiter);

        if (internal_resources->_destructTasks->empty()) {
            RoninMemory::free(internal_resources->_destructTasks);
            internal_resources->_destructTasks = nullptr;
        }
    }
}

void World::level_render_world(SDL_Renderer* renderer, ScoreWatcher* watcher)
{
    // set default color
    Gizmos::set_color(Color::white);

    if (watcher == nullptr)
        throw std::runtime_error("arg watcher is null");

    TimeEngine::begin_watch();

    if (internal_resources->_firstRunScripts) {
        if (!internal_resources->_realtimeScripts) {
            RoninMemory::alloc_self(internal_resources->_realtimeScripts);
        }

        for (Behaviour* exec : *(internal_resources->_firstRunScripts)) {
            if (exec->game_object()->m_active)
                exec->OnStart(); // go start (first draw)
            internal_resources->_realtimeScripts->emplace_back(exec);
        }
        RoninMemory::free(internal_resources->_firstRunScripts);
        internal_resources->_firstRunScripts = nullptr;
    }

    if (internal_resources->_realtimeScripts) {
        for (Behaviour* exec : *(internal_resources->_realtimeScripts)) {
            if (exec->game_object()->m_active)
                exec->OnUpdate();
        };
    }
    watcher->ms_wait_exec_scripts = TimeEngine::end_watch();
    // end watcher

    TimeEngine::begin_watch();
    // Render on main camera
    Camera* cam = Camera::main_camera(); // Draw level from active camera (main)
    if (!switched_world->internal_resources->request_unloading && cam) {
        Resolution res = Application::get_resolution();

        // FlushCache last result
        if (cam->targetClear)
            cam->renders.clear();

        // draw world in world size
        cam->render(renderer, { 0, 0, res.width, res.height }, internal_resources->main_object);
    }
    watcher->ms_wait_render_collect = TimeEngine::end_watch();

    // begin watcher
    TimeEngine::begin_watch();
    if (!switched_world->internal_resources->request_unloading && Camera::main_camera()) {
        if (internal_resources->_realtimeScripts) {
            for (auto exec : *(internal_resources->_realtimeScripts)) {
                if (exec->game_object()->m_active)
                    exec->OnGizmos();
            };
        }
        on_gizmo(); // Draw gizmos
    }
    watcher->ms_wait_render_gizmos = TimeEngine::end_watch();
    // end watcher

    TimeEngine::begin_watch();
    if (!switched_world->internal_resources->request_unloading)
        runtime_destructs();
    watcher->ms_wait_destructions = TimeEngine::end_watch();
}

void World::level_render_ui(SDL_Renderer* renderer)
{
    // render UI
    internal_resources->gui->native_draw_render(renderer);
}

void World::level_render_world_late(SDL_Renderer* renderer)
{
    on_late_update();
    if (internal_resources->_realtimeScripts) {
        for (auto exec : *(internal_resources->_realtimeScripts)) {
            if (exec->game_object()->m_active)
                exec->OnLateUpdate();
        }
    }
}
bool World::is_hierarchy() { return this->internal_resources->main_object != nullptr; }

std::string& World::name() { return this->m_name; }

UI::GUI* World::get_gui() { return this->internal_resources->gui; }

void World::request_unload() { this->internal_resources->request_unloading = true; }

int World::get_destroyed_frames() { return internal_resources->_destroyed; }

std::list<GameObject*> World::get_all_gameobjects()
{
    std::list<GameObject*> all_gobjects;
    GameObject* next = internal_resources->main_object;

    while (next) {
        for (Transform* e : *World::get_hierarchy(next->transform())) {
            all_gobjects.emplace_front(e->game_object());
        }
        if (next == all_gobjects.front())
            next = nullptr;
        else
            next = all_gobjects.front();
    }
    return all_gobjects;
}

std::list<Component*> World::get_all_components()
{
    std::list<Component*> components;
    std::list<GameObject*> all_objects = get_all_gameobjects();

    for (GameObject* curObject : all_objects) {
        for (Component* self_component : curObject->m_components) {
            components.emplace_back(self_component);
        }
    }
    return components;
}

const bool World::object_desctruction_cancel(GameObject* obj)
{
    if (!obj || !obj->exists()) {
        Application::fail_oom_kill();
        return false;
    }

    if (World::self()->internal_resources->_destructTasks) {
        for (std::pair<const float, std::set<GameObject*>>& mapIter : *World::self()->internal_resources->_destructTasks) {
            auto iter = mapIter.second.find(obj);
            if (iter != std::end(mapIter.second)) {
                mapIter.second.erase(iter);
                return true; // canceled
            }
        }
    }
    return false;
}

const int World::object_destruction_cost(GameObject* obj)
{
    int x;
    if (!obj || !obj->exists()) {
        Application::fail_oom_kill();
        return false;
    }
    if (World::self()->internal_resources->_destructTasks) {
        x = 0;
        for (std::pair<const float, std::set<GameObject*>>& mapIter : *World::self()->internal_resources->_destructTasks) {
            if (mapIter.second.find(obj) != std::end(mapIter.second)) {
                return x;
            }
            ++x;
        }
    }

    return -1;
}

const bool World::object_destruction_state(GameObject* obj) { return object_destruction_cost(obj) > ~0; }

const int World::object_destruction_count()
{
    int x = 0;
    if (World::self()->internal_resources->_destructTasks) {
        for (std::pair<const float, std::set<GameObject*>>& mapIter : *World::self()->internal_resources->_destructTasks) {
            x += mapIter.second.size();
        }
    }
    return x;
}

void World::on_awake() { }

void World::on_start() { }

void World::on_update() { }

void World::on_late_update() { }

void World::on_gizmo() { }

void World::on_unloading() { }

World* World::self() { return switched_world; }
