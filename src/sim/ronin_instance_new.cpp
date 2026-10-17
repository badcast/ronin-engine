#include "ronin.h"
#include "ronin_matrix.h"
#include "Spotlight.h"
#include "AudioSource.h"
#include "MoveController2D.h"

using namespace RoninEngine::Exception;

namespace RoninEngine
{
    namespace Runtime
    {
        template <typename T>
        Ref<T> instance_new(bool initInHierarchy, T *currentInstance, const char *name)
        {
            static_assert(std::is_base_of_v<RoninPointer, T>, "T is not an derived RoninPointer");

            if(currentInstance == nullptr)
            {
                if(name == nullptr)
                    RoninMemory::alloc_self(currentInstance);
                else
                    RoninMemory::alloc_self(currentInstance, name);
            }
            else
            {
                T *instanceNew;
                if constexpr(std::is_same_v<T, GameObject>)
                {
                    throw std::exception();
                }
                else if constexpr(std::is_same_v<T, Transform>)
                {
                    RoninMemory::alloc_self(instanceNew);
                }
                else if constexpr(std::is_same_v<T, SpriteRenderer>)
                {
                    RoninMemory::alloc_self(instanceNew, *currentInstance);
                }
                else if constexpr(std::is_same_v<T, Camera2D>)
                {
                    RoninMemory::alloc_self(instanceNew, *currentInstance);
                }
                else if constexpr(std::is_same_v<T, Terrain2D>)
                {
                    RoninMemory::alloc_self(instanceNew, *currentInstance);
                }
                else if constexpr(std::is_same_v<T, SimpleLight>)
                {
                    RoninMemory::alloc_self(instanceNew, *currentInstance);
                }
                else if constexpr(std::is_same_v<T, DirectLight>)
                {
                    RoninMemory::alloc_self(instanceNew, *currentInstance);
                }
                else if constexpr(std::is_same_v<T, Spotlight>)
                {
                    RoninMemory::alloc_self(instanceNew, *currentInstance);
                }
                else if constexpr(std::is_same_v<T, Behaviour>)
                {
                    RoninMemory::alloc_self(instanceNew, *currentInstance);
                }
                else
                {
                    instanceNew = nullptr;
                }
                currentInstance = instanceNew;
            }

            Ref<T> result { currentInstance };

            if constexpr(std::is_same_v<T, GameObject>)
            {
                currentInstance->m_components.push_back(StaticCast<Component>(instance_new<Transform>(false, nullptr, nullptr)));
                currentInstance->m_components.front()->_owner = result;
                Matrix::matrix_update(result->transform().ptr_);

                if(initInHierarchy)
                {
                    if(currentWorld == nullptr)
                        throw std::runtime_error("self is null");

                    if(!currentWorld->isHierarchy())
                        throw std::runtime_error("mainObject is null");

                    currentWorld->irs->mainObject->transform()->childAdd(currentInstance->transform());
                }
            }
            else if constexpr(std::is_same_v<T, Transform>)
            {
            }

            return result;
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
            GameObjectRef clone;
            clone = create_game_object((obj->m_name.find(_cloneStr) == std::string::npos ? obj->m_name + _cloneStr : obj->m_name));
            clone->m_layer = obj->m_layer;
            clone->m_zOrder = obj->m_zOrder;
            clone->m_tag = obj->m_tag;
            clone->m_active = obj->m_active;

            for(ComponentRef comp : obj->m_components)
            {
                if(comp.isNull())
                    continue;

                TransformRef refTransform = DynamicCast<Transform>(comp);
                if(refTransform)
                {
                    TransformRef clonedTransform = clone->transform();
                    clonedTransform->_angle_ = refTransform->_angle_;
                    clonedTransform->position(refTransform->_position);
                    for(TransformRef &y : refTransform->hierarchy)
                    {
                        if(y.isNull() || y->gameObject().isNull())
                            continue;
                        // Clone children recursively
                        GameObjectRef yClone = __instantiate(y->gameObject());
                        yClone->transform()->setParent(clonedTransform, true);
                        yClone->m_name = y->gameObject()->m_name;
                        yClone->m_name.shrink_to_fit();
                    }
                    continue;
                }

                if(CollisionRef cloneFrom = DynamicCast<Collision>(comp))
                {
                    CollisionRef cloneIt = clone->AddComponent<Collision>();
                    cloneIt->targetLayer = cloneFrom->targetLayer;
                    cloneIt->collideSize = cloneFrom->collideSize;
                    cloneIt->_enable = cloneFrom->_enable;
                    cloneIt->onCollision = cloneFrom->onCollision;
                    continue;
                }

                ComponentRef clonedComp = nullptr;

                if(auto sr = dynamic_cast<SpriteRenderer*>(comp.ptr_))
                {
                    clonedComp = StaticCast<Component>(instance_new<SpriteRenderer>(false, sr, nullptr));
                }
                else if(auto cam = dynamic_cast<Camera2D*>(comp.ptr_))
                {
                    clonedComp = StaticCast<Component>(instance_new<Camera2D>(false, cam, nullptr));
                }
                else if(auto ter = dynamic_cast<Terrain2D*>(comp.ptr_))
                {
                    clonedComp = StaticCast<Component>(instance_new<Terrain2D>(false, ter, nullptr));
                }
                else if(auto sl = dynamic_cast<SimpleLight*>(comp.ptr_))
                {
                    clonedComp = StaticCast<Component>(instance_new<SimpleLight>(false, sl, nullptr));
                }
                else if(auto dl = dynamic_cast<DirectLight*>(comp.ptr_))
                {
                    clonedComp = StaticCast<Component>(instance_new<DirectLight>(false, dl, nullptr));
                }
                else if(auto sp = dynamic_cast<Spotlight*>(comp.ptr_))
                {
                    clonedComp = StaticCast<Component>(instance_new<Spotlight>(false, sp, nullptr));
                }
                else if(auto as = dynamic_cast<AudioSource*>(comp.ptr_))
                {
                    AudioSourceRef toAudio = clone->AddComponent<AudioSource>();
                    toAudio->setClip(as->getClip());
                    toAudio->setVolume(as->getVolume());
                    toAudio->_enable = as->_enable;
                    continue;
                }
                else if(auto mc = dynamic_cast<MoveController2D*>(comp.ptr_))
                {
                    MoveController2DRef toMC = clone->AddComponent<MoveController2D>();
                    toMC->speed = mc->speed;
                    toMC->_enable = mc->_enable;
                    continue;
                }
                else
                {
                    continue;
                }

                if(clonedComp)
                {
                    clonedComp->_enable = comp->_enable;
                    clonedComp->_owner = nullptr;
                    clone->AddComponent(clonedComp);
                }
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
            if(!obj.isNull())
            {
                obj->transform()->position(position);
                obj->transform()->angle(angle);
            }
            return obj;
        }

        GameObjectRef Instantiate(GameObjectRef obj, Vec2 position, TransformRef parent, bool worldPositionStay)
        {
            obj = Instantiate(obj);
            if(!obj.isNull())
            {
                obj->transform()->position(position);
                obj->transform()->setParent(parent, worldPositionStay);
            }
            return obj;
        }

    } // namespace Runtime
} // namespace RoninEngine
