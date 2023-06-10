#pragma once

// #define USE_TRANSFORM_HIERARCHY

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Transform : public Component
    {
    private:
        friend class World;
        friend class GameObject;
        friend class Camera;
        friend class Camera2D;
        friend class Physics2D;
        friend RONIN_API GameObject* instantiate(GameObject* obj);
        friend RONIN_API void destroy_immediate(Object* obj);

    protected:
        std::list<Transform*> hierarchy;

        Transform* m_parent;
        Vec2 p;
        float _angle_;

        void parent_notify(Vec2 lastParentPoint);
        void parent_notify_active_state(GameObject* from);

        static void hierarchy_parent_change(Transform* from, Transform* newParent);
        static void hierarchy_remove(Transform* from, Transform* off);
        static void hierarchy_remove_all(Transform* from);
        static void hierarchy_append(Transform* from, Transform* off);
        static bool hierarchy_sibiling(Transform* from, int index);

    public:
        int layer;

        Transform();
        Transform(const std::string& name);
        ~Transform();

        Transform* parent();
        void set_parent(Transform* parent, bool worldPositionStays = true);

        int child_count();
        Transform* child_of(int index);

        Transform* root();

        void look_at(Vec2 target, Vec2 axis);
        void look_at(Vec2 target);
        void look_at(Transform* target, Vec2 axis);
        void look_at(Transform* target);
        void look_at_lerp(Vec2 target, float t);
        void look_at_lerp(Transform* target, float t);

        void as_first_child();

        bool child_has(Transform* child);
        void child_append(Transform* child);
        void child_remove(Transform* child);

        // look forward axis from world coordinates
        const Vec2 forward();
        // look backward axis from world coordinates
        const Vec2 back();
        // look right axis from world coordinates
        const Vec2 right();
        // look left axis from world coordinates
        const Vec2 left();
        // look up axis from world coordinates
        const Vec2 up();
        // look down axis from world coordinates
        const Vec2 down();
        // look axis from world coordinates
        const Vec2 transform_direction(Vec2 direction);
        // look axis from world coordinates
        const Vec2 transform_direction(float x, float y);

        // get position in world space
        Vec2 position();
        // set position in world space
        const Vec2& position(const Vec2& value);

        // get position in local space from parent
        Vec2 local_position();
        // set position in local space from parent
        const Vec2& local_position(const Vec2& value);

        // Angle with Degress
        float angle();
        void angle(float value);
        float local_angle();
        void local_angle(float value);
    };

} // namespace RoninEngine::Runtime
