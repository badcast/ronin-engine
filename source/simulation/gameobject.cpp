#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

using Level = RoninEngine::Level;

// NOTE: Решение многих проблем и времени, объявление
template RoninEngine::Runtime::Player* GameObject::addComponent<Player>();
template RoninEngine::Runtime::SpriteRenderer* GameObject::addComponent<SpriteRenderer>();
template RoninEngine::Runtime::Camera2D* GameObject::addComponent<Camera2D>();
template RoninEngine::Runtime::Spotlight* GameObject::addComponent<Spotlight>();
template RoninEngine::Runtime::Terrain2D* GameObject::addComponent<Terrain2D>();

GameObject::GameObject()
    : GameObject(typeid(*this).name())
{
}

GameObject::GameObject(const std::string& name)
    : Object(name)
{
    m_components.push_back(create_empty_transform());
    m_components.front()->_owner = this;
    m_active = true;
    Level::self()->_gameObjects.emplace_back(this);
}

GameObject::~GameObject()
{
    return;
    for (auto x : m_components) {
        GC::gc_unload(x);
    }
}

bool GameObject::isActive() { return m_active; }

void GameObject::setActive(bool state)
{
    if (m_active == state)
        return;

    m_active = state;
    transform()->parent_notify_activeState(this);
}

void GameObject::setActiveRecursivelly(bool state) { this->setActive(false); }

Transform* GameObject::transform()
{
    // NOTE: transform всегда первый объект из контейнера m_components
    return static_cast<Transform*>(m_components.front());
}

Component* GameObject::addComponent(Component* component)
{
    if (!component)
        throw std::exception();

    if (end(m_components) == std::find_if(begin(m_components), end(m_components), std::bind2nd(std::equal_to<Component*>(),component))){
        this->m_components.emplace_back(component);

        if (component->_owner)
            throw std::bad_exception();

        component->_owner = this;

        if (Behaviour* script = dynamic_cast<Behaviour*>(component)) {
            Level::self()->intenal_bind_script(script);
            script->OnAwake();
        } else if (Renderer* rend = dynamic_cast<Renderer*>(component)) {
            Level::self()->CC_Render_Push(rend);
        } else if (Light* light = dynamic_cast<Light*>(component)) {
            Level::self()->CC_Light_Push(light);
        }
    }

    return component;
}

// Transform* GameObject::getComponent() {
//     return transform();
// }
