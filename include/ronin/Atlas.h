#pragma once

#include "begin.h"

namespace RoninEngine::Runtime {

   class RONIN_API Atlas {
         friend ResourceManager;
         Texture* texture;
         std::map<std::string, Sprite> _sprites;

      public:
         Atlas() = default;
         Atlas(const Atlas&) = default;
         ~Atlas() = default;

         Sprite* get_sprite(const std::string& spriteName);
   };

}  // namespace RoninEngine::Runtime
