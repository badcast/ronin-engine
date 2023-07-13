#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    extern Transform *create_empty_transform();

    // NOTE: Решение многих проблем и времени, объявление
    template RONIN_API MoveController2D *GameObject::add_component<MoveController2D>();
    template RONIN_API SpriteRenderer *GameObject::add_component<SpriteRenderer>();
    template RONIN_API Camera2D *GameObject::add_component<Camera2D>();
    template RONIN_API Spotlight *GameObject::add_component<Spotlight>();
    template RONIN_API Terrain2D *GameObject::add_component<Terrain2D>();
    //----------------
    template RONIN_API MoveController2D *GameObject::get_component<MoveController2D>();
    template RONIN_API SpriteRenderer *GameObject::get_component<SpriteRenderer>();
    template RONIN_API Camera2D *GameObject::get_component<Camera2D>();
    template RONIN_API Spotlight *GameObject::get_component<Spotlight>();
    template RONIN_API Terrain2D *GameObject::get_component<Terrain2D>();
    template RONIN_API Transform *GameObject::get_component<Transform>();
    // Remove Component
    template RONIN_API bool GameObject::remove_component<MoveController2D>();
    template RONIN_API bool GameObject::remove_component<SpriteRenderer>();
    template RONIN_API bool GameObject::remove_component<Camera2D>();
    template RONIN_API bool GameObject::remove_component<Spotlight>();
    template RONIN_API bool GameObject::remove_component<Terrain2D>();

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

    bool GameObject::is_active()
    {
        return m_active;
    }

    void GameObject::set_active(bool state)
    {
        if(m_active == state)
            return;

        m_active = state;
        transform()->parent_notify_active_state(this);
    }

    const bool GameObject::destroy_cancel()
    {
        return World::self()->object_desctruction_cancel(this);
    }

    const bool GameObject::is_destruction()
    {
        return World::self()->object_destruction_state(this);
    }

    void GameObject::set_active_recursivelly(bool state)
    {
        std::list<GameObject *> __stack;
        __stack.emplace_back(this);
        while(__stack.size())
        {
            for(Transform *h : __stack.front()->transform()->hierarchy)
            {
                __stack.emplace_back(h->game_object());
            }
            __stack.front()->set_active(state);
            __stack.pop_front();
        }
    }

    Transform *GameObject::transform()
    {
        // NOTE: transform всегда первый объект из контейнера m_components
        return static_cast<Transform *>(m_components.front());
    }

    SpriteRenderer *GameObject::get_sprite_renderer()
    {
        return get_component<SpriteRenderer>();
    }

    Camera2D *GameObject::get_camera2D()
    {
        return get_component<Camera2D>();
    }

    Terrain2D *GameObject::get_terrain2D()
    {
        return get_component<Terrain2D>();
    }

    void GameObject::destroy()
    {
        this->destroy(0);
    }

    void GameObject::destroy(float t)
    {
        Runtime::destroy(this, t);
    }

    Component *GameObject::add_component(Component *component)
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
                World::self()->internal_bind_script(script);
                script->OnAwake();
            }
            else if(Renderer *rend = dynamic_cast<Renderer *>(component))
            {
                // awake on renderer
            }
            else if(Light *light = dynamic_cast<Light *>(component))
            {
                World::self()->push_light_object(light);
            }
        }

        return component;
    }

    bool GameObject::remove_component(Component *component)
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
