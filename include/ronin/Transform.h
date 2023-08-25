#pragma once

// #define USE_TRANSFORM_HIERARCHY

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Transform : public Component
    {
    private:
        Transform *m_parent;
        Vec2 _position;
        float _angle_;

    protected:
        std::list<Transform *> hierarchy;

        void parent_notify(Vec2 lastParentPoint);
        void parent_notify_active_state(GameObject *from);

    public:
        int layer;

        Transform();
        Transform(const std::string &name);

        Transform *parent() const;
        void set_parent(Transform *parent, bool worldPositionStays = true);

        int ChildCount() const;
        Transform *ChildOf(int index);

        void LookAt(Vec2 target, Vec2 axis);
        void LookAt(Vec2 target);
        void LookAt(Transform *target, Vec2 axis);
        void LookAt(Transform *target);
        void LookAtLerp(Vec2 target, float t);
        void LookAtLerp(Transform *target, float t);

        void AsFirstChild();

        bool ChildContain(Transform *child);
        void ChildAdd(Transform *child);
        void ChildRemove(Transform *child);

        // look forward axis from world coordinates
        const Vec2 forward() const;
        // look forward axis from world coordinates with forces
        const Vec2 forward(float force) const;
        // look backward axis from world coordinates
        const Vec2 back() const;
        // look backward axis from world coordinates with forces
        const Vec2 back(float force) const;
        // look right axis from world coordinates
        const Vec2 right();
        // look left axis from world coordinates
        const Vec2 left();
        // look up axis from world coordinates
        const Vec2 up();
        // look down axis from world coordinates
        const Vec2 down();
        // look axis from world coordinates
        const Vec2 TransformDirection(Vec2 direction);
        // look axis from world coordinates
        const Vec2 TransformDirection(float x, float y);

        /**
         * @brief Looks at the target
         * @param target the target candidate for look
         * @param maxAngle the angle, value from degress 0 ... 360
         * @return result of viewed sector
         */
        const bool LookOfAngle(Transform *target, float maxAngle) const;
        /**
         * @brief Looks at the target
         * @param target the target candidate for look
         * @param maxAngle the angle, value from degress 0 ... 360
         * @return result of viewed sector
         */
        const bool LookOfAngle(Vec2 target, float maxAngle) const;

        // get position in local space from parent
        Vec2 localPosition() const;
        // set position in local space from parent
        const Vec2 &localPosition(const Vec2 &value);

        // get position in world space
        Vec2 position() const;
        // set position in world space
        const Vec2 &position(const Vec2 &value);

        // Angle with Degress
        float angle() const;
        void angle(float value);
        float localAngle() const;
        void localAngle(float value);

        static Transform *root();
    };

} // namespace RoninEngine::Runtime
