#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace Runtime
    {
        constexpr char _cloneStr[] = " (clone)";

        template <typename T>
        T *internal_factory_base(bool initInHierarchy, T *instance, const char *name)
        {
            if(instance == nullptr)
            {
                if(name == nullptr)
                    RoninMemory::alloc_self(instance);
                else
                    RoninMemory::alloc_self(instance, name);
            }
            else
            {
                T *cloneComponent;
                if constexpr(std::is_same<T, GameObject>::value)
                    cloneComponent = Instantiate(instance);
                else if constexpr(std::is_same<T, Transform>::value)
                {
                    RoninMemory::alloc_self(cloneComponent);
                }
                else if constexpr(std::is_same<T, SpriteRenderer>::value)
                {
                    RoninMemory::alloc_self(cloneComponent, *instance);
                }
                else if constexpr(std::is_same<T, Camera2D>::value)
                {
                    RoninMemory::alloc_self(cloneComponent, *instance);
                }
                else if constexpr(std::is_same<T, Terrain2D>::value)
                {
                    RoninMemory::alloc_self(cloneComponent, *instance);
                }
                else if constexpr(std::is_same<T, Behaviour>::value)
                {
                    RoninMemory::alloc_self(cloneComponent, *instance);
                }
                else
                {
                    cloneComponent = nullptr;
                }
                instance = cloneComponent;
            }

            if constexpr(std::is_same<T, GameObject>::value)
            {
                if(initInHierarchy)
                {
                    if(switched_world == nullptr)
                        throw std::runtime_error("self is null");

                    if(!switched_world->isHierarchy())
                        throw std::runtime_error(">mainObject is null");

                    auto mainObj = switched_world->irs->main_object;
                    auto root = mainObj->transform();
                    Transform *tr = ((GameObject *) instance)->transform();
                    root->childAdd(tr);
                }
            }

            return instance;
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

        void Destroy(GameObject *obj)
        {
            Destroy(obj, 0);
        }

        void Destroy(GameObject *obj, float t)
        {
            if(!obj || !switched_world || t < 0)
                throw std::bad_exception();

#define provider (switched_world->irs->runtimeCollectors)
            if(!provider)
            {
                RoninMemory::alloc_self(provider);
            }
            else
            {
                // get error an exists destructed @object
                for(std::pair<const float, std::set<GameObject *>> &mapIter : *provider)
                {
                    auto i = mapIter.second.find(obj);
                    if(i != std::end(mapIter.second))
                    {
                        mapIter.second.erase(i);
                        break;
                    }
                }
            }
            t += TimeEngine::time();

            // So, destroy childrens of the object
            provider->operator[](t).emplace(obj);

#undef provider
        }

        void harakiri_Component(Component *candidate)
        {
            union
            {
                Renderer *renderer;
                Transform *transform;
                Behaviour *script;
                Camera *camera;
            } _knife;
#define self (_knife.transform)
            if((self = dynamic_cast<Transform *>(candidate)))
            {
                // Is Unloding world
                if(!switched_world->irs->request_unloading)
                {
                    if(self->m_parent)
                    {
                        // Parent is off for self Transform
                        hierarchy_child_remove(self->m_parent, self);
                    }
                    //hierarchy_childs_move(self, switched_world->irs->main_object->transform());

                    // picking from matrix
                    Matrix::matrix_remove(self);
                }
            }
#undef self
#define self (_knife.script)
            else if((self = dynamic_cast<Behaviour *>(candidate)))
            {
                // Run Script Destroy
                self->OnDestroy();

                // Is Unloding world
                if(!switched_world->irs->request_unloading)
                {
                    // Run last script object
                    if(switched_world->irs->_firstRunScripts)
                        switched_world->irs->_firstRunScripts->remove(self);
                    if(switched_world->irs->runtimeScripts)
                        switched_world->irs->runtimeScripts->remove(self);
                }
            }
#undef self
#define self (_knife.camera)
            else if((self = dynamic_cast<Camera *>(candidate)))
            {
                // Free Camera Resources
                switched_world->irs->event_camera_changed(self, CameraEvent::CAM_DELETED);
            }
#undef self
#define self (_knife.renderer)
            else if((self = dynamic_cast<Renderer *>(candidate)))
            {
            }
#undef self

            // Is Unloding world
            if(!switched_world->irs->request_unloading)
            {
                // Extractor remove
                candidate->__ = nullptr;
                candidate->_type_ = nullptr;
            }

            // Free object
            RoninMemory::free(candidate);
            --switched_world->irs->objects;
        }

        void harakiri_GameObject(GameObject *obj)
        {
            if(!obj)
                throw ronin_null_error();

#ifndef NDEBUG
            if(strcmp(obj->_type_, "GameObject"))
            {
                throw ronin_type_error();
            }
#endif

            // destroy childs
            GameObject *target = obj;
            std::list<GameObject *> __stacks;
            while(target)
            {
                for(Transform *t : target->transform()->hierarchy)
                    __stacks.emplace_back(t->gameObject());

                for(Component *component : target->m_components)
                    harakiri_Component(component);

                if(!__stacks.empty())
                {
                    target = __stacks.front();
                    __stacks.pop_front();
                    harakiri_GameObject(target);
                }
                else
                    target = nullptr;
            }

            if(!switched_world->irs->request_unloading)
                // remove extractor flag
                obj->__ = nullptr;

            // START FREE OBJECT
            RoninMemory::free(obj);
            --switched_world->irs->objects;
        }

        bool object_instanced(const Object *obj)
        {
            if(!obj)
                return false;
            if(!switched_world)
                throw ronin_init_error();
            return obj->__ == switched_world;
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
                        yClone->transform()->setParent(existent);
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
            obj->transform()->setParent(parent, worldPositionStay);
            return obj;
        }

        // base class
        Object::Object() : Object(DESCRIBE_AS_MAIN_OFF(Object))
        {
        }

        Object::Object(const std::string &name) : m_name(name)
        {
            if(switched_world != nullptr)
            {
                DESCRIBE_AS_MAIN(Object);
                ::check_object(this);
                ++switched_world->irs->objects;
            }

            // set as instanced
            __ = switched_world;
        }

        bool Object::exists() const
        {
            return object_instanced(this);
        }

        std::string &Object::name(const std::string &newName)
        {
            return (m_name = newName);
        }

        std::string &Object::name()
        {
            return m_name;
        }

        const char *Object::getType() const
        {
            return _type_;
        }

        Object::operator bool()
        {
            return object_instanced(this);
        }
    } // namespace Runtime
} // namespace RoninEngine
