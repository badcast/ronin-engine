#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

// TODO: optimize MatrixSelection USE TRANSFORM::LAYER component for selection (as INDEX)

namespace RoninEngine::Runtime
{
    extern Level* switched_level;
    bool unload_level(Level* level)
    {
        std::list<GameObject*> stacks;
        GameObject* target; // first;
        Level* lastLevel;

        if (level == nullptr) {
            throw std::runtime_error("arg level is null");
        }

        if ((target = level->main_object) == nullptr)
            return false;

        lastLevel = switched_level;
        switched_level = level;
        try {
            level->request_unload();

            // unloading owner
            level->on_unloading();

            if (level->_firstRunScripts) {
                RoninMemory::free(level->_firstRunScripts);
                level->_firstRunScripts = nullptr;
            }
            if (level->_realtimeScripts) {
                RoninMemory::free(level->_realtimeScripts);
                level->_realtimeScripts = nullptr;
            }
            if (level->_destructTasks) {
                RoninMemory::free(level->_destructTasks);
                level->_destructTasks = nullptr;
            }

            // free GUI objects
            if (level->ui) {
                RoninMemory::free(level->ui);
                level->ui = nullptr;
            }

            // free objects
            while (target) {
                for (Transform* e : *Level::get_hierarchy(target->transform())) {
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

            level->main_object = nullptr;

            if (!level->_objects.empty()) {
                throw std::runtime_error("that's objects isn't release");
                // this->_objects.clear();
            }
        } catch (ex) {
            switched_level = lastLevel;
            throw ex;
        }
        return true;
    }
}

Level::Level()
    : Level("Untitled")
{
}
Level::Level(const std::string& name)
    : main_object(nullptr)
    , _firstRunScripts(nullptr)
    , _realtimeScripts(nullptr)
    , _destructTasks(nullptr)
    , _destroyed(0)
    , _level_ids_(0)
    , _destroy_delay_time(0)
    , request_unloading(false)
    , m_name(name)
{
    if (switched_level != nullptr) {
        static_assert(true, "current level replaced by new");
    }

    RoninMemory::alloc_self(ui, this);
}
Level::~Level()
{
    // unloading
    unload_level(this);
}

// NOTE: Check game hierarchy
std::list<Transform*> Level::matrix_check_damaged()
{
    std::list<Transform*> damaged;

    for (auto x = std::begin(switched_level->matrixWorld); x != end(switched_level->matrixWorld); ++x) {
        for (auto y = begin(x->second); y != end(x->second); ++y) {
            if (Vec2::round_to_int((*y)->position()) != x->first) {
                damaged.emplace_back(*y);
            }
        }
    }
    return damaged;
}

int Level::matrix_restore()
{
    auto damaged = matrix_check_damaged();
    return matrix_restore(damaged);
}

// try restore damaged matrix element's
int Level::matrix_restore(const std::list<Runtime::Transform*>& damaged_content)
{
    int restored = 0;

    for (Runtime::Transform* dam : damaged_content) {
        Vec2Int find = Vec2::round_to_int(dam->p);
        auto vertList = switched_level->matrixWorld.find(find);

        if (vertList == std::end(switched_level->matrixWorld))
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

void Level::get_render_info(int* culled, int* fullobjects)
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

void Level::matrix_nature(Transform* target, Vec2Int lastPoint) { matrix_nature(target, Vec2::round_to_int(target->position()), lastPoint); }

// THIS is matrix get from world space
void Level::matrix_nature(Runtime::Transform* target, const RoninEngine::Runtime::Vec2Int& newPoint, const RoninEngine::Runtime::Vec2Int& lastPoint)
{
    if (newPoint == lastPoint)
        return;

    // 1. delete last point source
    auto iter = matrixWorld.find(lastPoint);

    if (std::end(matrixWorld) != iter) {
        iter->second.erase(target);

        // 2. erase empty matrix element
        if (iter->second.empty())
            matrixWorld.erase(iter);
    }

    // 3. add new point source
    matrixWorld[newPoint].insert(target);
}

void Level::matrix_nature_pickup(Runtime::Transform* target)
{
    auto iter = matrixWorld.find(Vec2::round_to_int(target->position()));

    if (std::end(matrixWorld) != iter) {
        iter->second.erase(target);
        // erase empty matrix element
        if (iter->second.empty())
            matrixWorld.erase(iter);
    }
}

void Level::push_light_object(Light* light) { _assoc_lightings.emplace_front(light); }

void Level::push_object(Object* obj) { _objects.insert(std::make_pair(obj, TimeEngine::time())); }

std::list<Transform*>* Level::get_hierarchy(Runtime::Transform* parent)
{
    if (!parent) {
        Application::fail("Argument is null");
    }
    return &parent->hierarchy;
}

void Level::intenal_bind_script(Behaviour* script)
{
    if (!_firstRunScripts)
        RoninMemory::alloc_self(_firstRunScripts);

    if (std::find_if(begin(*this->_firstRunScripts), end(*this->_firstRunScripts), std::bind2nd(std::equal_to<Behaviour*>(), script)) == end(*_firstRunScripts)) {
        if (_realtimeScripts && std::find_if(begin(*_realtimeScripts), end(*_realtimeScripts), std::bind2nd(std::equal_to<Behaviour*>(), script)) != end(*_realtimeScripts))
            return;

        _firstRunScripts->emplace_back(script);
    }
}

void Level::runtime_destructs()
{
    _destroyed = 0;
    // Destroy req objects
    if (_destructTasks && _destroy_delay_time < TimeEngine::time()) {
        float time = TimeEngine::time();
        std::map<float, std::set<Object*>>::iterator xiter = std::end(*_destructTasks), yiter = std::end(*_destructTasks);
        for (auto pair : *_destructTasks) {
            // The time for the destruction of the object has not yet come
            if (pair.first > time) {
                _destroy_delay_time = pair.first;
                break;
            }
            if (xiter == std::end(*_destructTasks))
                xiter = yiter = std::begin(*_destructTasks);

            // destroy
            for (auto target : pair.second) {
                if (target->exists()) {
                    // run destructor
                    destroy_immediate(target);
                    ++_destroyed;
                }
            }
            ++yiter;
        }

        if (xiter != yiter)
            _destructTasks->erase(xiter, yiter);

        if (_destructTasks->empty()) {
            RoninMemory::free(_destructTasks);
            _destructTasks = nullptr;
        }
    }
}

void Level::level_render_world(SDL_Renderer* renderer, ScoreWatcher* watcher)
{
    // set default color
    Gizmos::set_color(Color::white);

    if (watcher == nullptr)
        throw std::runtime_error("arg watcher is null");

    TimeEngine::begin_watch();

    if (_firstRunScripts) {
        if (!_realtimeScripts) {
            RoninMemory::alloc_self(_realtimeScripts);
        }

        for (Behaviour* exec : *_firstRunScripts) {
            if (exec->game_object()->m_active)
                exec->OnStart(); // go start (first draw)
            _realtimeScripts->emplace_back(exec);
        }
        RoninMemory::free(_firstRunScripts);
        _firstRunScripts = nullptr;
    }

    if (_realtimeScripts) {
        for (Behaviour* exec : *_realtimeScripts) {
            if (exec->game_object()->m_active)
                exec->OnUpdate();
        };
    }
    watcher->ms_wait_exec_scripts = TimeEngine::end_watch();
    // end watcher

    TimeEngine::begin_watch();
    // Render on main camera
    Camera* cam = Camera::main_camera(); // Draw level from active camera (main)
    if (cam) {
        Resolution res = Application::get_resolution();

        // FlushCache last result
        if (cam->targetClear)
            cam->renders.clear();

        // draw world in world size
        cam->render(renderer, { 0, 0, res.width, res.height }, main_object);
    }
    watcher->ms_wait_render_collect = TimeEngine::end_watch();

    // begin watcher
    TimeEngine::begin_watch();
    if (Camera::main_camera()) {
        if (_realtimeScripts) {
            for (auto exec : *_realtimeScripts) {
                if (exec->game_object()->m_active)
                    exec->OnGizmos();
            };
        }
        on_gizmo(); // Draw gizmos
    }
    watcher->ms_wait_render_gizmos = TimeEngine::end_watch();
    // end watcher

    TimeEngine::begin_watch();
    runtime_destructs();
    watcher->ms_wait_destructions = TimeEngine::end_watch();
}

void Level::level_render_ui(SDL_Renderer* renderer)
{
    // render UI
    ui->native_draw_render(renderer);
}

void Level::level_render_world_late(SDL_Renderer* renderer)
{
    on_late_update();
    if (_realtimeScripts) {
        for (auto exec : *_realtimeScripts) {
            if (exec->game_object()->m_active)
                exec->OnLateUpdate();
        }
    }
}
bool Level::is_hierarchy() { return this->main_object != nullptr; }

std::string& Level::name() { return this->m_name; }

UI::GUI* Level::get_gui() { return this->ui; }

void Level::request_unload() { this->request_unloading = true; }

int Level::get_destroyed_frames() { return _destroyed; }

std::list<GameObject*> Level::get_all_gameobjects()
{
    std::list<GameObject*> all_gobjects;
    GameObject* next = main_object;

    while (next) {
        for (Transform* e : *Level::get_hierarchy(next->transform())) {
            all_gobjects.emplace_front(e->game_object());
        }
        if (next == all_gobjects.front())
            next = nullptr;
        else
            next = all_gobjects.front();
    }
    return all_gobjects;
}

std::list<Component*> Level::get_all_components()
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

const bool Level::object_desctruction_cancel(Object* obj)
{
    if (!obj || !obj->exists()) {
        Application::fail_oom_kill();
        return false;
    }

    if (Level::self()->_destructTasks) {
        for (std::pair<float, std::set<Object*>> mapIter : *Level::self()->_destructTasks) {
            auto _set = mapIter.second;
            auto iter = _set.find(obj);
            if (iter != std::end(_set)) {
                _set.erase(iter);
                return true; // canceled
            }
        }
    }
    return false;
}

const int Level::object_destruction_cost(Object* obj)
{
    int x;
    if (!obj || !obj->exists()) {
        Application::fail_oom_kill();
        return false;
    }
    if (Level::self()->_destructTasks) {
        x = 0;
        for (std::pair<float, std::set<Object*>> mapIter : *Level::self()->_destructTasks) {
            auto _set = mapIter.second;
            if (_set.find(obj) != std::end(_set)) {
                return x;
            }
            ++x;
        }
    }

    return -1;
}

const bool Level::object_destruction_state(Object* obj) { return object_destruction_cost(obj) > ~0; }

const int Level::object_destruction_count()
{
    int x = 0;
    if (Level::self()->_destructTasks) {
        for (std::pair<float, std::set<Object*>> mapIter : *Level::self()->_destructTasks) {
            x += mapIter.second.size();
        }
    }
    return x;
}

void Level::on_awake() { }

void Level::on_start() { }

void Level::on_update() { }

void Level::on_late_update() { }

void Level::on_gizmo() { }

void Level::on_unloading() { }

Level* Level::self() { return switched_level; }
