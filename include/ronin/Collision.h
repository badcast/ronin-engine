#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Collision : public Behaviour
    {
    public:
        void OnAwake();
        void OnUpdate();
        void OnGizmos();

    public:
        int targetLayer;
        Vec2 collideSize;
        std::function<bool(CollisionRef self, CollisionRef target)> onCollision;

        Collision();
        Collision(const std::string &name);
        Collision(const Collision &);

        bool setSizeFrom(SpriteRendererRef spriteRenderer);
    };
} // namespace RoninEngine::Runtime
