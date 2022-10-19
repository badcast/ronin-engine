#pragma once

#include "dependency.h"

namespace RoninEngine::Runtime {

   class Atlas {
         friend GC;
         Texture* texture;
         std::map<std::string, Sprite> _sprites;

      public:
         Atlas() = default;
         Atlas(const Atlas&) = default;
         ~Atlas() = default;

         Sprite* Get_Sprite(const std::string& spriteName);
   };

}  // namespace RoninEngine::Runtime
