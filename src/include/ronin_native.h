/*
    This is header for override native classes and structures
*/

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

typedef SDL_Surface native_surface_t; // SDL_Surface
typedef SDL_Texture native_texture_t; // SDL_Texture
typedef SDL_Rect native_rect_t; // SDL_Rect
typedef SDL_FRect native_frect_t; // SDL_FRect
typedef SDL_Color native_color_t; // SDL_Color

// template <typename NativeType>
// constexpr inline auto native_to(NativeType*& native)
//{
//     if constexpr (std::is_same_v<NativeType, native_surface_t>)
//     {
//         return reinterpret_cast<SDL_Surface*>(native);
//     }
//     else if constexpr (std::is_same_v<NativeType, native_texture_t>)
//     {
//         return reinterpret_cast<SDL_Texture*>(native);
//     }
//     else
//     {
//         return static_cast<typename std::remove_pointer<NativeType>::type*>(native);
//     }
// }

// template <typename BasicType>
// constexpr auto native_from(BasicType*& src)
//{
//     if constexpr (std::is_same_v<BasicType, SDL_Surface>)
//     {
//         return reinterpret_cast<native_surface_t*>(src);
//     }
//     else if constexpr (std::is_same_v<BasicType, SDL_Texture>)
//     {
//         return reinterpret_cast<native_texture_t*>(src);
//     }
//     else
//     {
//         return static_cast<typename std::remove_pointer<BasicType>::type*>(src);
//     }
// }
