#include "ronin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        // base component
        Component::Component()
            : Component(DESCRIBE_TYPE(Component))
        {
        }

        Component::Component(const std::string& name)
            : Object(name)
            , _owner(nullptr)
        {
        }

        const bool Component::isBind() { return _owner != nullptr; }

        GameObject* Component::gameObject() { return _owner; }

        Transform* Component::transform()
        {
            if (!isBind())
                throw std::runtime_error("This component isn't binded");
            return static_cast<Transform*>(_owner->m_components.front());
        }
    } // namespace Runtime
} // namespace RoninEngine
