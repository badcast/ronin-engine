#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

// TODO: optimze MatrixSelection USE TRANSFORM::LAYER component for selection (as INDEX)

Level* selfLevel;

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
    , globalID(0)
    , _destroy_delay_time(0)
    , m_isUnload(false)
    , m_name(name)
{
    if (selfLevel != nullptr) {
        static_assert(true, "pCurrentScene replaced by new");
    }
    selfLevel = this;
    RoninMemory::alloc_self(ui, this);
}
Level::~Level()
{
    GameObject* target = main_object; // first
    Transform* tr;
    std::list<GameObject*> stack;

    if (selfLevel == this) {
        static_assert(true, "pCurrentScene set to null");
        selfLevel = nullptr;
    }

    if (_firstRunScripts) {
        RoninMemory::free(_firstRunScripts);
        _firstRunScripts = nullptr;
    }
    if (_realtimeScripts) {
        RoninMemory::free(_realtimeScripts);
        _realtimeScripts = nullptr;
    }
    if (_destructTasks) {
        RoninMemory::free(_destructTasks);
        _destructTasks = nullptr;
    }

    /*
        // free objects
        while (target) {
            tr = target->transform();
            for (auto c : tr->hierarchy) {
                stack.emplace_back(c->gameObject());
            }

            GC::gc_unload(target);

            if (!stack.empty()) {
                target = stack.front();
                stack.pop_front();
            } else
                target = nullptr;
        }
    */
    this->_objects.clear();

    RoninMemory::free(ui);
}

// NOTE: Check game hierarchy
std::list<Transform*> Level::matrixCheckDamaged()
{
    std::list<Transform*> damaged;

    for (auto x = begin(selfLevel->matrixWorld); x != end(selfLevel->matrixWorld); ++x) {
        for (auto y = begin(x->second); y != end(x->second); ++y) {
            if (Vec2::RoundToInt((*y)->position()) != x->first) {
                damaged.emplace_back(*y);
            }
        }
    }
    return damaged;
}

int Level::matrix_restore()
{
    auto damaged = matrixCheckDamaged();
    return matrix_restore(damaged);
}

// try restore damaged matrix element's
int Level::matrix_restore(const std::list<Runtime::Transform*>& damaged_content)
{
    int restored = 0;

    for (Runtime::Transform* dam : damaged_content) {
        Vec2Int find = Vec2::RoundToInt(dam->p);
        auto vertList = selfLevel->matrixWorld.find(find);

        if (vertList == std::end(selfLevel->matrixWorld))
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

void Level::matrix_nature(Transform* target, Vec2Int lastPoint) { matrix_nature(target, Vec2::RoundToInt(target->position()), lastPoint); }

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
    auto iter = matrixWorld.find(Vec2::RoundToInt(target->position()));

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

void Level::destructs()
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

void Level::level_render_world(SDL_Renderer* renderer)
{
    if (_firstRunScripts) {
        if (!_realtimeScripts) {
            RoninMemory::alloc_self(_realtimeScripts);
        }

        for (auto x : *_firstRunScripts) {
            x->OnStart(); // go start (first draw)
            this->_realtimeScripts->emplace_back(x);
        }
        RoninMemory::free(_firstRunScripts);
        _firstRunScripts = nullptr;
    }

    if (_realtimeScripts) {
        for (auto n : *_realtimeScripts) {
            n->OnUpdate();
        };
    }

    // Render on main camera
    Camera* cam = Camera::main_camera(); // Draw level
    if (cam) {
        Resolution res = Application::get_resolution();
        // FlushCache last result
        if (cam->targetClear)
            cam->renders.clear();
        // Рисуем в соотношение окна...
        cam->render(renderer, { 0, 0, res.width, res.height }, main_object);
    }
    if (_realtimeScripts) {
        for (auto n : *_realtimeScripts) {
            n->OnGizmos();
        };
    }
    destructs();
}

void Level::level_render_ui(SDL_Renderer* renderer)
{
    // render UI
    ui->native_draw_render(renderer);
}

void Level::level_render_world_late(SDL_Renderer* renderer)
{
    if (_realtimeScripts) {
        for (auto n : *_realtimeScripts) {
            n->OnLateUpdate();
        }
    }
}
bool Level::is_hierarchy() { return this->main_object != nullptr; }

std::string& Level::name() { return this->m_name; }

UI::GUI* Level::gui() { return this->ui; }

void Level::unload() { this->m_isUnload = true; }

int Level::get_destroyed_frames() { return _destroyed; }

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

void Level::awake() { }

void Level::start() { }

void Level::update() { }

void Level::lateUpdate() { }

void Level::onDrawGizmos() { }

void Level::onUnloading() { }

Level* Level::self() { return selfLevel; }
