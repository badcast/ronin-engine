/*This is file part an GameObject*/

template <typename T>
std::enable_if_t<std::is_base_of<Component, T>::value, T*> Runtime::GC::addComponent(){
    T* comp = Runtime::GC::gc_push<T>();
    addComponent(reinterpret_cast<Component*>(comp));
    return comp;
}

template <typename T>
T* Runtime::GC::getComponent() {
    return AttribGetTypeHelper<T>::getType(this->m_components);
}

template <typename T>
// std::enable_if<!std::is_same<RoninEngine::Runtime::Transform,T>::value, std::list<T*>>
std::list<T*> Runtime::GC::getComponents() {
    return AttribGetTypeHelper<T>::getTypes(this->m_components);
}

