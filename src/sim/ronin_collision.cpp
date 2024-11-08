#include "ronin.h"
#include "Collision.h"

namespace RoninEngine::Runtime
{

    // TODO: Optimize here
    bool CheckCollision(Rectf rectA, float angleA, Rectf rectB, float angleB)
    {
        float angleRadA = angleA * (float) Math::Pi / 180.0f;
        float angleRadB = angleB * (float) Math::Pi / 180.0f;

        float centerX_A = rectA.x + rectA.w / 2.0f;
        float centerY_A = rectA.y + rectA.h / 2.0f;
        float centerX_B = rectB.x + rectB.w / 2.0f;
        float centerY_B = rectB.y + rectB.h / 2.0f;

        float sinB = Math::Sin(angleRadB);
        float cosB = Math::Cos(angleRadB);

        float xA_inB = cosB * (centerX_A - centerX_B) + sinB * (centerY_A - centerY_B) + centerX_B;
        float yA_inB = -sinB * (centerX_A - centerX_B) + cosB * (centerY_A - centerY_B) + centerY_B;

        if(xA_inB < rectB.x || xA_inB > rectB.x + rectB.w || yA_inB < rectB.y || yA_inB > rectB.y + rectB.h)
        {
            return false;
        }

        // Если не произошло коллизии в выровненных координатах, то есть коллизия в оригинальных координатах
        return true;
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
            targets = cast->GetComponents<Collision>();
            for(CollisionRef &target : targets)
            {
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
        return;
        RenderUtility::SetColor(Color::lime);
        RenderUtility::DrawRectangleRotate(transform()->position(), collideSize, transform()->angle() * Math::deg2rad);
    }
} // namespace RoninEngine::Runtime
