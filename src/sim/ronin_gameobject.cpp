#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    extern TransformRef create_empty_transform();

    // Add Component
  // template RONIN_API MoveController2DRef GameObject::AddComponent<MoveController2D>();
  // template RONIN_API SpriteRendererRef GameObject::AddComponent<SpriteRenderer>();
  // template RONIN_API Camera2DRef GameObject::AddComponent<Camera2D>();
  // template RONIN_API SpotlightRef GameObject::AddComponent<Spotlight>();
  // template RONIN_API Terrain2DRef GameObject::AddComponent<Terrain2D>();
  // template RONIN_API ParticleSystemRef GameObject::AddComponent<ParticleSystem>();
  // template RONIN_API CollisionRef GameObject::AddComponent<Collision>();
  // // Get Component
  // template RONIN_API MoveController2DRef GameObject::GetComponent<MoveController2D>() const;
  // template RONIN_API SpriteRendererRef GameObject::GetComponent<SpriteRenderer>() const;
  // template RONIN_API Camera2DRef GameObject::GetComponent<Camera2D>() const;
  // template RONIN_API SpotlightRef GameObject::GetComponent<Spotlight>() const;
  // template RONIN_API Terrain2DRef GameObject::GetComponent<Terrain2D>() const;
  // template RONIN_API TransformRef GameObject::GetComponent<Transform>() const;
  // template RONIN_API ParticleSystemRef GameObject::GetComponent<ParticleSystem>() const;
  // template RONIN_API CollisionRef GameObject::GetComponent<Collision>() const;
  // // Remove Component
  // template RONIN_API bool GameObject::RemoveComponent<MoveController2D>();
  // template RONIN_API bool GameObject::RemoveComponent<SpriteRenderer>();
  // template RONIN_API bool GameObject::RemoveComponent<Camera2D>();
  // template RONIN_API bool GameObject::RemoveComponent<Spotlight>();
  // template RONIN_API bool GameObject::RemoveComponent<Terrain2D>();
  // template RONIN_API bool GameObject::RemoveComponent<ParticleSystem>();
  // template RONIN_API bool GameObject::RemoveComponent<Collision>();

    GameObject::GameObject() : GameObject(DESCRIBE_AS_MAIN_OFF(GameObject))
    {
    }

    GameObject::GameObject(const std::string &name) : Object(DESCRIBE_AS_ONLY_NAME(GameObject)), m_active(true), m_layer(0), m_zOrder(0)
    {
        DESCRIBE_AS_MAIN(GameObject);
        m_components.push_back(create_empty_transform());
        m_components.front()->_owner = this;
        // create matrix-slot for transform object
        Matrix::matrix_update(transform().get(), Matrix::matrix_get_key(Vec2::infinity));
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

    void GameObject::SetZOrderAll(int value, ZOrderBy orderBy)
    {
        std::list<GameObject *> __stack;
        __stack.emplace_back(this);
        while(__stack.size())
        {
            switch(orderBy)
            {
                case LinearAdd:
                    ++value;
                    break;
                case Inherit:;
                default:;
            }

            for(Transform *h : __stack.front()->transform()->hierarchy)
            {
                __stack.emplace_back(h->gameObject());
                h->_owner->m_zOrder = value;
            }
            __stack.pop_front();
        }
    }

    const bool GameObject::CancelDestroy()
    {
        return currentWorld->CancelObjectDestruction(this);
    }

    const bool GameObject::isDestroying()
    {
        return currentWorld->StateObjectDestruction(this);
    }

    void GameObject::SetActiveRecursivelly(bool state)
    {
        std::list<GameObject *> __stack;
        __stack.emplace_back(this);
        while(__stack.size())
        {
            for(TransformRef &h : __stack.front()->transform()->hierarchy)
            {
                __stack.emplace_back(h->gameObject());
            }
            __stack.front()->SetActive(state);
            __stack.pop_front();
        }
    }

    TransformRef GameObject::transform() const
    {
        // NOTE: Transform всегда первый объект из контейнера m_components
        return m_components.front().StaticCast<Transform>();
    }

    SpriteRendererRef GameObject::spriteRenderer() const
    {
        return GetComponent<SpriteRenderer>();
    }

    Camera2DRef GameObject::camera2D() const
    {
        return GetComponent<Camera2D>();
    }

    Terrain2DRef GameObject::terrain2D() const
    {
        return GetComponent<Terrain2D>();
    }

    bool GameObject::isPrefab()
    {
        return transform()->m_parent == nullptr && currentWorld->irs->mainObject != this;
    }

    void GameObject::Destroy()
    {
        Runtime::Destroy(this, 0);
    }

    void GameObject::Destroy(float t)
    {
        Runtime::Destroy(this, t);
    }

    ComponentRef GameObject::AddComponent(ComponentRef component)
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

    bool GameObject::RemoveComponent(ComponentRef component)
    {
        if(component.isNull())
            return false;

        if(component->_owner != this)
            return false;

        if(!component.DynamicCast<Transform>().isNull())
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
