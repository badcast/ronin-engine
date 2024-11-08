#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Transform : public Component
    {
    private:
        TransformRef m_parent;
        Vec2 _position;
        float _angle_;

    protected:
        std::list<TransformRef> hierarchy;

        void parent_notify_active_state(GameObjectRef from);

    public:
        Transform();
        Transform(const std::string &name);

        TransformRef parent() const;
        void setParent(TransformRef parent, bool worldPositionStays = true);

        void Detach();

        int childCount() const;
        TransformRef childOf(int index);

        void LookAt(Vec2 target, Vec2 axis);
        void LookAt(Vec2 target);
        void LookAt(TransformRef target, Vec2 axis);
        void LookAt(TransformRef target);
        void LookAtLerp(Vec2 target, float t);
        void LookAtLerp(Vec2 target, Vec2 axis, float t);
        void LookAtLerp(TransformRef target, float t);
        void LookAtLerp(TransformRef target, Vec2 axis, float t);

        void AsFirstChild();
        void AsLastChild();

        bool childContain(TransformRef child);
        void childAdd(TransformRef child);
        void childRemove(TransformRef child);

        std::list<TransformRef> GetChilds() const;

        // look forward axis from world coordinates
        const Vec2 forward() const;
        // look backward axis from world coordinates
        const Vec2 back() const;
        // look right axis from world coordinates
        const Vec2 right() const;
        // look left axis from world coordinates
        const Vec2 left() const;
        // look up axis from world coordinates
        const Vec2 up() const;
        // look down axis from world coordinates
        const Vec2 down() const;
        // look axis from world coordinates
        const Vec2 TransformDirection(Vec2 direction) const;
        // look axis from world coordinates
        const Vec2 TransformDirection(float x, float y) const;
        // move the Transform by translation
        void Translate(Vec2 translation);

        /**
         * @brief Looks at the target
         * @param target the target candidate for look
         * @param angle the angle, value of degress 0 ... 360
         * @return result of viewed sector
         */
        const bool LookOfAngle(Transform *target, float angle) const;
        /**
         * @brief Looks at the target
         * @param target the target candidate for look
         * @param angle the angle, value of degress 0 ... 360
         * @return result of viewed sector
         */
        const bool LookOfAngle(Vec2 target, float angle) const;

        // get position in local space from parent
        Vec2 localPosition() const;
        // set position in local space from parent
        const Vec2 localPosition(Vec2 value);

        // get position in world space
        Vec2 position() const;
        // set position in world space
        const Vec2 position(Vec2 value);

        // Angle with Degress
        float angle() const;
        void angle(float value);
        float localAngle() const;
        void localAngle(float value);

        // Layer for order
        int layer() const;
        void layer(int value);

        // Layer for zOrder
        int zOrder() const;
        void zOrder(int value);

        std::vector<TransformRef> GetAllTransforms();

        static TransformRef root();
    };

} // namespace RoninEngine::Runtime
