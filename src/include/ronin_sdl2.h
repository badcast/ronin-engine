/*
    This is header for override native classes and structures
*/

#pragma once

// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "../3rdparty/SDL2_gfx/SDL2_gfxPrimitives.h"

typedef SDL_Surface Image;            // SDL_Surface
typedef SDL_Cursor Cursor;            // SDL_Cursor
typedef SDL_Texture native_texture_t; // SDL_Texture
typedef SDL_Rect native_rect_t;       // SDL_Rect
typedef SDL_FRect native_frect_t;     // SDL_FRect
typedef SDL_Color native_color_t;     // SDL_Color
