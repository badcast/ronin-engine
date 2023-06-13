#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        class RONIN_API GameObject final : public Object
        {
            friend class Camera2D;
            friend class Component;
            friend class World;
            friend GameObject* instantiate(GameObject* obj);
            friend GameObject* instantiate(GameObject* obj, Vec2 position, float angle);
            friend GameObject* instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionState);
            friend void destroy(GameObject* obj);
            friend void destroy(GameObject* obj, float t);
            friend void destroy_immediate(GameObject* obj);

        private:
            std::list<Component*> m_components;
            bool m_active;

        public:
            GameObject();

            GameObject(const std::string&);

            GameObject(const GameObject&) = delete;

            ~GameObject();

            bool is_active();
            bool is_active_hierarchy();
            void set_active(bool state);
            void set_active_recursivelly(bool state);

            Transform* transform();

            SpriteRenderer* get_sprite_renderer();

            Camera2D* get_camera2D();

            Terrain2D* get_terrain2D();

            void destroy();
            void destroy(float time);
            const bool destroy_cancel();
            const bool is_destruction();

            Component* add_component(Component* component);
            bool remove_component(Component* component);

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T*> add_component();

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T*> get_component();

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, bool> remove_component();

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T*>> get_components();
        };

        template <typename T>
        std::enable_if_t<std::is_base_of<Component, T>::value, T*> GameObject::add_component()
        {
            static_assert(!(std::is_same<T, Transform>::value || std::is_base_of<Transform, T>::value), "Transform component can't assign");

            // init component
            T* component = RoninMemory::alloc<T>();
            this->add_component(static_cast<Component*>(component));

            return component;
        }

        template <typename T>
        std::enable_if_t<std::is_base_of<Component, T>::value, bool> GameObject::remove_component()
        {
            static_assert(!(std::is_same<T, Transform>::value || std::is_base_of<Transform, T>::value), "Transform component can't remove, basic component type");
            T* target = get_component<T>();
            if (target == nullptr)
                return false;
            return remove_component(target);
        }

        template <typename T>
        std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T*>> GameObject::get_components()
        {
            std::list<T*> types;
            T* cast;
            for (auto iter = std::begin(m_components); iter != std::end(m_components); ++iter) {
                if ((cast = dynamic_cast<T*>(*iter)) != nullptr) {
                    types.emplace_back(cast);
                }
            }

            return types;
        }

        template <typename T>
        std::enable_if_t<std::is_base_of<Component, T>::value, T*> GameObject::get_component()
        {
            if constexpr (std::is_same<T, Transform>::value) {
                return reinterpret_cast<Transform*>(m_components.front());
            } else {
                auto iter = std::find_if(begin(m_components), end(m_components), [](Component* c) { return dynamic_cast<T*>(c) != nullptr; });

                if (iter != end(m_components))
                    return static_cast<T*>(*iter);
            }
            return nullptr;
        }

    } // namespace Runtime
} // namespace RoninEngine
