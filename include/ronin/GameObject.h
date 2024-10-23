#pragma once

#include "Behaviour.h"

namespace RoninEngine
{
    namespace Runtime
    {
        enum ZOrderBy
        {
            Inherit,
            LinearAdd
        };

        class RONIN_API GameObject final : public Object, public IComponents, public IRoninBaseEvents<GameObject>
        {
        public:
            using Event = RoninBaseEvent<GameObject>;

        private:
            std::list<Component *> m_components;
            std::list<Event> ev_destroy;
            int m_layer;
            int m_zOrder;
            bool m_active;

        protected:
            enum BindType
            {
                Bind_Start = 1,
                Bind_Update = 2,
                Bind_LateUpdate = 4,
                Bind_Gizmos = 8
            };

            void bind_script(BindType bindFlags, Behaviour *script);

        public:
            /**
             * @brief Default constructor for GameObject.
             */
            GameObject();

            /**
             * @brief Constructor for GameObject with a specified name.
             * @param name The name to assign to the GameObject.
             */
            GameObject(const std::string &name);

            /**
             * @brief Deleted copy constructor for GameObject.
             */
            GameObject(const GameObject &) = delete;

            /**
             * @brief Check if the GameObject is active.
             * @return True if the GameObject is active, false otherwise.
             */
            bool isActive();

            /**
             * @brief Check if the GameObject or any of its children is active.
             * @return True if the GameObject or any child is active, false otherwise.
             */
            bool isActiveInHierarchy();

            /**
             * @brief Set the active state of the GameObject.
             * @param state The new active state.
             */
            void SetActive(bool state);

            /**
             * @brief Set the layer for draw order
             * @param layer The layer order to set
             */
            void SetLayer(int layer);

            /**
             * @brief Get The Layer order of the render
             * @return layer
             */
            int GetLayer() const;

            /**
             * @brief Get Z-Order for draw orders
             * @return Z-Order value
             */
            int GetZOrder() const;

            /**
             * @brief Set Z-Order for draw orders
             * @param value The new Z-Order for set
             */
            void SetZOrder(int value);

            /**
             * @brief Set Z-Order for draw all oreders on children.
             * @param value The new Z-Order for set all childrens.
             */
            void SetZOrderAll(int value, ZOrderBy orderBy);

            /**
             * @brief Set the active state recursively for the GameObject and its children.
             * @param state The new active state.
             */
            void SetActiveRecursivelly(bool state);

            /**
             * @brief Get the Transform component of the GameObject.
             * @return A pointer to the Transform component.
             */
            Transform *transform() const;

            /**
             * @brief Get the SpriteRenderer component of the GameObject.
             * @return A pointer to the SpriteRenderer component, or nullptr if not found.
             */
            SpriteRenderer *spriteRenderer() const;

            /**
             * @brief Get the Camera2D component of the GameObject.
             * @return A pointer to the Camera2D component, or nullptr if not found.
             */
            Camera2D *camera2D() const;

            /**
             * @brief Get the Terrain2D component of the GameObject.
             * @return A pointer to the Terrain2D component, or nullptr if not found.
             */
            Terrain2D *terrain2D() const;

            /**
             * @brief Check if the gameobject is PrefabObject
             * @return True if the GameObject is PrfabObject, false otherwise.
             *
             * @see PrefabObject
             */
            bool isPrefab();

            /**
             * @brief Mark the GameObject for destruction now.
             */
            void Destroy();

            /**
             * @brief Mark the GameObject for destruction with a delayed time.
             * @param time The time delay before destruction.
             */
            void Destroy(float time);

            /**
             * @brief Cancel the scheduled destruction for the GameObject.
             * @return True if the destruction was canceled, false otherwise.
             */
            const bool CancelDestroy();

            /**
             * @brief Checks whether the object is currently being destroyed.
             *
             * This function is used to determine if the object is in the process of being destroyed.
             * It returns true if the destruction process is currently ongoing, and false otherwise.
             *
             * @return True if the object is being destroyed, false otherwise.
             */
            const bool isDestroying();

            /**
             * @brief Adds a specific component to the GameObject.
             *
             * This function adds the provided component to the GameObject's list of components.
             * The component must be a valid instance derived from the base class Component.
             *
             * @param component A pointer to the component to be added.
             * @return A pointer to the added component, or nullptr if adding failed.
             */
            Component *AddComponent(Component *component);

            /**
             * @brief Removes a specific component from the GameObject.
             *
             * This function removes the provided component from the GameObject's list of components.
             * The component must be a valid instance that is currently attached to the GameObject.
             *
             * @param component A pointer to the component to be removed.
             * @return True if the component was successfully removed, false if it was not found.
             */
            bool RemoveComponent(Component *component);

            /**
             * @brief Adds a new component of the specified type to the GameObject.
             *
             * This function adds a new instance of the specified component type to the GameObject.
             * The component type must be derived from the base class Component.
             *
             * @tparam T The type of component to add. Must be derived from Component.
             * @return A pointer to the newly added component, or nullptr if adding failed.
             */
            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T *> AddComponent();

            /**
             * @brief Retrieves the first component of the specified type from the GameObject.
             *
             * This function attempts to retrieve the first instance of the specified component type
             * that is attached to the GameObject. The component type must be derived from the base class Component.
             *
             * @tparam T The type of component to retrieve. Must be derived from Component.
             * @return A pointer to the found component, or nullptr if the component was not found.
             */
            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, T *> GetComponent() const;

            /**
             * @brief Removes the first component of the specified type from the GameObject.
             *
             * This function removes the first instance of the specified component type from the GameObject.
             * The component type must be derived from the base class Component.
             *
             * @tparam T The type of component to remove. Must be derived from Component.
             * @return True if the component was successfully removed, false if it was not found.
             */
            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, bool> RemoveComponent();

            /**
             * @brief Retrieves a list of all components of the specified type from the GameObject.
             *
             * This function collects all instances of the specified component type that are attached
             * to the GameObject and returns them in a list.
             * The component type must be derived from the base class Component.
             *
             * @tparam T The type of components to retrieve. Must be derived from Component.
             * @return A list containing pointers to the found components.
             */
            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T *>> GetComponents() const;

            /**
             * @brief Retrieves a list of all components of the specified type from the GameObject an childs.
             *
             * This function collects all instances of the specified component type that are attached
             * to the GameObject and returns them in a list with childrens.
             * The component type must be derived from the base class Component.
             *
             * @tparam T The type of components to retrieve. Must be derived from Component.
             * @return A list containing pointers to the found components.
             */
            template <typename T>
            std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T *>> GetComponentsAnChilds() const;

            /**
             * @brief Register a callback for the GameObject's "destroy" event.
             *
             * This function allows you to register a callback for a GameObject object,
             * which will be invoked when the "destroy" event occurs.
             *
             * @param callback An Event object representing a function or functor that
             *                will be called when the GameObject is destroyed.
             *
             * @see ClearOnDestroy
             */
            void AddOnDestroy(Event callback);

            /**
             * @brief Unregister all callbaks OnDestroy
             * @see AddOnDestroy
             */
            void ClearOnDestroy();
        };

        template <typename T>
        inline std::enable_if_t<std::is_base_of<Component, T>::value, T *> GameObject::AddComponent()
        {
            static_assert(!(std::is_same<T, Transform>::value || std::is_base_of<Transform, T>::value), "Transform component can't be assigned");

            // init component
            T *component = RoninMemory::alloc<T>();
            this->AddComponent(static_cast<Component *>(component));

            if constexpr(std::is_base_of<Behaviour, T>::value)
            {
                int flags = 0;

#if 1
#ifdef __clang__ // it's work only Clang
#define CHECK_BASE_OVERRIDDEN(BASE, BINDER, METHOD) \
    if((&BASE::METHOD) != (&T::METHOD))             \
        flags |= BINDER;
#else // it's work on GCC
#define CHECK_BASE_OVERRIDDEN(BASE, BINDER, METHOD)                                               \
    if constexpr(reinterpret_cast<void *>(&BASE::METHOD) != reinterpret_cast<void *>(&T::METHOD)) \
        flags |= BINDER;
#endif

#else // if the not 0

#define CHECK_BASE_OVERRIDDEN(BASE, BINDER, METHOD) \
    if((&T::METHOD) != (&BASE::METHOD))             \
        flags |= BINDER;
#endif
                CHECK_BASE_OVERRIDDEN(Behaviour, Bind_Start, OnStart);
                CHECK_BASE_OVERRIDDEN(Behaviour, Bind_Update, OnUpdate);
                CHECK_BASE_OVERRIDDEN(Behaviour, Bind_LateUpdate, OnLateUpdate);
                CHECK_BASE_OVERRIDDEN(Behaviour, Bind_Gizmos, OnGizmos);
#undef CHECK_BASE_OVERRIDDEN

                bind_script(static_cast<BindType>(flags), static_cast<Behaviour *>(component));
            }

            return component;
        } // namespace Runtime

        template <typename T>
        inline std::enable_if_t<std::is_base_of<Component, T>::value, bool> GameObject::RemoveComponent()
        {
            static_assert(!(std::is_same<T, Transform>::value || std::is_base_of<Transform, T>::value), "Transform component can't remove, basic component type");

            T *target = GetComponent<T>();
            if(target == nullptr)
                return false;
            return RemoveComponent(target);
        }

        template <typename T>
        inline std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T *>> GameObject::GetComponents() const
        {
            T *cast;
            std::list<T *> types;
            for(auto iter = std::begin(m_components); iter != std::end(m_components); ++iter)
            {
                if((cast = dynamic_cast<T *>(*iter)) != nullptr)
                {
                    types.emplace_back(cast);
                }
            }

            return types;
        }

        template <typename T>
        inline std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T *>> GameObject::GetComponentsAnChilds() const
        {
            std::list<T *> types;
            std::list<const GameObject *> stacks;
            stacks.push_back(this);

            while(!stacks.empty())
            {
                types.merge(stacks.front()->GetComponents<T>());
                for(const Transform *c : stacks.front()->transform()->GetChilds())
                {
                    stacks.push_back(c->gameObject());
                }
                stacks.pop_front();
            }
            return types;
        }

        template <typename T>
        inline std::enable_if_t<std::is_base_of<Component, T>::value, T *> GameObject::GetComponent() const
        {
            if constexpr(std::is_same<T, Transform>::value)
            {
                return reinterpret_cast<Transform *>(m_components.front());
            }
            else
            {
                auto iter = std::find_if(std::cbegin(m_components), std::cend(m_components), [](const Component *c) { return dynamic_cast<const T *>(c) != nullptr; });

                if(iter != std::end(m_components))
                    return static_cast<T *>(*iter);
            }
            return nullptr;
        }

        template <typename T>
        std::enable_if_t<std::is_base_of<Component, T>::value, T *> Component::AddComponent()
        {
            return _owner->AddComponent<T>();
        }

        template <typename T>
        std::enable_if_t<std::is_base_of<Component, T>::value, T *> Component::GetComponent() const
        {
            return _owner->GetComponent<T>();
        }

        template <typename T>
        std::enable_if_t<std::is_base_of<Component, T>::value, bool> Component::RemoveComponent()
        {
            return _owner->RemoveComponent<T>();
        }

        template <typename T>
        std::enable_if_t<std::is_base_of<Component, T>::value, std::list<T *>> Component::GetComponents() const
        {
            return _owner->GetComponents<T>();
        }
    } // namespace Runtime
} // namespace RoninEngine
