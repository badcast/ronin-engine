#include "ronin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        // base class
        Object::Object() : Object(DESCRIBE_AS_MAIN_OFF(Object))
        {
        }

        Object::Object(const std::string &name) : m_name(name)
        {
            if(switched_world != nullptr)
            {
                DESCRIBE_AS_MAIN(Object);
                ::check_object(this);
                ++switched_world->irs->objects;
            }
        }

        bool Object::exists() const
        {
            return object_instanced(this);
        }

        std::string &Object::name(const std::string &newName)
        {
            return (m_name = newName);
        }

        std::string &Object::name()
        {
            return m_name;
        }

        const char *Object::getType() const
        {
            return _type_;
        }

        Object::operator bool()
        {
            return object_instanced(this);
        }
    } // namespace Runtime
} // namespace RoninEngine
