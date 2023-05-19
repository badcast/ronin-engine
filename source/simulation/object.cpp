#include "ronin.h"

namespace RoninEngine
{
    namespace Runtime
    {

        constexpr char _cloneStr[] = " (clone)";

        template <typename T>
        T* internal_factory_base(bool initInHierarchy, T* clone, const char* name)
        {
            if (clone == nullptr) {
                if (name == nullptr)
                    RoninMemory::alloc_self(clone);
                else
                    RoninMemory::alloc_self(clone, std::string(name));
            } else {
                T* newTComponent;
                if constexpr (std::is_same<T, GameObject>::value)
                    newTComponent = instantiate(clone);
                else if constexpr (std::is_same<T, Transform>::value) {
                    // TODO: Required cloning for Transform
                    RoninMemory::alloc_self(newTComponent);
                } else if constexpr (std::is_same<T, SpriteRenderer>::value) {
                    RoninMemory::alloc_self(newTComponent, *clone);
                } else if constexpr (std::is_same<T, Camera2D>::value) {
                    RoninMemory::alloc_self(newTComponent, *clone);
                } else if constexpr (std::is_same<T, Terrain2D>::value) {
                    RoninMemory::alloc_self(newTComponent, *clone);
                } else {
                    static_assert(true, "undefined type");
                    newTComponent = nullptr;
                }
                clone = newTComponent;
            }

            if (clone == nullptr)
                Application::fail_oom_kill();

            if constexpr (std::is_same<T, GameObject>::value) {
                if (initInHierarchy) {
                    if (Level::self() == nullptr)
                        throw std::runtime_error("pCurrentScene is null");

                    if (!Level::self()->is_hierarchy())
                        throw std::runtime_error("pCurrentScene->mainObject is null");

                    auto mainObj = Level::self()->main_object;
                    auto root = mainObj->transform();
                    Transform* tr = ((GameObject*)clone)->transform();
                    root->child_append(tr);
                }
            }

            return clone;
        }

        Transform* create_empty_transform() { return internal_factory_base<Transform>(false, nullptr, nullptr); }

        GameObject* create_empty_gameobject() { return internal_factory_base<GameObject>(false, nullptr, nullptr); }

        /*NOTE: WoW: Здесь профиксина 6 месячная проблема
        template GameObject* CreateObject<GameObject>();
        template Transform* CreateObject<Transform>();
        template Player* CreateObject<Player>();
        template Camera2D* CreateObject<Camera2D>();
        template Spotlight* CreateObject<Spotlight>();
        template SpriteRenderer* CreateObject<SpriteRenderer>();

        template GameObject* CreateObject<GameObject>(const string&);
        template Transform* CreateObject<Transform>(const string&);
        template Player* CreateObject<Player>(const string&);
        template Camera2D* CreateObject<Camera2D>(const string&);
        template Spotlight* CreateObject<Spotlight>(const string&);
        template SpriteRenderer* CreateObject<SpriteRenderer>(const string&);
        -------------------------------------------------------------------*/

        RONIN_API GameObject* create_game_object() { return internal_factory_base<GameObject>(true, nullptr, nullptr); }

        RONIN_API GameObject* create_game_object(const std::string& name) { return internal_factory_base<GameObject>(true, nullptr, name.data()); }

        void destroy(Object* obj) { destroy(obj, 0); }

        void destroy(Object* obj, float t)
        {
            if (!obj || !Level::self() || t < 0)
                throw std::bad_exception();

            auto& destructors = Level::self()->_destructTasks;

            if (!destructors) {
                destructors = Level::self()->_destructTasks = RoninMemory::alloc<std::remove_pointer<decltype(Level::self()->_destructTasks)>::type>();
            } else {
                // get error an exists destructed @object
                for (std::pair<float, std::set<Object*>> mapIter : *destructors) {
                    auto _set = mapIter.second;

                    if (_set.find(obj) != std::end(_set)) {
                        throw std::runtime_error("Object an prev destruct state");
                    }
                }
            }
            t += TimeEngine::time();
            destructors->operator[](t).emplace(obj);
        }

        void destroy_immediate(Object* obj)
        {
            if (!obj)
                throw std::runtime_error("Object is null");

            GameObject* gObj;
            if ((gObj = dynamic_cast<GameObject*>(obj)) != nullptr) {
                if (Level::self()->_firstRunScripts) {
                    Level::self()->_firstRunScripts->remove_if([gObj](Behaviour* x) {
                        auto iter = find_if(std::begin(gObj->m_components), std::end(gObj->m_components), [x](const Component* c) { return (Component*)x == c; });
                        return iter != end(gObj->m_components);
                    });
                } else if (Level::self()->_realtimeScripts) {
                    Level::self()->_realtimeScripts->remove_if([gObj](Behaviour* x) {
                        auto iter = find_if(std::begin(gObj->m_components), std::end(gObj->m_components), [x](Component* c) { return (Component*)x == c; });

                        return iter != end(gObj->m_components);
                    });
                } else // destroy other types
                {
                    // FIXME: destroy other types from gameobject->m_components (delete a list)
                    // Recursive method
                    for (Component* component : gObj->m_components) {
                        destroy_immediate(component);
                    }
                }
                // other types
            } else {
                Renderer* r;
                Transform* t;
                if ((t = dynamic_cast<Transform*>(obj))) {
                    // picking from matrix
                    if (t->parent()) {
                        Transform::hierarchy_remove(t->parent(), t);
                    }
                    Transform::hierarchy_removeAll(t);
                    Level::self()->matrix_nature_pickup(t);
                }
            }

            Level::self()->_objects.erase(obj);

            // START FREE OBJECT
            RoninMemory::free(obj);
        }

        bool instanced(Object* obj)
        {
            if (!obj)
                return false;
            if (!Level::self())
                throw std::bad_exception();
            auto iter = Level::self()->_objects.find(obj);
            return iter != end(Level::self()->_objects);
        }

        GameObject* instantiate(GameObject* obj)
        {
            GameObject* clone;

            clone = create_game_object((obj->m_name.find(_cloneStr) == std::string::npos ? obj->m_name + _cloneStr : obj->m_name));
            for (auto iter = begin(obj->m_components); iter != end(obj->m_components); ++iter) {
                Component* replacement = *iter;
                if (Transform* t = dynamic_cast<Transform*>(replacement)) {
                    Transform* existent = clone->transform();
                    existent->_angle = t->_angle;
                    existent->position(t->position());
                    // BUG: Hierarchy cnage is bug, fix now;
                    //  Clone childs recursive
                    for (Transform* y : t->hierarchy) {
                        GameObject* yClone = instantiate(y->gameObject());
                        yClone->transform()->set_parent(existent);
                        yClone->m_name = t->gameObject()->m_name; // put " (clone)" name
                        yClone->m_name.shrink_to_fit();
                    }
                    // skip transform existent component
                    continue;
                } else if (dynamic_cast<SpriteRenderer*>(replacement)) {
                    replacement = internal_factory_base<SpriteRenderer>(false, reinterpret_cast<SpriteRenderer*>(replacement), nullptr);
                } else if (dynamic_cast<Camera2D*>(replacement)) {
                    replacement = internal_factory_base<Camera2D>(false, reinterpret_cast<Camera2D*>(replacement), nullptr);
                } else {
                    static_assert(true, "Undefined type");
                    continue;
                }

                replacement->_owner = nullptr;
                clone->add_component(replacement);
            }

            return clone;
        }
        GameObject* instantiate(GameObject* obj, Vec2 position, float angle)
        {
            obj = instantiate(obj);
            obj->transform()->position(position);
            obj->transform()->angle(angle);
            return obj;
        }
        GameObject* instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionStay)
        {
            obj = instantiate(obj);
            obj->transform()->position(position);
            obj->transform()->set_parent(parent, worldPositionStay);
            return obj;
        }

        // base class

        Object::Object()
            : Object(DESCRIBE_TYPE(Object, this, nullptr))

        {
        }

        Object::Object(const std::string& name)
            : m_name(name)
            , t(-1)
        {
            DESCRIBE_TYPE(Object, this, &t);
            ::check_object(this);
            if (Level::self() == nullptr)
                throw std::bad_exception();

            id = Level::self()->globalID++;
            Level::self()->push_object(this);
        }

        const bool Object::exists() { return (this->operator bool()); }

        void Object::destroy() { RoninEngine::Runtime::destroy(this); }

        const bool Object::destroy_cancel() { return Level::self()->object_desctruction_cancel(this); }

        const bool Object::is_destruction() { return Level::self()->object_destruction_state(this); }

        std::string& Object::name(const std::string& newName) { return (m_name = newName); }

        const std::string& Object::name() { return m_name; }

        const int Object::get_type() const { return t; }

        Object::operator bool() { return instanced(this); }
    } // namespace Runtime
} // namespace RoninEngine
