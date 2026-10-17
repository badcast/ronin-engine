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
            if(_owner.isNull() || _owner->m_components.empty())
                return TransformRef();
            return StaticCast<Transform>(_owner->m_components.front());
        }

        GameObjectRef Component::gameObject()
        {
            return _owner;
        }

        Camera2DRef Component::camera2D()
        {
            return _owner ? _owner->camera2D() : Camera2DRef();
        }

        SpriteRendererRef Component::spriteRenderer()
        {
            return _owner ? _owner->spriteRenderer() : SpriteRendererRef();
        }

        Terrain2DRef Component::terrain2D()
        {
            return _owner ? _owner->terrain2D() : Terrain2DRef();
        }

        ComponentRef Component::AddComponent(ComponentRef component)
        {
            return _owner ? _owner->AddComponent(component) : ComponentRef();
        }

        bool Component::RemoveComponent(ComponentRef component)
        {
            return _owner ? _owner->RemoveComponent(component) : false;
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
