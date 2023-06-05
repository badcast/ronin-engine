#pragma once

#include "Object.h"

namespace RoninEngine
{

    namespace Runtime
    {

        class RONIN_API Component : public Object
        {
        private:
            friend class World;
            friend class Object;
            friend class GameObject;
            friend RONIN_API GameObject* instantiate(GameObject* obj);
            friend RONIN_API void destroy_immediate(Object* obj);

            GameObject* _owner;

        public:
            bool enabled();
            Component();
            explicit Component(const std::string& name);
            Component(const Component&) = delete;
            virtual ~Component() = default;

            const bool is_binded();
            GameObject* game_object();
            Transform* transform();
        };
    } // namespace Runtime
} // namespace RoninEngine
