#pragma once

#include "Vec2.h"
#include "Declarations.h"

namespace RoninEngine
{
    namespace Runtime
    {
        class RONIN_API Object : public RoninPointer
        {
        protected:
            const char * _type_;
            std::string m_name;
        public:
            Object();
            Object(const std::string &name);
            virtual ~Object() = default;

            std::string name(const std::string &newName);

            std::string name();
        };        
    } // namespace Runtime
} // namespace RoninEngine
