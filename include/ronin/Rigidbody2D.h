#pragma once

#include "begin.h"

namespace box2d
{
    struct Body;
}

namespace RoninEngine::Runtime
{
    enum class BodyType2D
    {
        Dynamic = 0,
        Kinematic = 1,
        Static = 2
    };

    enum class RigidbodyConstraints2D
    {
        None = 0,
        FreezePositionX = 1 << 0,
        FreezePositionY = 1 << 1,
        FreezePosition = FreezePositionX | FreezePositionY,
        FreezeRotation = 1 << 2,
        FreezeAll = FreezePosition | FreezeRotation
    };

    class RONIN_API Rigidbody2D : public Behaviour
    {
    protected:
        box2d::Body *m_internalBody{nullptr};

    public:
        BodyType2D bodyType{BodyType2D::Dynamic};
        float mass{1.0f};
        float linearDamping{0.0f};
        float angularDamping{0.05f};
        float gravityScale{1.0f};
        bool simulated{true};
        bool useAutoMass{false};
        RigidbodyConstraints2D constraints{RigidbodyConstraints2D::None};

        Vec2 velocity{0.0f, 0.0f};
        float angularVelocity{0.0f};

        Rigidbody2D();
        Rigidbody2D(const std::string &name);
        Rigidbody2D(const Rigidbody2D &) = delete;
        ~Rigidbody2D();

        void OnAwake();
        void OnDestroy();

        bool isKinematic() const { return bodyType == BodyType2D::Kinematic; }
        void setKinematic(bool value) { bodyType = value ? BodyType2D::Kinematic : BodyType2D::Dynamic; }

        bool freezeRotation() const
        {
            return (static_cast<int>(constraints) & static_cast<int>(RigidbodyConstraints2D::FreezeRotation)) != 0;
        }

        void setFreezeRotation(bool freeze)
        {
            if (freeze)
                constraints = static_cast<RigidbodyConstraints2D>(static_cast<int>(constraints) | static_cast<int>(RigidbodyConstraints2D::FreezeRotation));
            else
                constraints = static_cast<RigidbodyConstraints2D>(static_cast<int>(constraints) & ~static_cast<int>(RigidbodyConstraints2D::FreezeRotation));
        }

        void AddForce(Vec2 force);
        void AddTorque(float torque);
        void AddImpulse(Vec2 impulse);

        void Reset();

        box2d::Body *getInternalBody() const { return m_internalBody; }
        void setInternalBody(box2d::Body *body) { m_internalBody = body; }
    };
} // namespace RoninEngine::Runtime
