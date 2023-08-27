#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine
{
    namespace Runtime
    {
        constexpr char _cloneStr[] = " (clone)";

        template <typename T>
        T *internal_factory_base(bool initInHierarchy, T *clone, const char *name)
        {
            if(clone == nullptr)
            {
                if(name == nullptr)
                    RoninMemory::alloc_self(clone);
                else
                    RoninMemory::alloc_self(clone, name);
            }
            else
            {
                T *newTComponent;
                if constexpr(std::is_same<T, GameObject>::value)
                    newTComponent = Instantiate(clone);
                else if constexpr(std::is_same<T, Transform>::value)
                {
                    RoninMemory::alloc_self(newTComponent);
                }
                else if constexpr(std::is_same<T, SpriteRenderer>::value)
                {
                    RoninMemory::alloc_self(newTComponent, *clone);
                }
                else if constexpr(std::is_same<T, Camera2D>::value)
                {
                    RoninMemory::alloc_self(newTComponent, *clone);
                }
                else if constexpr(std::is_same<T, Terrain2D>::value)
                {
                    RoninMemory::alloc_self(newTComponent, *clone);
                }
                else if constexpr(std::is_same<T, Behaviour>::value)
                {
                    RoninMemory::alloc_self(newTComponent, *clone);
                }
                else
                {
                    newTComponent = nullptr;
                }
                clone = newTComponent;
            }

            if(clone == nullptr)
                RoninSimulator::Kill();

            if constexpr(std::is_same<T, GameObject>::value)
            {
                if(initInHierarchy)
                {
                    if(World::self() == nullptr)
                        throw std::runtime_error("self is null");

                    if(!World::self()->isHierarchy())
                        throw std::runtime_error(">mainObject is null");

                    auto mainObj = World::self()->internal_resources->main_object;
                    auto root = mainObj->transform();
                    Transform *tr = ((GameObject *) clone)->transform();
                    root->ChildAdd(tr);
                }
            }

            return clone;
        }

        void internal_destroy_object_dyn(Object *obj)
        {
            union
            {
                Renderer *renderer;
                Transform *transform;
                Behaviour *script;
                Camera *camera;
            } candidate;
            if((candidate.transform = dynamic_cast<Transform *>(obj)))
                internal_destroy_object<Transform>(candidate.transform);
            else if((candidate.script = dynamic_cast<Behaviour *>(obj)))
                internal_destroy_object<Behaviour>(candidate.script);
            else if((candidate.renderer = dynamic_cast<Renderer *>(obj)))
                internal_destroy_object<Renderer>(candidate.renderer);
            else if((candidate.camera = dynamic_cast<Camera *>(obj)))
                internal_destroy_object<Camera>(candidate.camera);
            else
                internal_destroy_object<Object>(obj);
        }

        template <typename T, typename std::enable_if<std::is_base_of<Object, T>::value, std::nullptr_t>::type>
        void internal_destroy_object(T *obj)
        {
            static_assert(std::is_base_of<Object, T>::value && !std::is_same<GameObject, T>::value, "obj var not based of Object");

            if constexpr(std::is_same<T, Transform>::value)
            {
                if(obj->parent())
                {
                    // something.*(HackGet<int Something::*>::ptr);
                    hierarchy_remove(obj->parent(), obj);
                }
                hierarchy_remove_all(obj);
                // picking from matrix
                Matrix::matrix_nature_pickup(obj);
            }
            else if constexpr(std::is_same<T, Behaviour>::value)
            {
                WorldResources *wrs = World::self()->internal_resources;
                if(wrs->_firstRunScripts)
                    wrs->_firstRunScripts->remove(obj);
                if(wrs->_realtimeScripts)
                    wrs->_realtimeScripts->remove(obj);
            }
            else if constexpr(std::is_same<T, Camera2D>::value)
            {
                World::self()->internal_resources->event_camera_changed(obj, CameraEvent::CAM_DELETED);

                // release main object
                RoninMemory::free(obj->camera_resources);
            }

            World::self()->internal_resources->world_objects.erase(obj);
            RoninMemory::free(obj);
        }

        Transform *create_empty_transform()
        {
            return internal_factory_base<Transform>(false, nullptr, nullptr);
        }

        GameObject *create_empty_gameobject()
        {
            return internal_factory_base<GameObject>(false, nullptr, nullptr);
        }

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

        RONIN_API GameObject *create_game_object()
        {
            return internal_factory_base<GameObject>(true, nullptr, nullptr);
        }

        RONIN_API GameObject *create_game_object(const std::string &name)
        {
            return internal_factory_base<GameObject>(true, nullptr, name.data());
        }

        void destroy(GameObject *obj)
        {
            destroy(obj, 0);
        }

        void destroy(GameObject *obj, float t)
        {
            if(!obj || !World::self() || t < 0)
                throw std::bad_exception();

            auto provider = World::self()->internal_resources->_destructTasks;

            if(!provider)
            {
                provider = World::self()->internal_resources->_destructTasks =
                    RoninMemory::alloc<std::remove_pointer<decltype(World::self()->internal_resources->_destructTasks)>::type>();
            }
            else
            {
                // get error an exists destructed @object
                for(std::pair<const float, std::set<GameObject *>> &mapIter : *provider)
                {
                    auto &_set = mapIter.second;
                    auto i = _set.find(obj);
                    if(i != std::end(_set))
                    {
                        _set.erase(i);

                        if(t == 0)
                        {
                            // destroy now
                            destroy_immediate(obj);
                            return;
                        }
                    }
                }
            }
            t += TimeEngine::time();
            provider->operator[](t).emplace(obj);
        }

        void destroy_immediate(GameObject *obj)
        {
            if(!obj)
                throw std::runtime_error("Object is null");

            // FIXME: destroy other types from gameobject->m_components (delete a list)
            // FIXME: Replace Recursive method to stack linear
            // Recursive method
            for(Component *component : obj->m_components)
            {
                internal_destroy_object_dyn(component);
            }

            obj->m_components.clear();

            World::self()->internal_resources->world_objects.erase(obj);

            // START FREE OBJECT
            RoninMemory::free(obj);
        }

        bool instanced(Object *obj)
        {
            if(!obj)
                return false;
            if(!World::self())
                throw std::bad_exception();
            auto iter = World::self()->internal_resources->world_objects.find(obj);
            return iter != end(World::self()->internal_resources->world_objects);
        }

        GameObject *Instantiate(GameObject *obj)
        {
            GameObject *clone;

            clone = create_game_object((obj->m_name.find(_cloneStr) == std::string::npos ? obj->m_name + _cloneStr : obj->m_name));
            for(auto iter = begin(obj->m_components); iter != end(obj->m_components); ++iter)
            {
                Component *replacement = *iter;
                if(Transform *t = dynamic_cast<Transform *>(replacement))
                {
                    Transform *existent = clone->transform();
                    existent->_angle_ = t->_angle_;
                    existent->position(t->position());
                    // BUG: Hierarchy change is bug, fix now;
                    //  Clone childs recursive
                    for(Transform *y : t->hierarchy)
                    {
                        GameObject *yClone = Instantiate(y->gameObject());
                        yClone->transform()->set_parent(existent);
                        yClone->m_name = t->gameObject()->m_name; // put " (clone)" name
                        yClone->m_name.shrink_to_fit();
                    }
                    // skip transform existent component
                    continue;
                }
                else if(dynamic_cast<SpriteRenderer *>(replacement))
                {
                    replacement = internal_factory_base<SpriteRenderer>(false, reinterpret_cast<SpriteRenderer *>(replacement), nullptr);
                }
                else if(dynamic_cast<Camera2D *>(replacement))
                {
                    replacement = internal_factory_base<Camera2D>(false, reinterpret_cast<Camera2D *>(replacement), nullptr);
                    //} else if (dynamic_cast<Behaviour*>(replacement)) {

                    /// replacement = internal_factory_base<Behaviour>(false, reinterpret_cast<Behaviour*>(replacement),
                    /// nullptr);
                }
                else
                {
                    static_assert(true, "Undefined type");
                    continue;
                }

                replacement->_owner = nullptr;
                clone->AddComponent(replacement);
            }

            return clone;
        }
        GameObject *Instantiate(GameObject *obj, Vec2 position, float angle)
        {
            obj = Instantiate(obj);
            obj->transform()->position(position);
            obj->transform()->angle(angle);
            return obj;
        }
        GameObject *Instantiate(GameObject *obj, Vec2 position, Transform *parent, bool worldPositionStay)
        {
            obj = Instantiate(obj);
            obj->transform()->position(position);
            obj->transform()->set_parent(parent, worldPositionStay);
            return obj;
        }

        // base class

        Object::Object() : Object(DESCRIBE_AS_MAIN_OFF(Object))

        {
        }

        Object::Object(const std::string &name) : m_name(name)
        {
            DESCRIBE_AS_MAIN(Object);
            ::check_object(this);
            if(World::self() == nullptr)
                throw std::bad_exception();

            id = World::self()->internal_resources->_level_ids_++;
            World::self()->push_object(this);
        }

        const bool Object::exists()
        {
            return (this->operator bool());
        }

        std::string &Object::name(const std::string &newName)
        {
            return (m_name = newName);
        }

        const std::string &Object::name()
        {
            return m_name;
        }

        uint32_t Object::get_id()
        {
            return id;
        }

        const char *Object::get_type() const
        {
            return _type_;
        }

        Object::operator bool()
        {
            return instanced(this);
        }
    } // namespace Runtime
} // namespace RoninEngine
