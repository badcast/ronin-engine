#pragma once

#include "Namespaces.h"
#include "Vec2.h"
#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        extern RONIN_API GameObject* create_game_object();
        extern RONIN_API GameObject* create_game_object(const std::string& name);

        // Уничтожает объект после рендера.
        extern RONIN_API void destroy(Object* obj);

        // Уничтожает объект после прошедшего времени.
        extern RONIN_API void destroy(Object* obj, float t);

        // Уничтожает объект принудительно игнорируя все условия его существования.
        extern RONIN_API void destroy_immediate(Object* obj);

        // Проверка на существование объекта
        extern RONIN_API bool instanced(Object* obj);

        // Клонирует объект
        extern RONIN_API GameObject* instantiate(GameObject* obj, Vec2 position, float angle = 0);
        // Клонирует объект
        extern RONIN_API GameObject* instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionStay = true);

        class RONIN_API Object
        {
            friend RONIN_API GameObject* instantiate(GameObject* obj);
            friend RONIN_API GameObject* instantiate(GameObject* obj, Vec2 position, float angle);
            friend RONIN_API GameObject* instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionStay);

            friend RONIN_API void destroy(Object* obj);
            friend RONIN_API void destroy(Object* obj, float t);
            friend RONIN_API void destroy_immediate(Object* obj);
            std::size_t id;

        protected:
            std::string m_name;

        public:
            std::string& name(const std::string& newName);
            const std::string& name();

            std::size_t get_id();

            Object();
            Object(const std::string& name);
            virtual ~Object() = default;

            void destroy();

            const bool exists();

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

        class RONIN_API Primitive
        {
        public:
            static GameObject* CreateEmptyGameObject(Vec2 position = Vec2::zero);
            static Camera2D* CreateCamera2D(Vec2 position = Vec2::zero);
            static Sprite* CreateSprite2D(Vec2Int size = Vec2Int::zero, Color fillColor = Color::white);
        };
    } // namespace Runtime
} // namespace RoninEngine
