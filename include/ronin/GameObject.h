#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        //[attrib class]
        template <typename T>
        class GameObjectTypeHelper
        {
        public:
            static constexpr T* get_type(const std::list<Component*>& container)
            {
                if constexpr (std::is_same<T, Transform>::value) {
                    return reinterpret_cast<Transform*>(container.front());
                } else {
                    auto iter = std::find_if(begin(container), end(container), [](Component* c) { return dynamic_cast<T*>(c) != nullptr; });

                    if (iter != end(container))
                        return reinterpret_cast<T*>(*iter);
                }
                return nullptr;
            }

            static std::list<T*> get_types(const std::list<Component*>& container)
            {
                std::list<T*> types;
                T* cast;
                for (auto iter = std::begin(container); iter != std::end(container); ++iter) {
                    if ((cast = dynamic_cast<T*>(*iter))) {
                        types.emplace_back(cast);
                    }
                }

                return types;
            }
        };

        class RONIN_API GameObject final : public Object
        {
            friend class Camera2D;
            friend class Component;
            friend GameObject* instantiate(GameObject* obj);
            friend GameObject* instantiate(GameObject* obj, Vec2 position, float angle);
            friend GameObject* instantiate(GameObject* obj, Vec2 position, Transform* parent, bool worldPositionState);
            friend void destroy(Object* obj);
            friend void destroy(Object* obj, float t);
            friend void destroy_immediate(Object* obj);

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

            Component* add_component(Component* component);

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T*> add_component();

            SpriteRenderer* spriteRenderer() { return get_component<SpriteRenderer>(); }

            Camera2D* camera2D() { return get_component<Camera2D>(); }

            Terrain2D* terraind2D() { return get_component<Terrain2D>(); }

            template <typename T>
            T* get_component();

            template <typename T>
            std::list<T*> get_components();
        };

        template <typename T>
        // std::enable_if<!std::is_same<RoninEngine::Runtime::Transform,T>::value, std::list<T*>>
        std::list<T*> GameObject::get_components()
        {
            return GameObjectTypeHelper<T>::get_types(this->m_components);
        }

    } // namespace Runtime
} // namespace RoninEngine
