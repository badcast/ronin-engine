#pragma once

#include "Object.h"

namespace  RoninEngine {

   namespace Runtime {

      class SHARK Component : public Object {
         private:
            friend class GameObject;
            friend GameObject* Instantiate(GameObject* obj);

            GameObject* _owner;

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