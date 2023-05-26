#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API MoveController2D : public Behaviour
    {
    public:
        Camera2D* playerCamera;
        SpriteRenderer* spriteRenderer;
        Spotlight* spotlight;

        MoveController2D();
        MoveController2D(const std::string& name);
        float speed;

        void OnAwake();
        void OnStart();
        void OnUpdate();
        void OnLateUpdate();
        void OnDestroy();
    };
} // namespace RoninEngine::Runtime
