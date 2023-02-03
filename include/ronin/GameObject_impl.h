/*This is file part an GameObject*/
#pragma once

#include "GameObject.h"

namespace RoninEngine::Runtime
{
    template <typename T>
    std::enable_if_t<std::is_base_of<Component, T>::value, T*> GameObject::addComponent()
    {
        T* comp = GC::gc_push<T>();
        this->addComponent(static_cast<Component*>(comp));
        return comp;
    }

    template <typename T>
    T* GameObject::getComponent()
    {
        return GameObjectTypeHelper<T>::getType(this->m_components);
    }

    template <typename T>
    // std::enable_if<!std::is_same<RoninEngine::Runtime::Transform,T>::value, std::list<T*>>
    std::list<T*> GameObject::getComponents()
    {
        return GameObjectTypeHelper<T>::getTypes(this->m_components);
    }
}
