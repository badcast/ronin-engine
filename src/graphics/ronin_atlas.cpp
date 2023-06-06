#include "ronin.h"

namespace RoninEngine::Runtime
{
    Sprite* Atlas::get_sprite(const std::string& spriteName)
    {
        auto iter = _sprites.find(spriteName);
        if (iter == end(_sprites))
            return nullptr;
        return &iter->second;
    }
}
