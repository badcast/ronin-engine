#pragma once

#include "Declarations.h"
#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Behaviour : public Component
    {
        friend class GameObject;

    public:
        Behaviour();

        Behaviour(const std::string &name);
        Behaviour(const Behaviour &) = delete;
        Behaviour(Behaviour &&) = delete;
        virtual ~Behaviour() = default;

        virtual void OnAwake();
        virtual void OnStart();
        virtual void OnUpdate();
        virtual void OnLateUpdate();
        virtual void OnGizmos();
        virtual void OnDestroy();
    };

} // namespace RoninEngine::Runtime
