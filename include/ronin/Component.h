#pragma once

#include "Object.h"
#include "GameObject.h"

namespace RoninEngine::Runtime
{
    /**
     * @brief A base class for components in the RONIN engine.
     *      * The Component class serves as a base class for various components that can be attached to GameObjects
     * in the RONIN engine. Components are responsible for adding functionality and behavior to GameObjects.
     * This class provides common functionality and methods that can be used by derived component classes.
     */
    class RONIN_API Component : public Object, public IComponents, public IRoninBaseEvents<Component>
    {
    public:
        using Event = RoninBaseEvent<Component>;

    private:
        GameObjectRef _owner; ///< The owner GameObject to which this component is attached.
        bool _enable;         ///< Flag indicating whether the component is enabled.

    protected:
        std::list<Event> ev_destroy; ///< List of destroy events associated with this component.

    public:
        /**
         * @brief Get the enable state of the component.
         * @return True if the component is enabled, false otherwise.
         */
        bool enable() const;

        /**
         * @brief Set the enable state of the component.
         * @param value True to enable the component, false to disable it.
         */
        void enable(bool value);

        /**
         * @brief Default constructor for the Component class.
         */
        Component();

        /**
         * @brief Constructor for the Component class with a specified name.
         * @param name The name of the component.
         */
        explicit Component(const std::string &name);

        /**
         * @brief Copy constructor for the Component class (deleted).
         */
        Component(const Component &) = delete;

        /**
         * @brief Default destructor for the Component class.
         */
        virtual ~Component() = default;

        /**
         * @brief Check if the component is bound to a GameObject.
         * @return True if the component is bound, false otherwise.
         */
        const bool isBinded();

        /**
         * @brief Get the GameObject to which this component is attached.
         * @return A pointer to the GameObject.
         */
        GameObjectRef gameObject();

        /**
         * @brief Register a callback function to be called when the component is destroyed.
         * @param callback The callback function to register.
         */
        void AddOnDestroy(Event callback);

        /**
         * @brief Unregister all callbaks OnDestroy
         * @see AddOnDestroy
         */
        void ClearOnDestroy();

        /**
         * @brief Get the Transform component of the GameObject.
         * @return A pointer to the Transform component.
         */
        TransformRef transform();

        /**
         * @brief Get the SpriteRenderer component of the GameObject.
         * @return A pointer to the SpriteRenderer component.
         */
        SpriteRendererRef spriteRenderer();

        /**
         * @brief Get the Camera2D component of the GameObject.
         * @return A pointer to the Camera2D component.
         */
        Camera2DRef camera2D();

        /**
         * @brief Get the Terrain2D component of the GameObject.
         * @return A pointer to the Terrain2D component.
         */
        Terrain2DRef terrain2D();

        /**
         * @brief Add a component to the GameObject.
         *          * This function allows you to add a component to the GameObject and returns a pointer to the added component.
         * The new component will become a part of the GameObject's functionality.
         *          * @param component A pointer to the component to be added.
         * @return A pointer to the added component.
         */
        ComponentRef AddComponent(ComponentRef component);

        /**
         * @brief Remove a component from the GameObject.
         *          * This function removes the specified component from the GameObject and returns true if the removal was successful.
         * If the component is not found on the GameObject, it returns false.
         *          * @param component A pointer to the component to be removed.
         * @return True if the component was successfully removed, false otherwise.
         */
        bool RemoveComponent(ComponentRef component);

        /**
         * @brief Add a component of the specified type to the GameObject.
         *          * This template function allows you to add a component of the specified type to the GameObject and returns a pointer
         * to the added component. The new component will become a part of the GameObject's functionality.
         *          * @tparam T The type of component to add, which must be derived from the Component base class.
         * @return A pointer to the added component.
         */
        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, Ref<T>> AddComponent();

        /**
         * @brief Get a component of the specified type from the GameObject.
         *          * This template function allows you to retrieve a component of the specified type from the GameObject and returns a
         * pointer to the component if found. If the component is not found, it returns nullptr.
         *          * @tparam T The type of component to retrieve, which must be derived from the Component base class.
         * @return A pointer to the component if found, nullptr if not found.
         */
        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, Ref<T>> GetComponent();

        /**
         * @brief Remove a component of the specified type from the GameObject.
         *          * This template function allows you to remove a component of the specified type from the GameObject and returns true if
         * the removal was successful. If the component is not found on the GameObject, it returns false.
         *          * @tparam T The type of component to remove, which must be derived from the Component base class.
         * @return True if the component was successfully removed, false otherwise.
         */
        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, bool> RemoveComponent();

        /**
         * @brief Get a list of components of the specified type from the GameObject.
         *          * This template function allows you to retrieve a list of components of the specified type from the GameObject and
         * returns a list of pointers to the components if found. If no components of the specified type are found, an empty
         * list is returned.
         *          * @tparam T The type of components to retrieve, which must be derived from the Component base class.
         * @return A list of pointers to the components of the specified type.
         */
        template <typename T>
        std::enable_if_t<std::is_base_of_v<Component, T>, std::list<Ref<T>>> GetComponents();
    };
} // namespace RoninEngine::Runtime
