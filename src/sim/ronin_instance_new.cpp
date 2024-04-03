#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace Runtime
    {
        template <typename T>
        T *instance_new(bool initInHierarchy, T *instance, const char *name)
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

        bool object_instanced(const Object *obj)
        {
            if(obj == nullptr || switched_world == nullptr)
                return false;
            return true;
        }

        Transform *create_empty_transform()
        {
            return instance_new<Transform>(false, nullptr, nullptr);
        }

        GameObject *create_empty_gameobject()
        {
            return instance_new<GameObject>(false, nullptr, nullptr);
        }

        RONIN_API GameObject *create_game_object()
        {
            return instance_new<GameObject>(true, nullptr, nullptr);
        }

        RONIN_API GameObject *create_game_object(const std::string &name)
        {
            return instance_new<GameObject>(true, nullptr, name.data());
        }

        constexpr char _cloneStr[] = " (clone)";

        GameObject *__instantiate(GameObject *obj, std::vector<std::pair<Transform *, Transform *>> &postApplyProp)
        {
            GameObject *clone;

            clone = create_game_object((obj->m_name.find(_cloneStr) == std::string::npos ? obj->m_name + _cloneStr : obj->m_name));

            for(auto iter = begin(obj->m_components); iter != end(obj->m_components); ++iter)
            {
                Component *replacement = *iter;
                if(Transform *refTransform = dynamic_cast<Transform *>(replacement))
                {
                    Transform *clonedTransform = clone->transform();
                    clonedTransform->_angle_ = refTransform->_angle_;
                    //  parent->position(t->_position);
                    postApplyProp.emplace_back(clonedTransform, refTransform);
                    //  Clone childs recursive
                    for(Transform *y : refTransform->hierarchy)
                    {
                        GameObject *yClone = __instantiate(y->gameObject(), postApplyProp);
                        yClone->transform()->setParent(clonedTransform, false);
                        yClone->m_name = refTransform->gameObject()->m_name; // put " (clone)" name
                        yClone->m_name.shrink_to_fit();
                    }
                    // skip transform existent component
                    continue;
                }
                else if(dynamic_cast<SpriteRenderer *>(replacement))
                {
                    replacement = instance_new<SpriteRenderer>(false, reinterpret_cast<SpriteRenderer *>(replacement), nullptr);
                }
                else if(dynamic_cast<Camera2D *>(replacement))
                {
                    replacement = instance_new<Camera2D>(false, reinterpret_cast<Camera2D *>(replacement), nullptr);
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

        GameObject *Instantiate(GameObject *obj)
        {
            std::vector<std::pair<Transform *, Transform *>> postApplyProp;

            if(obj == nullptr)
                return nullptr;

            obj = __instantiate(obj, postApplyProp);
            for(auto iter = std::rbegin(postApplyProp); iter != std::rend(postApplyProp); ++iter)
            {
                iter->first->position(iter->second->_position);
                iter->first->angle(iter->second->_angle_);
            }
            return obj;
        }

        GameObject *Instantiate(GameObject *obj, Vec2 position, float angle)
        {
            obj = Instantiate(obj);
            if(obj != nullptr)
            {
                obj->transform()->position(position);
                obj->transform()->angle(angle);
            }
            return obj;
        }

        GameObject *Instantiate(GameObject *obj, Vec2 position, Transform *parent, bool worldPositionStay)
        {
            obj = Instantiate(obj);
            if(obj != nullptr)
            {
                obj->transform()->position(position);
                obj->transform()->setParent(parent, worldPositionStay);
            }
            return obj;
        }

    } // namespace Runtime
} // namespace RoninEngine
