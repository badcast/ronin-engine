#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    template <typename T>
    std::enable_if_t<std::is_base_of_v<Component, T>, Ref<T>> Component::AddComponent()
    {
        return _owner->AddComponent<T>();
    }

    template <typename T>
    std::enable_if_t<std::is_base_of_v<Component, T>, Ref<T>> Component::GetComponent()
    {
        return _owner->GetComponent<T>();
    }

    template <typename T>
    std::enable_if_t<std::is_base_of_v<Component, T>, bool> Component::RemoveComponent()
    {
        return _owner->RemoveComponent<T>();
    }

    template <typename T>
    std::enable_if_t<std::is_base_of_v<Component, T>, std::list<Ref<T>>> Component::GetComponents()
    {
        return _owner->GetComponents<T>();
    }
} // namespace RoninEngine::Runtime
