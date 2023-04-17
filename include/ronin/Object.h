#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {

        extern RONIN_API Transform* create_empty_transform();
        extern RONIN_API GameObject* create_empty_gameobject();

        extern RONIN_API GameObject* CreateGameObject();
        extern RONIN_API GameObject* CreateGameObject(const std::string& name);

        // Уничтожает объект после рендера.
        extern RONIN_API void Destroy(Object* obj);

        // Уничтожает объект после прошедшего времени.
        extern RONIN_API void Destroy(Object* obj, float t);

        // Уничтожает объект принудительно игнорируя все условия его существования.
        extern RONIN_API void Destroy_Immediate(Object* obj);

        // Проверка на существование объекта
        extern RONIN_API bool instanced(Object* obj);

        // clone a object
        template <typename ObjectType>
        extern RONIN_API ObjectType* Instantiate(ObjectType* obj);
        // Клонирует объект
        extern RONIN_API GameObject* Instantiate(GameObject* obj, Vec2 position, float angle = 0);
        // Клонирует объект
        extern RONIN_API GameObject* Instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionStay = true);

        class RONIN_API Object
        {
            template <typename ObjectType>
            friend RONIN_API ObjectType* Instantiate(ObjectType* obj);
            friend RONIN_API GameObject* Instantiate(GameObject* obj);
            friend RONIN_API GameObject* Instantiate(GameObject* obj, Vec2 position, float angle);
            friend RONIN_API GameObject* Instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionState);

            friend RONIN_API void Destroy(Object* obj);
            friend RONIN_API void Destroy(Object* obj, float t);
            friend RONIN_API void Destroy_Immediate(Object* obj);
            std::size_t id;

        protected:
            std::string m_name;

        public:
            std::string& name(const std::string& newName);
            const std::string& name();

            std::size_t ID();

            Object();
            Object(const std::string& name);
            virtual ~Object() = default;

            void Destroy();

            /// Check a exist an object
            operator bool();
        };

        template <typename base, typename _derived>
        constexpr bool object_base_of()
        {
            return std::is_base_of<base, _derived>();
        }

        template <typename base, typename _derived>
        constexpr bool object_base_of(base* obj, _derived* compare)
        {
            return std::is_base_of<base, _derived>();
        }
    } // namespace Runtime
} // namespace RoninEngine
