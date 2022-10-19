#pragma once

#include "dependency.h"
#include "Component.h"

namespace RoninEngine::Runtime {

   class Behaviour :
         public Component
   {
      public:
         Behaviour() : Behaviour(typeid(*this).name()) {}
         Behaviour(const std::string& name) : Component(name) {}
         Behaviour(const Behaviour&) = delete;
         Behaviour(Behaviour&&) = delete;
         virtual ~Behaviour() = default;

         virtual void OnAwake() {}
         virtual void OnStart() {}
         virtual void OnUpdate() {}
         virtual void OnLateUpdate() {}
         virtual void OnGizmos() {}
         virtual void OnDestroy() {}
   };
}

