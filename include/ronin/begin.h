#pragma once

// standard include
#include <cstdint>
#include <string>
#include <functional>
#include <limits>

// standard listing include
#include <list>
#include <vector>

#ifndef RONIN_COMPILLING
struct SDL_Surface;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_DisplayMode;
struct SDL_Color
{
    std::uint8_t r, g, b, a;
};
struct SDL_Rect
{
    int x, y, w, h;
};

struct SDL_FRect
{
    float x, y, w, h;
};

typedef int SDL_BlendMode, SDL_TextureAccess, SDL_ScaleMode, SDL_Scancode, SDL_PixelFormatEnum;
#endif

#include "Defines.h"

#include "Namespaces.h"

#include "RoninMemory.h"

#include "Color.h"
#include "Vec2.h"

#include "Types.h"
