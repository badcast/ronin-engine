#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Behaviour : public Component
    {
    public:
        Behaviour();

        Behaviour(const std::string &name);
        Behaviour(const Behaviour &) = delete;
        Behaviour(Behaviour &&) = delete;
        virtual ~Behaviour() = default;

        virtual void OnAwake()
        {
        }
        virtual void OnStart()
        {
        }
        virtual void OnUpdate()
        {
        }
        virtual void OnLateUpdate()
        {
        }
        virtual void OnGizmos()
        {
        }
        virtual void OnDestroy()
        {
        }
    };
} // namespace RoninEngine::Runtime
