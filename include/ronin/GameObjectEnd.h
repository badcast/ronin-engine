#pragma once

#include "Behaviour.h"
#include "GameObject.h"

namespace RoninEngine
{
    namespace Runtime
    {
        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, Ref<T>> GameObject::AddComponent()
        {
            static_assert(!(std::is_same_v<T, Transform> || std::is_base_of_v<Transform, T>), "Transform component can't be assigned");

            // init component
            ComponentRef component { RoninMemory::alloc<T>() };
            RefRegister(StaticCast<RoninPointer>(component));
            AddComponent(component);

            if constexpr(std::is_base_of_v<Behaviour, T>)
            {
                int flags = 0;
#if 1
#ifdef __clang__ // it's work only Clang
#define CHECK_BASE_OVERRIDDEN(BASE, BINDER, METHOD) \
    if((&BASE::METHOD) != (&T::METHOD))             \
        flags |= BINDER;
#else // it's work on GCC
#define CHECK_BASE_OVERRIDDEN(BASE, BINDER, METHOD)                                               \
    if constexpr(reinterpret_cast<void *>(&BASE::METHOD) != reinterpret_cast<void *>(&T::METHOD)) \
        flags |= BINDER;
#endif

#else // if the not 0

#define CHECK_BASE_OVERRIDDEN(BASE, BINDER, METHOD) \
    if((&T::METHOD) != (&BASE::METHOD))             \
        flags |= BINDER;
#endif
                CHECK_BASE_OVERRIDDEN(Behaviour, Bind_Start, OnStart);
                CHECK_BASE_OVERRIDDEN(Behaviour, Bind_Update, OnUpdate);
                CHECK_BASE_OVERRIDDEN(Behaviour, Bind_LateUpdate, OnLateUpdate);
                CHECK_BASE_OVERRIDDEN(Behaviour, Bind_Gizmos, OnGizmos);
#undef CHECK_BASE_OVERRIDDEN

                bind_script(static_cast<BindType>(flags), StaticCast<Behaviour>(component));
            }

            return StaticCast<T>(component);
        } // namespace Runtime

        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, bool> GameObject::RemoveComponent()
        {
            static_assert(!(std::is_same_v<T, Transform> || std::is_base_of_v<Transform, T>), "Transform component can't remove, basic component type");
            Ref<T> target = GetComponent<T>();
            if(target.isNull())
                return false;
            return RemoveComponent(StaticCast<Component>(target));
        }

        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, std::list<Ref<T>>> GameObject::GetComponents()
        {
            Ref<T> cast {};
            std::list<Ref<T>> types;
            for(ComponentRef &comp : m_components)
            {
                if(!(cast = DynamicCast<T>(comp)).isNull())
                {
                    types.emplace_back(cast);
                }
            }
            return types;
        }

        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, std::list<Ref<T>>> GameObject::GetComponentsAnChilds()
        {
            std::list<Ref<T>> types {};
            std::list<GameObjectRef> stacks {};
            stacks.push_back(this->GetRef<GameObject>());
            while(!stacks.empty())
            {
                types.merge(stacks.front()->GetComponents<T>());
                for(TransformRef &t : stacks.front()->transform()->GetChilds())
                {
                    stacks.push_back(t->gameObject());
                }
                stacks.pop_front();
            }
            return types;
        }

        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, Ref<T>> GameObject::GetComponent()
        {
            if constexpr(std::is_same_v<T, Transform>)
            {
                return StaticCast<Transform>(m_components.front());
            }
            else
            {
                std::list<Ref<Component>>::iterator iter = std::find_if(std::begin(m_components), std::end(m_components), [](const ComponentRef &c) { return dynamic_cast<const T *>(c.get()) != nullptr; });
                if(iter != std::end(m_components))
                   return StaticCast<T>(*iter);
            }
            return nullptr;
        }
    } // namespace Runtime
} // namespace RoninEngine
