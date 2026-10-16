#include "ronin.h"
#include "ronin_matrix.h"

namespace RoninEngine::Runtime
{
    void Physics2D::SimulateStorm(Vec2Int origin, int edges, std::function<void(const Vec2Int &)> predicate, bool fillInside)
    {
        if(edges < 0 || !predicate)
            return;
        int x = 0, y = 0, dx = 0, dy = -1;
        int side = 2 * edges + 1;
        int maxI = side * side;
        for(int i = 0; i < maxI; i++)
        {
            if (fillInside)
                predicate(origin + Vec2Int {x, y});
            else if(
                ((x == -edges || x == edges) && (y >= -edges && y <= edges)) || 
                ((y == -edges || y == edges) && (x >= -edges && x <= edges)))
            {
                predicate(origin + Vec2Int {x, y});
            }

            if(x == y || (x < 0 && x == -y) || (x > 0 && x == 1 - y))
            {
                int t = dx;
                dx = -dy;
                dy = t;
            }
            x += dx;
            y += dy;
        }
    }


    template <typename container_result>
    container_result Physics2D::GetStormCast(Vec2 origin, int edges, int layer)
    {
        float size = edges * 2.f; 
        return GetRectangleCast<container_result>(origin, Vec2(size, size), layer);
    }

    template <typename container_result>
    container_result Physics2D::GetRectangleCast(Vec2 center, Vec2 size, int layer)
    {
        container_result result;
        if(!currentWorld || !currentWorld->irs)
            return result;
        size = Vec2::Abs(size);
        Rectf rect(center.x - size.x / 2.f, center.y - size.y / 2.f, size.x, size.y);

        std::set<Transform*> intersects;
        for(auto &findedIter : currentWorld->irs->matrix)
        {
            if(findedIter.second && (layer == -1 || (findedIter.first & layer) || findedIter.first == layer))
            {
                findedIter.second->Retrieve(intersects, rect);
            }
        }
        
        for (auto obj : intersects)
        {
            if(!obj)
                continue;
            if constexpr(std::is_same<container_result, std::set<Transform *>>::value)
                result.insert(obj);
            else
                result.emplace_back(obj);
        }

        return result;
    }

    template <typename container_result>
    container_result Physics2D::GetCircleCast(Vec2 origin, float distance, int layer)
    {
        container_result result;
        container_result candidates = GetRectangleCast<container_result>(origin, Vec2(distance * 2.f, distance * 2.f), layer);
        
        for (auto& obj : candidates)
        {
            if (obj && Vec2::Distance(origin, obj->position()) <= distance)
            {
                if constexpr(std::is_same<container_result, std::set<Transform *>>::value)
                    result.insert(obj);
                else
                    result.emplace_back(obj);
            }
        }
        return result;
    }
    template RONIN_API std::list<Transform *> Physics2D::GetStormCast(Vec2, int, int);
    template RONIN_API std::list<Transform *> Physics2D::GetRectangleCast(Vec2, Vec2, int);
    template RONIN_API std::list<Transform *> Physics2D::GetCircleCast(Vec2, float, int);

    template RONIN_API std::vector<Transform *> Physics2D::GetStormCast(Vec2, int, int);
    template RONIN_API std::vector<Transform *> Physics2D::GetRectangleCast(Vec2, Vec2, int);
    template RONIN_API std::vector<Transform *> Physics2D::GetCircleCast(Vec2, float, int);

    template RONIN_API std::set<Transform *> Physics2D::GetStormCast(Vec2, int, int);
    template RONIN_API std::set<Transform *> Physics2D::GetRectangleCast(Vec2, Vec2, int);
    template RONIN_API std::set<Transform *> Physics2D::GetCircleCast(Vec2, float, int);

    bool Physics2D::CheckCollision(Rectf rectA, float angleRadA, Rectf rectB, float angleRadB)
    {
        return Collision::CheckCollision(rectA, angleRadA, rectB, angleRadB);
    }

    bool Physics2D::CheckCollision(Transform *a, Vec2 sizeA, Transform *b, Vec2 sizeB)
    {
        if(!a || !b)
            return false;
        Rectf rA { a->position().x - sizeA.x / 2.0f, a->position().y - sizeA.y / 2.0f, sizeA.x, sizeA.y };
        Rectf rB { b->position().x - sizeB.x / 2.0f, b->position().y - sizeB.y / 2.0f, sizeB.x, sizeB.y };
        return Collision::CheckCollision(rA, a->angle() * Math::deg2rad, rB, b->angle() * Math::deg2rad);
    }

} // namespace RoninEngine::Runtime

#undef MX
