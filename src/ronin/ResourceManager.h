#pragma once

#include "dependency.h"

namespace RoninEngine {
namespace Runtime {

constexpr int GCInvalidID = 0xffffffff;
constexpr std::uint8_t SDL_TYPE_MAX_INDEX = 31;
constexpr std::uint8_t InvalidType = 0xff;

// any type get indexer
template <typename, std::uint8_t>
struct type2index_low;

template <typename T, std::uint8_t _Index>
struct type2index_low {
    typedef T type;
    static constexpr std::uint8_t typeIndex = _Index;
};

template <typename>
struct type2index;

template <>
struct type2index<void> : public type2index_low<void, 0> {  // Any pointer
};
template <>
struct type2index<SDL_Texture> : public type2index_low<SDL_Texture, 1> {  // SDL Texture
};
template <>
struct type2index<SDL_Surface> : public type2index_low<SDL_Surface, 2> {  // SDL Surface
};
template <>
struct type2index<Texture> : public type2index_low<Texture, 32> {  // Ronin Texture
};
template <>
struct type2index<Sprite> : public type2index_low<Sprite, 33> {  // Ronin Sprite
};

/// Object type index
template <typename T>
struct type2index : public type2index_low<std::enable_if_t<std::is_base_of_v<Object, T>, T>, 128> {  // Ronin Object
};

// TODO: indexToType is not completed (impl)
template <std::uint8_t, typename J = void>
struct index2type {};

template <typename T>
struct is_sdl_type : public std::integral_constant<bool, type2index<T>::typeIndex <= SDL_TYPE_MAX_INDEX> {};
template <typename T>
struct is_invalid_type : public std::integral_constant<bool, type2index<T>::typeIndex == InvalidType> {};

struct GCMemoryStick {
    int memoryType;
    void *memory;
};

class GC {
    friend class ::RoninEngine::Application;
    friend class Scene;

   protected:
    static void gc_init();
    static void gc_free();
    static void gc_free_source();
    static void gc_collect();

    static void UnloadAll(bool immediate);
    static void CheckResources();

    static void suspend_gc();
    static void continue_gc();

   public:
    [[deprecated]] static void LoadImages(const char *filename);
    [[deprecated]] static void UnloadUnused();

    [[deprecated]] static std::list<SDL_Surface *> *LoadSurfaces(const std::string &packName);
    [[deprecated]] static std::list<Runtime::Texture *> *LoadTextures(const std::string &packName, bool autoUnload = true);
    [[deprecated]] static SDL_Surface *GetSurface(const std::string &surfaceName);
    [[deprecated]] static SDL_Surface *GetSurface(const std::string &surfaceName, FolderKind pathOn);
    [[deprecated]] static Texture *GetTexture(const std::string &resourceName, FolderKind pathOn, bool autoUnload = true);
    [[deprecated]] static Texture *GetTexture(const std::string &resourceName, bool autoUnload = true);
    [[deprecated]] static Texture *GetTexture(const int w, const int h);
    [[deprecated]] static Texture *GetTexture(const int w, const int h, const ::SDL_PixelFormatEnum format);
    [[deprecated]] static Texture *GetTexture(const int w, const int h, const ::SDL_PixelFormatEnum format,
                                             const ::SDL_TextureAccess access);
    [[deprecated]] static SDL_Cursor *GetCursor(const std::string &resourceName, const Vec2Int &hotspot, bool autoUnload = true);
    [[deprecated]] static SDL_Cursor *GetCursor(SDL_Surface *texture, const Vec2Int &hotspot);

    [[deprecated]] static Atlas *GetAtlas(const std::string &atlasName);

    static void *gc_malloc(std::size_t size);

    template <typename T, typename... Args>
    static T *&gc_push_lvalue(T *&lvalue_pointer, Args &&...arguments) {
        return lvalue_pointer = reinterpret_cast<T *>(gc_push<T>(arguments...));
    }

    template <typename T, typename... Args>
    static typename std::enable_if<std::is_base_of<Object, T>::value, T *>::type gc_push(Args &&..._Args);

    template <typename T>
    static bool valid_type();

    static void gc_free(void *memory);

    static void *gc_realloc(void *mem, std::size_t size);

    static void gc_lock();

    static void gc_unlock();

    static bool gc_is_lock();

    /// Unload from low level
    static bool gc_unload(int id);

    template <typename T>
    static typename std::enable_if<std::is_base_of<Object, T>::value, bool>::type gc_unload(T *pointer);

    template <typename T, typename... Args>
    static T *&gc_alloc_lval(T *&lval, Args &&..._Args) {
        return lval = gc_alloc<T>(std::forward<Args &&>(_Args)...);
    }

    template <typename T, typename... Args>
    static T *gc_alloc(Args &&..._Args);

    template <typename T>
    static void gc_unalloc(T *p);

    static int get_id(void *ptr);

    static Texture *gc_get_texture(int id);

    static SDL_Texture *gc_get_sdl_texture(int id);

    static SDL_Surface *gc_get_sdl_surface(int id);

    static const Sprite *gc_get_sprite(int id);

    static bool gc_is_null(int id);

    static std::tuple<int, int> gc_countn();

    static int resource_bitmap(const std::string &resourceName, FolderKind folderKind, SDL_Surface **sdlsurfacePtr);

    static SDL_Surface *resource_bitmap(const std::string &resourceName, FolderKind folderKind);

    static int gc_alloc_sdl_surface(SDL_Surface **sdlsurfacePtr, const int &w, const int &h);

    static int gc_alloc_sdl_surface(SDL_Surface **sdlsurfacePtr, const int &w, const int &h, const SDL_PixelFormatEnum &format);

    static int gc_alloc_sdl_texture(SDL_Texture **sdltexturePtr, const int &w, const int &h);

    static int gc_alloc_sdl_texture(SDL_Texture **sdltexturePtr, const int &w, const int &h, const SDL_PixelFormatEnum &format);

    static int gc_alloc_sdl_texture(SDL_Texture **sdltexturePtr, const int &w, const int &h, const SDL_PixelFormatEnum &format,
                                    const SDL_TextureAccess &access);

    static int gc_alloc_sdl_texture(SDL_Texture **sdltexturePtr, SDL_Surface *from);

    /// Create Texture with 32x32 pixels, RGBA8888 and access as STATIC
    static int gc_alloc_texture_empty(Texture **texturePtr);

    /// Return allocated id
    static int gc_alloc_texture(Texture **texturePtr, const int &w, const int &h);

    static int gc_alloc_texture(Texture **texturePtr, const int &w, const int &h, const SDL_PixelFormatEnum &format);
    static int gc_alloc_texture(Texture **texturePtr, const int &w, const int &h, const SDL_PixelFormatEnum &format,
                                const SDL_TextureAccess &access);

    static int gc_alloc_texture_from(Texture **texturePtr, SDL_Surface *sdlsurface);

    static int gc_alloc_texture_from(Texture **texturePtr, SDL_Texture *sdltexture);

    /// Create empty Sprite
    static int gc_alloc_sprite_empty(Sprite **spritePtr);

    static int gc_alloc_sprite_empty(Sprite **spritePtr, const Rect &rect);

    static int gc_alloc_sprite_with(Sprite **spritePtr, Texture *texture);

    static int gc_alloc_sprite_with(Sprite **spritePtr, Texture *texture, const Vec2 &center);

    static int gc_alloc_sprite_with(Sprite **spritePtr, Texture *texture, const Rect &rect, const Vec2 &center = Vec2::half);

    static int gc_alloc_cursor(SDL_Cursor **cursorPtr, SDL_Surface *surface, int hotspot_x, int hotspot_y);

    static std::size_t gc_total_allocated();
};

template <typename T, typename... Args>
constexpr T *_paste_oop_init(T *m, Args &&...args) {
    return new (m) T(std::forward<Args &&>(args)...);
}

template <typename T>
constexpr T *_cut_oop_from(T *m) {
    m->~T();
    return m;
}

template <typename T, typename... Args>
typename std::enable_if<std::is_base_of<Object, T>::value, T *>::type GC::gc_push(Args &&..._Args) {
    extern int gc_write_memblock_runtime(GCMemoryStick * *ms, const std::uint8_t &typeIndex, const std::size_t size);

    GCMemoryStick *ms;
    int id;
    T *mem;

    id = gc_write_memblock_runtime(&ms, type2index<T>::typeIndex, sizeof(T));
    if (id == GCInvalidID) throw std::bad_alloc();
    mem = reinterpret_cast<T *>(ms->memory);
    _paste_oop_init(mem, std::forward<Args>(_Args)...);
    return mem;  // result
}

template <typename T>
typename std::enable_if<std::is_base_of<Object, T>::value, bool>::type GC::gc_unload(T *pointer) {
    extern int gc_native_collect(const int freeID);
    int id;
    int released = 0;
    id = get_id(pointer);
    if (id != GCInvalidID) released = gc_native_collect(id);

    return released != 0;
}

template <typename T, typename... Args>
T *GC::gc_alloc(Args &&..._Args) {
    T *p = reinterpret_cast<T *>(gc_malloc(sizeof(T)));
    if (p == nullptr) throw std::bad_alloc();
    memset(p, 0, sizeof(T));
    _paste_oop_init(p, std::forward<Args &&>(_Args)...);
    return p;
}

template <typename T>
void GC::gc_unalloc(T *p) {
    if (p == nullptr) throw std::invalid_argument("p");

    _cut_oop_from(p);
    gc_free(p);
}

}  // namespace Runtime
}  // namespace RoninEngine
