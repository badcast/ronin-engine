#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime
{
    extern Transform* create_empty_transform();

    // NOTE: Решение многих проблем и времени, объявление
    template RONIN_API MoveController2D* GameObject::add_component<MoveController2D>();
    template RONIN_API SpriteRenderer* GameObject::add_component<SpriteRenderer>();
    template RONIN_API Camera2D* GameObject::add_component<Camera2D>();
    template RONIN_API Spotlight* GameObject::add_component<Spotlight>();
    template RONIN_API Terrain2D* GameObject::add_component<Terrain2D>();
    //----------------
    template RONIN_API MoveController2D* GameObject::get_component<MoveController2D>();
    template RONIN_API SpriteRenderer* GameObject::get_component<SpriteRenderer>();
    template RONIN_API Camera2D* GameObject::get_component<Camera2D>();
    template RONIN_API Spotlight* GameObject::get_component<Spotlight>();
    template RONIN_API Terrain2D* GameObject::get_component<Terrain2D>();
    template RONIN_API Transform* GameObject::get_component<Transform>();

    GameObject::GameObject()
        : GameObject(DESCRIBE_AS_MAIN_OFF(GameObject))
    {
    }

    GameObject::GameObject(const std::string& name)
        : Object(DESCRIBE_AS_ONLY_NAME(GameObject))
    {
        DESCRIBE_AS_MAIN(GameObject);
        m_components.push_back(create_empty_transform());
        m_components.front()->_owner = this;
        m_active = true;
    }

    GameObject::~GameObject() { }

    bool GameObject::is_active() { return m_active; }

    void GameObject::set_active(bool state)
    {
        if (m_active == state)
            return;

        m_active = state;
        transform()->parent_notify_active_state(this);
    }

    // TODO: Recursivelly set active all hierarchy
    void GameObject::set_active_recursivelly(bool state) { this->set_active(false); }

    Transform* GameObject::transform()
    {
        // NOTE: transform всегда первый объект из контейнера m_components
        return static_cast<Transform*>(m_components.front());
    }

    template <typename T>
    T* GameObject::get_component()
    {
        return GameObjectTypeHelper<T>::get_type(this->m_components);
    }

    Component* GameObject::add_component(Component* component)
    {
        if (!component)
            throw std::exception();

        if (end(m_components) == std::find_if(begin(m_components), end(m_components), std::bind2nd(std::equal_to<Component*>(), component))) {
            this->m_components.emplace_back(component);

            if (component->_owner)
                throw std::runtime_error("Component has Owner GameObject");

            component->_owner = this;

            if (Behaviour* script = dynamic_cast<Behaviour*>(component)) {
                Level::self()->intenal_bind_script(script);
                script->OnAwake();
            } else if (Renderer* rend = dynamic_cast<Renderer*>(component)) {
                // awake on renderer
            } else if (Light* light = dynamic_cast<Light*>(component)) {
                Level::self()->push_light_object(light);
            }
        }

        return component;
    }

    // template Transform* GameObject::get_component() { return transform(); }
}
