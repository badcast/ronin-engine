#include "ronin.h"
#include "Collision.h"

namespace RoninEngine::Runtime
{

    // TODO: Optimize here
    bool CheckCollision(Rectf rectA, float angleRadA, Rectf rectB, float angleRadB)
    {
        Vec2 cA {rectA.x + rectA.w / 2.0f, rectA.y + rectA.h / 2.0f};
        Vec2 cB {rectB.x + rectB.w / 2.0f, rectB.y + rectB.h / 2.0f};
        Vec2 eA {rectA.w / 2.0f, rectA.h / 2.0f};
        Vec2 eB {rectB.w / 2.0f, rectB.h / 2.0f};

        Vec2 uAx {Math::Cos(angleRadA), Math::Sin(angleRadA)};
        Vec2 uAy {-Math::Sin(angleRadA), Math::Cos(angleRadA)};
        Vec2 uBx {Math::Cos(angleRadB), Math::Sin(angleRadB)};
        Vec2 uBy {-Math::Sin(angleRadB), Math::Cos(angleRadB)};

        Vec2 d = cB - cA;

        // Axis uAx
        float rA = eA.x;
        float rB = eB.x * std::abs(Vec2::Dot(uBx, uAx)) + eB.y * std::abs(Vec2::Dot(uBy, uAx));
        if(std::abs(Vec2::Dot(d, uAx)) > rA + rB)
            return false;

        // Axis uAy
        rA = eA.y;
        rB = eB.x * std::abs(Vec2::Dot(uBx, uAy)) + eB.y * std::abs(Vec2::Dot(uBy, uAy));
        if(std::abs(Vec2::Dot(d, uAy)) > rA + rB)
            return false;

        // Axis uBx
        rA = eA.x * std::abs(Vec2::Dot(uAx, uBx)) + eA.y * std::abs(Vec2::Dot(uAy, uBx));
        rB = eB.x;
        if(std::abs(Vec2::Dot(d, uBx)) > rA + rB)
            return false;

        // Axis uBy
        rA = eA.x * std::abs(Vec2::Dot(uAx, uBy)) + eA.y * std::abs(Vec2::Dot(uAy, uBy));
        rB = eB.y;
        if(std::abs(Vec2::Dot(d, uBy)) > rA + rB)
            return false;

        return true;
    }

    bool Collision::CheckCollision(Rectf rectA, float angleRadA, Rectf rectB, float angleRadB)
    {
        return RoninEngine::Runtime::CheckCollision(rectA, angleRadA, rectB, angleRadB);
    }

    Collision::Collision() : Collision("Collision")
    {
    }

    Collision::Collision(const std::string &name) : Behaviour(name), targetLayer(-1)
    {
    }

    Collision::Collision(const Collision &other) : Behaviour(other.m_name), targetLayer(other.targetLayer), collideSize(other.collideSize)
    {
    }

    bool Collision::setSizeFrom(SpriteRendererRef spriteRenderer)
    {
        bool result;

        if((result = (spriteRenderer && spriteRenderer->getSprite())))
            collideSize = Vec2::Scale(spriteRenderer->getSprite()->size(), spriteRenderer->getSize());

        return result;
    }

    void Collision::OnAwake()
    {
        // Get collide size from SpriteRenderer::size() if collideSize is empty
        if(collideSize != Vec2::zero)
            return;

        // Get size from SpriteRenderer
        if(!setSizeFrom(GetComponent<SpriteRenderer>()))
            collideSize = Vec2::one;
    }

    void Collision::OnUpdate()
    {
        if(!onCollision)
            return;

        std::vector<Transform *> casts = Physics2D::GetCircleCast<std::vector<Transform *>>(transform()->position(), (collideSize.x + collideSize.y) * 2, targetLayer);

        Rectf r1, r2;
        float angleLhs, angleRhs;
        std::list<CollisionRef> targets;

        angleLhs = transform()->angle() * Math::deg2rad;
        r1.w = collideSize.x;
        r1.h = collideSize.y;
        r1.x = transform()->position().x - r1.w / 2;
        r1.y = transform()->position().y - r1.h / 2;

        for(Transform *cast : casts)
        {
            if(!cast || cast == transform().ptr_ || !cast->_owner)
                continue;

            targets = cast->GetComponents<Collision>();
            for(CollisionRef &target : targets)
            {
                if(!target || target.ptr_ == this)
                    continue;

                angleRhs = target->transform()->angle() * Math::deg2rad;
                r2.w = target->collideSize.x;
                r2.h = target->collideSize.y;
                r2.x = target->transform()->position().x - r2.w / 2;
                r2.y = target->transform()->position().y - r2.h / 2;
                if(CheckCollision(r1, angleLhs, r2, angleRhs))
                {
                    if(!onCollision(this->GetRef<Collision>(), target)) // awake collision
                    {
                        cast = nullptr;
                        break;
                    }
                }
            }
            if(cast == nullptr)
                break;
        }
    }

    void Collision::OnGizmos()
    {
        RenderUtility::SetColor(Color::lime);
        RenderUtility::DrawRectangleRotate(transform()->position(), collideSize, transform()->angle() * Math::deg2rad);
    }
} // namespace RoninEngine::Runtime
