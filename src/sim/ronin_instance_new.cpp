#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace Runtime
    {
        template <typename T>
        Bushido<T> instance_new(bool initInHierarchy, T *instance, const char *name)
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
                    if(currentWorld == nullptr)
                        throw std::runtime_error("self is null");

                    if(!currentWorld->isHierarchy())
                        throw std::runtime_error("mainObject is null");

                    auto mainObj = currentWorld->irs->mainObject;
                    auto root = mainObj->transform();
                    Transform *tr = ((GameObject *) instance)->transform();
                    root->childAdd(tr);
                }
            }

            return instance;
        }

        bool object_instanced(const Object *obj)
        {
            if(obj == nullptr || currentWorld == nullptr)
                return false;
            return true;
        }

        TransformRef create_empty_transform()
        {
            return instance_new<Transform>(false, nullptr, nullptr);
        }

        GameObjectRef create_empty_gameobject()
        {
            return instance_new<GameObject>(false, nullptr, nullptr);
        }

        RONIN_API GameObjectRef create_game_object()
        {
            return instance_new<GameObject>(true, nullptr, nullptr);
        }

        RONIN_API GameObjectRef create_game_object(const std::string &name)
        {
            return instance_new<GameObject>(true, nullptr, name.data());
        }

        constexpr char _cloneStr[] = " (clone)";

        GameObjectRef __instantiate(GameObjectRef &obj)
        {

            // TODO: Use Reflectable Class <Prototype> class reference for instance a new object from method <Clone>

            GameObjectRef clone;

            clone = create_game_object((obj->m_name.find(_cloneStr) == std::string::npos ? obj->m_name + _cloneStr : obj->m_name));

            for(auto iter = begin(obj->m_components); iter != end(obj->m_components); ++iter)
            {
                ComponentRef &replacement = *iter;
                if(TransformRef &refTransform = replacement.DynamicCast<Transform>())
                {
                    TransformRef clonedTransform = clone->transform();
                    clonedTransform->_angle_ = refTransform->_angle_;
                    clonedTransform->position(refTransform->_position);
                    for(TransformRef &y : refTransform->hierarchy)
                    {
                        // Clone childs recursive
                        GameObjectRef yClone = __instantiate(y->gameObject());
                        yClone->transform()->setParent(clonedTransform, false);
                        yClone->m_name = refTransform->gameObject()->m_name;
                        yClone->m_name.shrink_to_fit();
                    }
                    continue;
                }
                else if(CollisionRef cloneIt = replacement.DynamicCast<Collision>()))
                {
                    CollisionRef cloneFrom = cloneIt;
                    cloneIt = clone->AddComponent<Collision>();
                    cloneIt->targetLayer = cloneFrom->targetLayer;
                    cloneIt->collideSize = cloneFrom->collideSize;
                    cloneIt->_enable = cloneFrom->_enable;
                    continue;
                }
                else if(!replacement.DynamicCast<SpriteRenderer>().isNull())
                {
                    replacement = instance_new<SpriteRenderer>(false, reinterpret_cast<SpriteRenderer *>(replacement), nullptr);
                }
                else if(!replacement.DynamicCast<Camera2D>().isNull())
                {
                    replacement = instance_new<Camera2D>(false, replacement.StaticCast<Camera2D(), nullptr);

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

        GameObjectRef Instantiate(GameObjectRef obj)
        {
            if(obj.isNull())
                return nullptr;

            obj = __instantiate(obj);

            return obj;
        }

        GameObjectRef Instantiate(GameObjectRef obj, Vec2 position, float angle)
        {
            obj = Instantiate(obj);
            if(obj != nullptr)
            {
                obj->transform()->position(position);
                obj->transform()->angle(angle);
            }
            return obj;
        }

        GameObjectRef Instantiate(GameObjectRef obj, Vec2 position, TransformRef parent, bool worldPositionStay)
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
