#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    extern Transform *create_empty_transform();

    // NOTE: Решение многих проблем и времени, объявление
    template RONIN_API MoveController2D *GameObject::AddComponent<MoveController2D>();
    template RONIN_API SpriteRenderer *GameObject::AddComponent<SpriteRenderer>();
    template RONIN_API Camera2D *GameObject::AddComponent<Camera2D>();
    template RONIN_API Spotlight *GameObject::AddComponent<Spotlight>();
    template RONIN_API Terrain2D *GameObject::AddComponent<Terrain2D>();
    //----------------
    template RONIN_API MoveController2D *GameObject::GetComponent<MoveController2D>();
    template RONIN_API SpriteRenderer *GameObject::GetComponent<SpriteRenderer>();
    template RONIN_API Camera2D *GameObject::GetComponent<Camera2D>();
    template RONIN_API Spotlight *GameObject::GetComponent<Spotlight>();
    template RONIN_API Terrain2D *GameObject::GetComponent<Terrain2D>();
    template RONIN_API Transform *GameObject::GetComponent<Transform>();
    // Remove Component
    template RONIN_API bool GameObject::RemoveComponent<MoveController2D>();
    template RONIN_API bool GameObject::RemoveComponent<SpriteRenderer>();
    template RONIN_API bool GameObject::RemoveComponent<Camera2D>();
    template RONIN_API bool GameObject::RemoveComponent<Spotlight>();
    template RONIN_API bool GameObject::RemoveComponent<Terrain2D>();

    GameObject::GameObject() : GameObject(DESCRIBE_AS_MAIN_OFF(GameObject))
    {
    }

    GameObject::GameObject(const std::string &name) : Object(DESCRIBE_AS_ONLY_NAME(GameObject))
    {
        DESCRIBE_AS_MAIN(GameObject);
        m_components.push_back(create_empty_transform());
        m_components.front()->_owner = this;
        m_active = true;
    }

    GameObject::~GameObject()
    {
    }

    bool GameObject::isActive()
    {
        return m_active;
    }

    void GameObject::SetActive(bool state)
    {
        if(m_active == state)
            return;

        m_active = state;
        transform()->parent_notify_active_state(this);
    }

    const bool GameObject::CancelDestroy()
    {
        return World::self()->CancelObjectDestruction(this);
    }

    const bool GameObject::isDestroying()
    {
        return World::self()->StateObjectDestruction(this);
    }

    void GameObject::SetActiveRecursivelly(bool state)
    {
        std::list<GameObject *> __stack;
        __stack.emplace_back(this);
        while(__stack.size())
        {
            for(Transform *h : __stack.front()->transform()->hierarchy)
            {
                __stack.emplace_back(h->gameObject());
            }
            __stack.front()->SetActive(state);
            __stack.pop_front();
        }
    }

    Transform *GameObject::transform()
    {
        // NOTE: transform всегда первый объект из контейнера m_components
        return static_cast<Transform *>(m_components.front());
    }

    SpriteRenderer *GameObject::spriteRenderer()
    {
        return GetComponent<SpriteRenderer>();
    }

    Camera2D *GameObject::camera2D()
    {
        return GetComponent<Camera2D>();
    }

    Terrain2D *GameObject::terrain2D()
    {
        return GetComponent<Terrain2D>();
    }

    void GameObject::Destroy()
    {
        this->Destroy(0);
    }

    void GameObject::Destroy(float t)
    {
        Runtime::destroy(this, t);
    }

    Component *GameObject::AddComponent(Component *component)
    {
        if(!component)
            throw ronin_null_error();
        if(component->_owner)
            throw ronin_conflict_component_error();

        if(end(m_components) == std::find_if(begin(m_components), end(m_components), std::bind2nd(std::equal_to<Component *>(), component)))
        {
            this->m_components.emplace_back(component);

            component->_owner = this;

            if(Behaviour *script = dynamic_cast<Behaviour *>(component))
            {
                internal_bind_script(script);
                script->OnAwake();
            }
            else if(Renderer *rend = dynamic_cast<Renderer *>(component))
            {
                // TODO: awake on renderer
            }
            else if(Light *light = dynamic_cast<Light *>(component))
            {
                // TODO: awake on Light
            }
        }

        return component;
    }

    bool GameObject::RemoveComponent(Component *component)
    {
        if(component == nullptr)
            throw ronin_null_error();

        if(component->_owner != this)
        {
            throw ronin_conflict_component_error();
        }

        if(dynamic_cast<Transform *>(component) != nullptr)
            return false;
        // BUG: Destroying object canceled with signal SIGILL
        component->_owner = nullptr; // remove owner
        m_components.remove(component);
        Runtime::internal_destroy_object_dyn(component);
    }
} // namespace RoninEngine::Runtime
