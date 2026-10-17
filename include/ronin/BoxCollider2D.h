#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API BoxCollider2D : public Behaviour
    {
    public:
        Vec2 size{1.0f, 1.0f};
        Vec2 offset{0.0f, 0.0f};
        bool isTrigger{false};
        float density{1.0f};
        float friction{0.4f};
        float bounciness{0.1f};

        BoxCollider2D();
        BoxCollider2D(const std::string &name);
        BoxCollider2D(const BoxCollider2D &) = delete;

        void OnAwake();
        void Reset();

        static bool CheckCollision(
            const BoxCollider2D &a,
            Transform *transA,
            const BoxCollider2D &b,
            Transform *transB);
    };
} // namespace RoninEngine::Runtime
