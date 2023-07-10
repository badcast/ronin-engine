#pragma once

#include "Object.h"
#include "GameObject.h"

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
            Terrain2D* get_terrain2D();
            Component* add_component(Component* component);
            bool remove_component(Component* component);

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T*> add_component()
            {
                return _owner->add_component<T>();
            };
            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T*> get_component()
            {
                return _owner->get_component<T>();
            };
            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, bool> remove_component()
            {
                return _owner->remove_component<T>();
            };
            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T*>> get_components()
            {
                return _owner->get_components<T>();
            };
        };
    } // namespace Runtime
} // namespace RoninEngine
