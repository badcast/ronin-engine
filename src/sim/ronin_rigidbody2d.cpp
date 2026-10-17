#include "ronin.h"
#include "Rigidbody2D.h"
#include "box2d/b2_body.h"

namespace RoninEngine::Runtime
{
    Rigidbody2D::Rigidbody2D() : Rigidbody2D("Rigidbody2D")
    {
    }

    Rigidbody2D::Rigidbody2D(const std::string &name) : Behaviour(name)
    {
        Reset();
    }

    Rigidbody2D::~Rigidbody2D()
    {
        OnDestroy();
    }

    void Rigidbody2D::OnAwake()
    {
    }

    void Rigidbody2D::OnDestroy()
    {
        m_internalBody = nullptr;
    }

    void Rigidbody2D::AddForce(Vec2 force)
    {
        if (m_internalBody)
        {
            m_internalBody->AddForce(box2d::Vec2(force.x, force.y));
        }
    }

    void Rigidbody2D::AddTorque(float torque)
    {
        if (m_internalBody)
        {
            m_internalBody->AddTorque(torque);
        }
    }

    void Rigidbody2D::AddImpulse(Vec2 impulse)
    {
        velocity += impulse * (mass > 0.0f ? (1.0f / mass) : 0.0f);
        if (m_internalBody)
        {
            m_internalBody->AddImpulse(box2d::Vec2(impulse.x, impulse.y));
        }
    }

    void Rigidbody2D::Reset()
    {
        bodyType = BodyType2D::Dynamic;
        mass = 1.0f;
        linearDamping = 0.0f;
        angularDamping = 0.05f;
        gravityScale = 1.0f;
        simulated = true;
        useAutoMass = false;
        constraints = RigidbodyConstraints2D::None;
        velocity = Vec2::zero;
        angularVelocity = 0.0f;

        if (m_internalBody)
        {
            m_internalBody->velocity.Set(0.0f, 0.0f);
            m_internalBody->angularVelocity = 0.0f;
            m_internalBody->SetBodyType(box2d::BodyType::Dynamic);
        }
    }
} // namespace RoninEngine::Runtime
