#include "framework.h"

namespace RoninEngine {
//TODO: optimze MatrixSelection USE TRANSFORM::LAYER component for selection (as INDEX)

Level *selfLevel;

Level::Level() : Level("Untitled") {}
Level::Level(const std::string &name) {
    if (selfLevel != nullptr) {
        static_assert("pCurrentScene replaced by new");
    }
    main_object = nullptr;
    selfLevel = this;
    _firstRunScripts = nullptr;
    _realtimeScripts = nullptr;
    _destructions = nullptr;
    globalID = 0;
    m_isUnload = false;
    m_name = name;
    GC::gc_alloc_lval(ui, this);
}
Level::~Level() {
    GameObject *target = main_object;  // first
    Transform *tr;
    std::list<GameObject *> stack;

    if (selfLevel == this) {
        static_assert("pCurrentScene set to null");
        selfLevel = nullptr;
    }

    // BUG: _destruction members is not free
    if (_firstRunScripts) {
        GC::gc_unalloc(_firstRunScripts);
    }
    if (_realtimeScripts) {
        GC::gc_unalloc(_realtimeScripts);
    }
    if (_destructions) {
        GC::gc_unalloc(_destructions);
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

    GC::gc_unalloc(ui);
}

// NOTE: Check game hierarchy
std::list<Transform *> Level::matrixCheckDamaged() {
    std::list<Transform *> damaged;

    for (auto x = begin(selfLevel->matrixWorld); x != end(selfLevel->matrixWorld); ++x) {
        for (auto y = begin(x->second); y != end(x->second); ++y) {
            if (Vec2::RoundToInt((*y)->position()) != x->first) {
                damaged.emplace_back(*y);
            }
        }
    }
    return damaged;
}

int Level::matrixRestore() {
    auto damaged = matrixCheckDamaged();
    return matrixRestore(damaged);
}

//try restore damaged matrix element's
int Level::matrixRestore(const std::list<Runtime::Transform *> &damaged_content) {
    int restored = 0;

    for (Runtime::Transform *dam : damaged_content) {
        Vec2Int find = Vec2::RoundToInt(dam->p);
        auto vertList = selfLevel->matrixWorld.find(find);

        if (vertList == std::end(selfLevel->matrixWorld)) continue;

        if (vertList->first != find) static_assert(true, "Hash function is collision");

        auto fl = vertList->second.find(dam);

        if (fl == end(vertList->second)) {
            // restore
            vertList->second.emplace(dam);
            ++restored;
        }
    }

    return restored;
}

void Level::render_info(int *culled, int *fullobjects) {
    Camera *curCamera = Camera::mainCamera();

    if (!curCamera) return;

    if (culled) {
        (*culled) = selfLevel->_assoc_renderers.size() - curCamera->renders.size();
    }

    if (fullobjects) {
        (*fullobjects) = selfLevel->_assoc_renderers.size();
    }
}

void Level::matrix_nature(Transform *target, Vec2Int lastPoint) {
    matrix_nature(target, Vec2::RoundToInt(target->position()), lastPoint);
}

//THIS is matrix get from world space
void Level::matrix_nature(Runtime::Transform *target, const RoninEngine::Runtime::Vec2Int &newPoint, const RoninEngine::Runtime::Vec2Int &lastPoint) {
    if (newPoint == lastPoint) return;

    // 1. delete last point source
    auto iter = matrixWorld.find(lastPoint);

    if (std::end(matrixWorld) != iter) {
        iter->second.erase(target);

        // 2. erase empty matrix element
        if (iter->second.empty()) matrixWorld.erase(iter);
    }

    // 3. add new point source
    matrixWorld[newPoint].insert(target);
}

void Level::CC_Render_Push(Renderer *rend) { _assoc_renderers.emplace_front(rend); }

void Level::CC_Light_Push(Light *light) { _assoc_lightings.emplace_front(light); }

void Level::ObjectPush(Object *obj) { _objects.insert(std::make_pair(obj, Time::time())); }

std::vector<RoninEngine::Runtime::Transform *> *RoninEngine::Level::getHierarchy(Runtime::Transform *parent) {
    if (!parent) {
        Application::fail("Argument is null");
    }
    return &parent->hierarchy;
}

void Level::PinScript(Behaviour *behav) {
    if (!_firstRunScripts) GC::gc_alloc_lval(_firstRunScripts);
    if (std::find_if(begin(*this->_firstRunScripts), end(*this->_firstRunScripts),
                     [behav](Behaviour *ref) { return behav == ref; }) == end(*_firstRunScripts)) {
        if (_realtimeScripts && std::find_if(begin(*this->_realtimeScripts), end(*this->_realtimeScripts),
                                             [behav](Behaviour *ref) { return behav == ref; }) != end(*_realtimeScripts))
            return;

        _firstRunScripts->emplace_back(behav);
    }
}

void Level::RenderLevel(SDL_Renderer *renderer) {
    if (_firstRunScripts) {
        if (!_realtimeScripts) {
            GC::gc_alloc_lval(_realtimeScripts);
            if (_realtimeScripts == nullptr) throw std::bad_alloc();
        }

        for (auto x : *_firstRunScripts) {
            x->OnStart();  // go start (first draw)
            this->_realtimeScripts->emplace_back(x);
        }
        GC::gc_unalloc(_firstRunScripts);
        _firstRunScripts = nullptr;
    }

    if (_realtimeScripts) {
        for (auto n : *_realtimeScripts) {
            n->OnUpdate();
        };
    }

    // Render on main camera
    Camera *cam = Camera::mainCamera();  // Draw level
    if (cam) {
        Resolution res = Application::getResolution();
        // FlushCache last result
        if (cam->targetClear) cam->renders.clear();
        // Рисуем в соотношение окна...
        cam->render(renderer, {0, 0, res.width, res.height}, main_object);
    }

    if (_realtimeScripts) {
        for (auto n : *_realtimeScripts) {
            n->OnGizmos();
        };
    }

    // Destroy req objects
    if (_destructions) {
        if (_destructions->begin() == _destructions->end()) {
            GC::gc_unalloc(_destructions);
        } else {
            _destructions->remove_if([](std::pair<Object *, float> &p) {
                if (p.second <= Time::time()) {
                    Destroy_Immediate(p.first);
                    return true;
                }
                return false;
            });
        }
    }
}

void Level::RenderUI(SDL_Renderer *renderer) {
    // render UI
    ui->Do_Present(renderer);
}

void Level::RenderSceneLate(SDL_Renderer *renderer) {
    if (_realtimeScripts) {
        for (auto n : *_realtimeScripts) {
            n->OnLateUpdate();
        }
    }
}
bool Level::is_hierarchy() { return this->main_object != nullptr; }

std::string &Level::name() { return this->m_name; }

UI::GUI *Level::Get_GUI() { return this->ui; }

void Level::Unload() { this->m_isUnload = true; }

void Level::awake() {}

void Level::start() {}

void Level::update() {}

void Level::lateUpdate() {}

void Level::onDrawGizmos() {}

void Level::onUnloading() {}

Level *Level::self() { return selfLevel; }
}  // namespace RoninEngine
