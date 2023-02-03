#include "ronin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        // base component
        Component::Component()
            : Component(typeid(Component).name())
        {
        }

        Component::Component(const std::string& name)
            : Object(name)
            , pin(nullptr)
        {
        }

        const bool Component::isBind() { return pin != nullptr; }

        GameObject* Component::gameObject() { return pin; }

        Transform* Component::transform()
        {
            if (!isBind())
                throw std::runtime_error("This component isn't binded");
            return pin->getComponent<Transform>();
        }
    } // namespace Runtime
} // namespace RoninEngine
