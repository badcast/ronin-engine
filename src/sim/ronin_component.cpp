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
            return _owner != nullptr;
        }

        Transform *Component::transform() const
        {
            return static_cast<Transform *>(_owner->m_components.front());
        }

        GameObject *Component::gameObject() const
        {
            return _owner;
        }

        Camera2D *Component::camera2D() const
        {
            return _owner->camera2D();
        }

        SpriteRenderer *Component::spriteRenderer() const
        {
            return _owner->spriteRenderer();
        }

        Terrain2D *Component::terrain2D() const
        {
            return _owner->terrain2D();
        }

        Component *Component::AddComponent(Component *component)
        {
            return gameObject()->AddComponent(component);
        }

        bool Component::RemoveComponent(Component *component)
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
