#pragma once

#include "Object.h"
#include "dependency.h"

namespace  RoninEngine {

   namespace Runtime {

      class SHARK Component : public Object {
         private:
            friend GameObject;
            friend GameObject* Instantiate(GameObject* obj);

            GameObject* pin;

         public:
            bool enabled();
            Component();
            explicit Component(const std::string& name);
            Component(const Component&) = delete;
            virtual ~Component() = default;

            const bool isBind();
            GameObject* gameObject();
            Transform* transform();
      };
   }  // namespace Runtime
}  // namespace RoninEngine
