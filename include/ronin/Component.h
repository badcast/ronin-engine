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
            GameObject *_owner;

        public:
            bool enabled();
            Component();
            explicit Component(const std::string &name);
            Component(const Component &) = delete;
            virtual ~Component() = default;

            const bool is_binded();
            GameObject *gameObject();
            Transform *transform();
            SpriteRenderer *spriteRenderer();
            Camera2D *camera2D();
            Terrain2D *terrain2D();
            Component *AddComponent(Component *component);
            bool RemoveComponent(Component *component);

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T *> AddComponent()
            {
                return _owner->AddComponent<T>();
            }

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T *> GetComponent()
            {
                return _owner->GetComponent<T>();
            }

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, bool> RemoveComponent()
            {
                return _owner->RemoveComponent<T>();
            }

            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T *>> GetComponents()
            {
                return _owner->GetComponents<T>();
            }
        };
    } // namespace Runtime
} // namespace RoninEngine
