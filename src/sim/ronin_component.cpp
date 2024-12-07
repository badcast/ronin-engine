#include "ronin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        // base component
        Component::Component() : Component(DESCRIBE_AS_MAIN_OFF(Component))
        {
        }

        Component::Component(const std::string &name) : _owner(nullptr), Object(DESCRIBE_AS_ONLY_NAME(Component)), _enable(true)
        {
            DESCRIBE_AS_MAIN(Component);
        }

        bool Component::enable() const
        {
            return _enable;
        }

        void Component::enable(bool value)
        {
            _enable = value;
        }

        const bool Component::isBinded()
        {
            return !_owner.isNull();
        }

        TransformRef Component::transform()
        {
            return StaticCast<Transform>(_owner->m_components.front());
        }

        GameObjectRef Component::gameObject()
        {
            return _owner;
        }

        Camera2DRef Component::camera2D()
        {
            return _owner->camera2D();
        }

        SpriteRendererRef Component::spriteRenderer()
        {
            return _owner->spriteRenderer();
        }

        Terrain2DRef Component::terrain2D()
        {
            return _owner->terrain2D();
        }

        ComponentRef Component::AddComponent(ComponentRef component)
        {
            return gameObject()->AddComponent(component);
        }

        bool Component::RemoveComponent(ComponentRef component)
        {
            return gameObject()->RemoveComponent(component);
        }

        void Component::AddOnDestroy(Event callback)
        {
            ev_destroy.emplace_back(callback);
        }

        void Component::ClearOnDestroy()
        {
            ev_destroy.clear();
        }

    } // namespace Runtime
} // namespace RoninEngine
