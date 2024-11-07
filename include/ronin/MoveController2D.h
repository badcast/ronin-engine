#pragma once
#include "Behaviour.h"

namespace RoninEngine::Runtime
{
    class RONIN_API MoveController2D : public Behaviour
    {
    public:
        float speed;
        Camera2DRef playerCamera;

        MoveController2D();
        MoveController2D(const std::string &name);

        void OnAwake();
        void OnUpdate();
    };
} // namespace RoninEngine::Runtime
