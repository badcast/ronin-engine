#pragma once
#include "ronin.h"

namespace RoninEngine {
namespace Runtime {
//[attrib class]
template <typename T>
class AttribGetTypeHelper {
   public:
    static T* getType(const std::list<Component*>& container) {
        auto iter =
            std::find_if(++begin(container), end(container), [](Component* c) { return dynamic_cast<T*>(c) != nullptr; });

        if (iter != end(container)) return reinterpret_cast<T*>(*iter);

        return nullptr;
    }

    static std::list<T*> getTypes(const std::list<Component*>& container) {
        std::list<T*> types;
        T* cast;
        for (auto iter = std::begin(container); iter != std::end(container); ++iter) {
            if ((cast = dynamic_cast<T*>(*iter))) {
                types.emplace_back(cast);
            }
        }

        return types;
    }
};

class SHARK GameObject final : public Object {
    friend class Camera2D;
    friend class Component;
    friend GameObject* Instantiate(GameObject* obj);
    friend GameObject* Instantiate(GameObject* obj, Vec2 position, float angle);
    friend GameObject* Instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionState);
    friend void Destroy(Object* obj);
    friend void Destroy(Object* obj, float t);
    friend void Destroy_Immediate(Object* obj);

   private:
    std::list<Component*> m_components;
    // TODO: Реализовать компонент m_active, для объектов GameObject
    bool m_active;

   public:
    GameObject();

    GameObject(const std::string&);

    GameObject(const GameObject&) = delete;

    ~GameObject();

    bool isActive();
    bool isActiveHierarchy();
    void setActive(bool state);
    void setActiveRecursivelly(bool state);

    Transform* transform();

    Component* addComponent(Component* component);

    SpriteRenderer* spriteRenderer() { return getComponent<SpriteRenderer>(); }

    Camera2D* camera2D() { return getComponent<Camera2D>(); }

    Terrain2D* terraind2D() { return getComponent<Terrain2D>(); }

    template <typename T>
    std::enable_if_t<std::is_base_of<Component, T>::value, T*> addComponent(){
        T* comp = Runtime::GC::gc_push<T>();
        addComponent(reinterpret_cast<Component*>(comp));
        return comp;
    }

    template <typename T>
    T* getComponent() {
        return AttribGetTypeHelper<T>::getType(this->m_components);
    }

    template <typename T>
    // std::enable_if<!std::is_same<RoninEngine::Runtime::Transform,T>::value, std::list<T*>>
    std::list<T*> getComponents() {
        return AttribGetTypeHelper<T>::getTypes(this->m_components);
    }

#include "GameObject_impl.h"

    //Transform* getComponent();
};

}  // namespace Runtime
}  // namespace RoninEngine
