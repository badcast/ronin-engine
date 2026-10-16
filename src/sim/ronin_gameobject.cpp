#include "ronin.h"
#include "ronin_matrix.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    extern TransformRef create_empty_transform();

    GameObject::GameObject() : GameObject(DESCRIBE_AS_MAIN_OFF(GameObject))
    {
    }

    GameObject::GameObject(const std::string &name) : Object(DESCRIBE_AS_ONLY_NAME(GameObject)), m_active(true), m_layer(0), m_zOrder(0)
    {
        DESCRIBE_AS_MAIN(GameObject);
    }

    bool GameObject::isActive()
    {
        return m_active;
    }

    bool GameObject::isActiveInHierarchy()
    {
        TransformRef p = transform();
        for(; p && p->_owner && p->_owner->m_active;)
            p = p->m_parent;
        return p.isNull();
    }

    void GameObject::SetActive(bool state)
    {
        if(m_active == state)
            return;
        m_active = state;
        transform()->parent_notify_active_state(GetRef<GameObject>());
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
        this->m_zOrder = value;
        std::list<GameObject*> __stack;
        __stack.emplace_back(this);
        while(__stack.size())
        {
            GameObject *front = __stack.front();
            __stack.pop_front();
            if(!front || !front->transform())
                continue;
            for(TransformRef &h : front->transform()->hierarchy)
            {
                if(h && h->_owner)
                {
                    if(orderBy == LinearAdd)
                        ++value;
                    h->_owner->m_zOrder = value;
                    __stack.emplace_back(h->_owner.ptr_);
                }
            }
        }
    }

    const bool GameObject::CancelDestroy()
    {
        if(!currentWorld)
            return false;
        return currentWorld->CancelObjectDestruction(this->GetRef<GameObject>());
    }

    const bool GameObject::isDestroying()
    {
        if(!currentWorld)
            return false;
        return currentWorld->StateObjectDestruction(this->GetRef<GameObject>());
    }

    void GameObject::SetActiveRecursivelly(bool state)
    {
        std::list<GameObject *> __stack;
        __stack.emplace_back(this);
        while(__stack.size())
        {
            GameObject *front = __stack.front();
            __stack.pop_front();
            if(!front || !front->transform())
                continue;
            for(TransformRef &h : front->transform()->hierarchy)
            {
                if(h && h->_owner)
                {
                    __stack.emplace_back(h->_owner.ptr_);
                }
            }
            front->SetActive(state);
        }
    }

    TransformRef GameObject::transform()
    {
        return StaticCast<Transform>(m_components.front());
    }

    SpriteRendererRef GameObject::spriteRenderer()
    {
        return GetComponent<SpriteRenderer>();
    }

    Camera2DRef GameObject::camera2D()
    {
        return GetComponent<Camera2D>();
    }

    Terrain2DRef GameObject::terrain2D()
    {
        return GetComponent<Terrain2D>();
    }

    void GameObject::setTag(const std::string &tag)
    {
        m_tag = tag;
    }

    std::string GameObject::getTag() const
    {
        return m_tag;
    }

    bool GameObject::isPrefab()
    {
        return transform()->m_parent.isNull() && (!currentWorld || !currentWorld->irs || currentWorld->irs->mainObject.ptr_ != this);
    }

    void GameObject::Destroy()
    {
        Runtime::Destroy(GetRef<GameObject>());
    }

    void GameObject::Destroy(float t)
    {
        Runtime::Destroy(GetRef<GameObject>(), t);
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
            Camera* cam;
        } _utp;

        if(std::end(m_components) == std::find_if(begin(m_components), end(m_components), std::bind(std::equal_to<ComponentRef>(), std::placeholders::_1, component)))
        {
            component->_owner = GetRef<GameObject>();
            m_components.emplace_back(component);

            if(_utp.script = dynamic_cast<Behaviour *>(component.ptr_))
            {
                _utp.script->OnAwake();
            }
            else if(_utp.render = dynamic_cast<Renderer *>(component.ptr_))
            {
                if(transform())
                    Matrix::matrix_update(transform().ptr_);
            }
            else if(_utp.light = dynamic_cast<Light *>(component.ptr_))
            {
                if(transform())
                    Matrix::matrix_update(transform().ptr_);
            }
            else if(_utp.cam = dynamic_cast<Camera*>(component.ptr_))
            {
                _utp.cam->Focus();
            }
        }

        return component;
    }

    bool GameObject::RemoveComponent(ComponentRef component)
    {
        if(component.isNull())
            return false;

        if(component->_owner.ptr_ != this)
            return false;

        if(!DynamicCast<Transform>(component).isNull())
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
