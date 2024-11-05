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
            if(currentWorld != nullptr)
            {
                DESCRIBE_AS_MAIN(Object);

#if RONIN_USE_TYPESTR
                ::check_object(this);
#endif

                ++currentWorld->irs->objects;
            }
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

    } // namespace Runtime
} // namespace RoninEngine
