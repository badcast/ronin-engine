#include "ronin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        // base component
        Component::Component()
            : Component(DESCRIBE_AS_MAIN_OFF(Component))
        {
        }

        Component::Component(const std::string& name)
            : _owner(nullptr)
            , Object(DESCRIBE_AS_ONLY_NAME(Component))
        {
            DESCRIBE_AS_MAIN(Component);
        }

        const bool Component::is_binded() { return _owner != nullptr; }

        Transform* Component::transform() { return _owner->transform(); }

        GameObject* Component::game_object() { return _owner; }

        Camera2D* Component::get_camera2D() { return _owner->get_camera2D(); }

        SpriteRenderer* Component::get_sprite_renderer() { return _owner->get_sprite_renderer(); }

        Terrain2D* Component::get_terrain2D() { return _owner->get_terrain2D(); }

        Component* Component::add_component(Component* component) { return game_object()->add_component(component); }

        bool Component::remove_component(Component* component) { return game_object()->remove_component(component); }

    } // namespace Runtime
} // namespace RoninEngine
