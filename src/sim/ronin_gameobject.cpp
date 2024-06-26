#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    extern Transform *create_empty_transform();

    // Add Component
    template RONIN_API MoveController2D *GameObject::AddComponent<MoveController2D>();
    template RONIN_API SpriteRenderer *GameObject::AddComponent<SpriteRenderer>();
    template RONIN_API Camera2D *GameObject::AddComponent<Camera2D>();
    template RONIN_API Spotlight *GameObject::AddComponent<Spotlight>();
    template RONIN_API Terrain2D *GameObject::AddComponent<Terrain2D>();
    template RONIN_API ParticleSystem *GameObject::AddComponent<ParticleSystem>();
    template RONIN_API Collision *GameObject::AddComponent<Collision>();
    // Get Component
    template RONIN_API MoveController2D *GameObject::GetComponent<MoveController2D>() const;
    template RONIN_API SpriteRenderer *GameObject::GetComponent<SpriteRenderer>() const;
    template RONIN_API Camera2D *GameObject::GetComponent<Camera2D>() const;
    template RONIN_API Spotlight *GameObject::GetComponent<Spotlight>() const;
    template RONIN_API Terrain2D *GameObject::GetComponent<Terrain2D>() const;
    template RONIN_API Transform *GameObject::GetComponent<Transform>() const;
    template RONIN_API ParticleSystem *GameObject::GetComponent<ParticleSystem>() const;
    template RONIN_API Collision *GameObject::GetComponent<Collision>() const;
    // Remove Component
    template RONIN_API bool GameObject::RemoveComponent<MoveController2D>();
    template RONIN_API bool GameObject::RemoveComponent<SpriteRenderer>();
    template RONIN_API bool GameObject::RemoveComponent<Camera2D>();
    template RONIN_API bool GameObject::RemoveComponent<Spotlight>();
    template RONIN_API bool GameObject::RemoveComponent<Terrain2D>();
    template RONIN_API bool GameObject::RemoveComponent<ParticleSystem>();
    template RONIN_API bool GameObject::RemoveComponent<Collision>();

    GameObject::GameObject() : GameObject(DESCRIBE_AS_MAIN_OFF(GameObject))
    {
    }

    GameObject::GameObject(const std::string &name) : Object(DESCRIBE_AS_ONLY_NAME(GameObject)), m_active(true), m_layer(0), m_zOrder(0)
    {
        DESCRIBE_AS_MAIN(GameObject);
        m_components.push_back(create_empty_transform());
        m_components.front()->_owner = this;
        // create matrix-slot for transform object
        Matrix::matrix_update(transform(), Matrix::matrix_get_key(Vec2::infinity));
    }

    bool GameObject::isActive()
    {
        return m_active;
    }

    bool GameObject::isActiveInHierarchy()
    {
        Transform *p;

        p = transform();
        for(; p && p->_owner->m_active;)
            p = p->m_parent;

        return p == nullptr;
    }

    void GameObject::SetActive(bool state)
    {
        if(m_active == state)
            return;

        m_active = state;
        transform()->parent_notify_active_state(this);
    }

    void GameObject::SetLayer(int layer)
    {
        transform()->layer(layer);
    }

    int GameObject::GetLayer() const
    {
        return m_layer;
    }

    int GameObject::GetZOrder() const
    {
        return m_zOrder;
    }

    void GameObject::SetZOrder(int value)
    {
        m_zOrder = value;
    }

    const bool GameObject::CancelDestroy()
    {
        return _world->CancelObjectDestruction(this);
    }

    const bool GameObject::isDestroying()
    {
        return _world->StateObjectDestruction(this);
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

    Transform *GameObject::transform() const
    {
        // NOTE: Transform всегда первый объект из контейнера m_components
        return static_cast<Transform *>(m_components.front());
        // return GetComponent<Transform>();
    }

    SpriteRenderer *GameObject::spriteRenderer() const
    {
        return GetComponent<SpriteRenderer>();
    }

    Camera2D *GameObject::camera2D() const
    {
        return GetComponent<Camera2D>();
    }

    Terrain2D *GameObject::terrain2D() const
    {
        return GetComponent<Terrain2D>();
    }

    bool GameObject::isPrefab()
    {
        return transform()->m_parent == nullptr && _world->irs->mainObject != this;
    }

    void GameObject::Destroy()
    {
        Runtime::Destroy(this, 0);
    }

    void GameObject::Destroy(float t)
    {
        Runtime::Destroy(this, t);
    }

    Component *GameObject::AddComponent(Component *component)
    {
        if(!component)
            throw ronin_null_error();
        if(component->_owner)
            throw ronin_conflict_component_error();

        union
        {
            Behaviour *script;
            Renderer *render;
            Light *light;
        } _utp;

        if(end(m_components) == std::find_if(begin(m_components), end(m_components), std::bind(std::equal_to<Component *>(), std::placeholders::_1, component)))
        {
            component->_owner = this;
            m_components.emplace_back(component);

            if(_utp.script = dynamic_cast<Behaviour *>(component))
            {
                _utp.script->OnAwake();
            }
            else if(_utp.render = dynamic_cast<Renderer *>(component))
            {
                // TODO: awake on renderer
            }
            else if(_utp.light = dynamic_cast<Light *>(component))
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
            throw ronin_conflict_component_error();

        if(dynamic_cast<Transform *>(component) != nullptr)
            return false;

        for(Component::Event &event : component->ev_destroy)
            event(component);

        Runtime::sepuku_Component(component);
        m_components.remove(component);
        return true;
    }

    void GameObject::AddOnDestroy(Event callback)
    {
        ev_destroy.emplace_back(callback);
    }

    void GameObject::ClearOnDestroy()
    {
        ev_destroy.clear();
    }
} // namespace RoninEngine::Runtime
