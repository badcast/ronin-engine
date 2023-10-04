#pragma once
#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API MoveController2D : public Behaviour
    {
    public:
        Camera2D *playerCamera;

        MoveController2D();
        MoveController2D(const std::string &name);
        float speed;

        void OnAwake();
        void OnUpdate();
    };
} // namespace RoninEngine::Runtime
