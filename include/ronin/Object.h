#pragma once

#include "Namespaces.h"
#include "Vec2.h"

namespace RoninEngine
{
    namespace Runtime
    {
        extern RONIN_API GameObject *create_game_object();
        extern RONIN_API GameObject *create_game_object(const std::string &name);

        // Уничтожает объект после рендера.
        extern RONIN_API void Destroy(GameObject *obj);

        // Уничтожает объект после прошедшего времени.
        extern RONIN_API void Destroy(GameObject *obj, float t);

        // Клонирует объект
        extern RONIN_API GameObject *Instantiate(GameObject *obj);
        // Клонирует объект
        extern RONIN_API GameObject *Instantiate(GameObject *obj, Vec2 position, float angle = 0);
        // Клонирует объект
        extern RONIN_API GameObject *Instantiate(GameObject *obj, Vec2 position, Transform *parent, bool worldPositionStay = true);

        class RONIN_API Object
        {
        protected:
            // unique type (static)
            char *_type_;
            // object name (realtime)
            std::string m_name;

        public:
            Object();
            Object(const std::string &name);
            virtual ~Object() = default;

            std::string &name(const std::string &newName);

            std::string &name();

            const char *getType() const;
        };

        class RONIN_API Primitive
        {
        public:
            static GameObject *create_empty_game_object(Vec2 position = Vec2::zero);
            static GameObject *create_box2d(Vec2 position = Vec2::zero, float angle = 0, Color fillColor = Color::white);
            static Camera2D *create_camera2D(Vec2 position = Vec2::zero);
            static Sprite *create_empty_sprite2D();
            static Sprite *create_sprite2D_box(Vec2 size = Vec2::one, Color fillColor = Color::white);
            static Sprite *create_sprite2D_circle(Vec2 size = Vec2::one, float radius = 1.f, Color fillColor = Color::white);
            static Sprite *create_sprite2D_triangle(Vec2 size = Vec2::one, float height = 1.f, Color fillColor = Color::white);
            static Sprite *create_sprite2d_from(Image *surface);
        };
    } // namespace Runtime
} // namespace RoninEngine
