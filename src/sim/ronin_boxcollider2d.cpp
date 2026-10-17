#include "ronin.h"
#include "BoxCollider2D.h"
#include "Collision.h"

namespace RoninEngine::Runtime
{
    BoxCollider2D::BoxCollider2D() : BoxCollider2D("BoxCollider2D")
    {
    }

    BoxCollider2D::BoxCollider2D(const std::string &name) : Behaviour(name)
    {
        Reset();
    }

    void BoxCollider2D::OnAwake()
    {
    }

    void BoxCollider2D::Reset()
    {
        size = Vec2::one;
        offset = Vec2::zero;
        isTrigger = false;
        density = 1.0f;
        friction = 0.4f;
        bounciness = 0.1f;
    }

    bool BoxCollider2D::CheckCollision(
        const BoxCollider2D &a,
        Transform *transA,
        const BoxCollider2D &b,
        Transform *transB)
    {
        if (!transA || !transB)
            return false;

        Vec2 posA = transA->position() + a.offset;
        Vec2 posB = transB->position() + b.offset;
        float angA = transA->angle() * Math::deg2rad;
        float angB = transB->angle() * Math::deg2rad;

        Rectf rectA(posA.x - a.size.x * 0.5f, posA.y - a.size.y * 0.5f, a.size.x, a.size.y);
        Rectf rectB(posB.x - b.size.x * 0.5f, posB.y - b.size.y * 0.5f, b.size.x, b.size.y);

        return Collision::CheckCollision(rectA, angA, rectB, angB);
    }
} // namespace RoninEngine::Runtime
