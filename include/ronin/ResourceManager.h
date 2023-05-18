#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {

        enum { GCInvalidID = 0xffffffff };

        class RONIN_API ResourceManager
        {
            friend class RoninEngine::Application;
            friend class Scene;
            static void gc_release();

        public:
            [[deprecated]] static void LoadImages(const char* filename);

            [[deprecated]] static std::list<SDL_Surface*>* LoadSurfaces(const std::string& packName);
            [[deprecated]] static std::list<Runtime::Texture*>* LoadTextures(const std::string& packName, bool autoUnload = true);
            [[deprecated]] static SDL_Surface* GetSurface(const std::string& surfaceName);
            [[deprecated]] static Texture* GetTexture(const std::string& resourceName, bool autoUnload = true);
            [[deprecated]] static Texture* GetTexture(const int w, const int h);
            [[deprecated]] static Texture* GetTexture(const int w, const int h, const ::SDL_PixelFormatEnum format);
            [[deprecated]] static Texture* GetTexture(const int w, const int h, const ::SDL_PixelFormatEnum format, const ::SDL_TextureAccess access);
            [[deprecated]] static SDL_Cursor* GetCursor(const std::string& resourceName, const Vec2Int& hotspot, bool autoUnload = true);
            [[deprecated]] static SDL_Cursor* GetCursor(SDL_Surface* texture, const Vec2Int& hotspot);

            static int resource_bitmap(const std::string& resourceName, SDL_Surface** sdlsurfacePtr);

            static SDL_Surface* resource_bitmap(const std::string& resourceName);

            static int gc_alloc_sdl_surface(SDL_Surface** sdlsurfacePtr, const int& w, const int& h);

            static int gc_alloc_sdl_surface(SDL_Surface** sdlsurfacePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format);

            static int gc_alloc_sdl_texture(SDL_Texture** sdltexturePtr, const int& w, const int& h);

            static int gc_alloc_sdl_texture(SDL_Texture** sdltexturePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format);

            static int gc_alloc_sdl_texture(SDL_Texture** sdltexturePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format, const SDL_TextureAccess& access);

            static int gc_alloc_sdl_texture(SDL_Texture** sdltexturePtr, SDL_Surface* from);

            /// Create Texture with 32x32 pixels, RGBA8888 and access as STATIC
            static int gc_alloc_texture_empty(Texture** texturePtr);

            static int gc_alloc_texture(Texture** texturePtr, const int& w, const int& h);

            static int gc_alloc_texture(Texture** texturePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format);
            static int gc_alloc_texture(Texture** texturePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format, const SDL_TextureAccess& access);

            static int gc_alloc_texture_from(Texture** texturePtr, SDL_Surface* sdlsurface);

            static int gc_alloc_texture_from(Texture** texturePtr, SDL_Texture* sdltexture);

            static int gc_alloc_sprite_empty(Sprite** spritePtr);

            static int gc_alloc_sprite_empty(Sprite** spritePtr, const Rect& rect);

            static int gc_alloc_sprite_with(Sprite** spritePtr, SDL_Surface* src);

            static int gc_alloc_sprite_with(Sprite** spritePtr, SDL_Surface* src, const Vec2& center);

            static int gc_alloc_sprite_with(Sprite** spritePtr, SDL_Surface* src, const Rect& rect, const Vec2& center = Vec2::half);

            static int gc_alloc_cursor(SDL_Cursor** cursorPtr, SDL_Surface* src, int hotspot_x, int hotspot_y);
        };
    } // namespace Runtime
} // namespace RoninEngine
