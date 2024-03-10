#pragma once

#include "Declarations.h"
#include "Vec2.h"

namespace RoninEngine
{
    namespace Runtime
    {
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
            static GameObject *CreateEmptyGameObject(Vec2 position = Vec2::zero, float angle = 0);
            static GameObject *CreateEmptyGameObject(const std::string &name, Vec2 position = Vec2::zero, float angle = 0);
            static GameObject *CreateBox2D(
                Vec2 position = Vec2::zero, float angle = 0, Vec2 size = Vec2::one, Color fillColor = Color::white);
            static Camera2D *CreateCamera2D(Vec2 position = Vec2::zero);
            static Sprite *CreateEmptySprite(bool localSprite = true);
            static Sprite *CreateSpriteRectangle(bool localSprite = true, Vec2 size = Vec2::one, Color fillColor = Color::white);
            static Sprite *CreateSpriteCircle(
                bool localSprite = true, Vec2 size = Vec2::one, float radius = 1.f, Color fillColor = Color::white);
            static Sprite *CreateSpriteTriangle(
                bool local = true, Vec2 size = Vec2::one, float height = 1.f, Color fillColor = Color::white);
            static Sprite *CreateSpriteFrom(Image *surface, bool localSprite = true);
        };
    } // namespace Runtime
} // namespace RoninEngine
