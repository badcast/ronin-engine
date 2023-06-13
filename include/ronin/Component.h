#pragma once

#include "Object.h"

namespace RoninEngine
{

    namespace Runtime
    {

        class RONIN_API Component : public Object, public IComponents
        {
        private:
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
            SpriteRenderer* get_sprite_renderer();
            Camera2D* get_camera2D();
            Terrain2D * get_terrain2D();
        };
    } // namespace Runtime
} // namespace RoninEngine
