/*
   This is file describe type
*/

#pragma once

#include "ronin.h"

static std::unordered_map<std::type_index, std::string> main_ronin_types;

#define DESCRIBE_TYPE(type) (main_ronin_types[std::type_index(typeid(type))] = #type)

namespace RoninEngine::Runtime
{
    struct Render_info {
        SDL_Renderer* renderer;
        Rect src;
        Rectf_t dst;
        SDL_Texture* texture;
    };
}
