#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace Runtime
    {
        template <typename T>
        Ref<T> instance_new(bool initInHierarchy, T *instance, const char *name)
        {
            static_assert(std::is_base_of_v<RoninPointer, T>, "T is not an derived RoninPointer");

            if(instance == nullptr)
            {
                if(name == nullptr)
                    RoninMemory::alloc_self(instance);
                else
                    RoninMemory::alloc_self(instance, name);
            }
            else
            {
                T *cloning;
                if constexpr(std::is_same_v<T, GameObject>)
                {
                    throw std::exception();
                    //cloning = Instantiate(instance->ref<T>()).ptr_;
                }
                else if constexpr(std::is_same_v<T, Transform>)
                {
                    RoninMemory::alloc_self(cloning);
                }
                else if constexpr(std::is_same_v<T, SpriteRenderer>)
                {
                    RoninMemory::alloc_self(cloning, *instance);
                }
                else if constexpr(std::is_same_v<T, Camera2D>)
                {
                    RoninMemory::alloc_self(cloning, *instance);
                }
                else if constexpr(std::is_same_v<T, Terrain2D>)
                {
                    RoninMemory::alloc_self(cloning, *instance);
                }
                else if constexpr(std::is_same_v<T, Behaviour>)
                {
                    RoninMemory::alloc_self(cloning, *instance);
                }
                else
                {
                    cloning = nullptr;
                }
                instance = cloning;
            }

            Ref<T> result { instance };
            if(instance)
            {
                currentWorld->irs->refPointers[static_cast<RoninPointer*>(instance)] = std::move(StaticCast<RoninPointer>(result));
            }

            if constexpr(std::is_same_v<T, GameObject>)
            {
                instance->m_components.front()->_owner = result;
                Matrix::matrix_update(result->transform().get(), Matrix::matrix_get_key(Vec2::infinity));

                if(initInHierarchy)
                {
                    if(currentWorld == nullptr)
                        throw std::runtime_error("self is null");

                    if(!currentWorld->isHierarchy())
                        throw std::runtime_error("mainObject is null");

                    currentWorld->irs->mainObject->transform()->childAdd(instance->transform());
                }
            }

            return result;
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

        GameObjectRef create_game_object()
        {
            return instance_new<GameObject>(true, nullptr, nullptr);
        }

        GameObjectRef create_game_object(const std::string &name)
        {
            return instance_new<GameObject>(true, nullptr, name.data());
        }

        constexpr char _cloneStr[] = " (clone)";

        GameObjectRef __instantiate(GameObjectRef obj)
        {

            // TODO: Use Reflectable Class <Prototype> class reference for instance a new object from method <Clone>

            GameObjectRef clone;

            clone = create_game_object((obj->m_name.find(_cloneStr) == std::string::npos ? obj->m_name + _cloneStr : obj->m_name));

            for(auto iter = begin(obj->m_components); iter != end(obj->m_components); ++iter)
            {
                ComponentRef &replacement = *iter;
                TransformRef refTransform = DynamicCast<Transform>(replacement);
                if(refTransform)
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
                else if(CollisionRef cloneIt = DynamicCast<Collision>(replacement))
                {
                    CollisionRef cloneFrom = cloneIt;
                    cloneIt = clone->AddComponent<Collision>();
                    cloneIt->targetLayer = cloneFrom->targetLayer;
                    cloneIt->collideSize = cloneFrom->collideSize;
                    cloneIt->_enable = cloneFrom->_enable;
                    continue;
                }
                else if(!DynamicCast<SpriteRenderer>(replacement).isNull())
                {
                    replacement = StaticCast<Component>(instance_new<SpriteRenderer>(false, reinterpret_cast<SpriteRenderer *>(replacement.ptr_), nullptr));
                }
                else if(!DynamicCast<Camera2D>(replacement).isNull())
                {
                    replacement = StaticCast<Component>(instance_new<Camera2D>(false, reinterpret_cast<Camera2D*>(replacement.ptr_), nullptr));

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
