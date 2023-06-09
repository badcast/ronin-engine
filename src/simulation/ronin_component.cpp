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

        GameObject* Component::game_object() { return _owner; }

        Transform* Component::transform()
        {
            if (!is_binded())
                throw std::runtime_error("This component isn't binded");
            return _owner->transform();
        }
    } // namespace Runtime
} // namespace RoninEngine
