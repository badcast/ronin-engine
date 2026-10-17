#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API CircleCollider2D : public Behaviour
    {
    public:
        float radius{0.5f};
        Vec2 offset{0.0f, 0.0f};
        bool isTrigger{false};
        float density{1.0f};
        float friction{0.4f};
        float bounciness{0.1f};

        CircleCollider2D();
        CircleCollider2D(const std::string &name);
        CircleCollider2D(const CircleCollider2D &) = delete;

        void OnAwake();
        void Reset();
    };
} // namespace RoninEngine::Runtime
