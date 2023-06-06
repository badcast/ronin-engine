#include <filesystem>

#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::Runtime::RoninMemory;
using namespace RoninEngine::UI;
using namespace RoninEngine::AIPathFinder;
using namespace jno;

// ResourceManager::ResourceManager () { }

std::map<int, std::list<std::string>>* _assocMultiFiles;
std::map<int, std::list<SDL_Surface*>*>* _assocMultiLoadedImages;
std::map<std::list<SDL_Surface*>*, std::list<Texture*>*>* _assocMultiCacheTextures;

std::map<int, std::string>* _assocSingleFile;
std::map<int, SDL_Surface*>* _resourceLoaded_surfaces;
std::map<SDL_Surface*, Texture*>* _assocCacheTextures;
std::map<SDL_Surface*, SDL_Cursor*>* _assocCacheCursors;

std::map<std::string, Mix_Music*>* _assocLoadedMusic;

namespace RoninEngine::Runtime
{
    void manager_init()
    {
        constexpr int bufferSize = sizeof(*_assocMultiFiles) + sizeof(*_assocMultiLoadedImages) + sizeof(*_assocMultiCacheTextures) + sizeof(*_assocSingleFile) + sizeof(*_resourceLoaded_surfaces) + sizeof(*_assocCacheTextures)
            + sizeof(*_assocCacheCursors) + sizeof(*_assocLoadedMusic);

        void* buffer = std::malloc(bufferSize);
        memset(buffer, 0, bufferSize);
        _assocMultiFiles = (decltype(_assocMultiFiles))buffer;
        _paste_oop_init(_assocMultiFiles);
        _assocMultiLoadedImages = (decltype(_assocMultiLoadedImages))(_assocMultiFiles + 1);
        _paste_oop_init(_assocMultiLoadedImages);
        _assocMultiCacheTextures = (decltype(_assocMultiCacheTextures))(_assocMultiLoadedImages + 1);
        _paste_oop_init(_assocMultiCacheTextures);
        _assocSingleFile = (decltype(_assocSingleFile))(_assocMultiCacheTextures + 1);
        _paste_oop_init(_assocSingleFile);
        _resourceLoaded_surfaces = (decltype(_resourceLoaded_surfaces))(_assocSingleFile + 1);
        _paste_oop_init(_resourceLoaded_surfaces);
        _assocCacheTextures = (decltype(_assocCacheTextures))(_resourceLoaded_surfaces + 1);
        _paste_oop_init(_assocCacheTextures);
        _assocCacheCursors = (decltype(_assocCacheCursors))(_assocCacheTextures + 1);
        _paste_oop_init(_assocCacheCursors);
        _assocLoadedMusic = (decltype(_assocLoadedMusic))(_assocCacheCursors + 1);
        _paste_oop_init(_assocLoadedMusic);
    }
    void ResourceManager::gc_release()
    {
        if (_assocMultiFiles == nullptr)
            return;

        _cut_oop_from(_assocMultiLoadedImages);
        _cut_oop_from(_assocMultiCacheTextures);
        _cut_oop_from(_assocSingleFile);
        _cut_oop_from(_resourceLoaded_surfaces);
        _cut_oop_from(_assocCacheTextures);
        _cut_oop_from(_assocCacheCursors);
        _cut_oop_from(_assocLoadedMusic);

        RoninMemory::free(_assocMultiFiles);
        _assocMultiFiles = nullptr;
    }

    void ResourceManager::LoadImages(const char* filename)
    {
        jno_object_parser parser;

        parser.deserialize(filename);
        auto data = parser.get_struct();

        for (auto iter = begin(data); iter != end(data); ++iter) {
            if (!iter->second.isString()) {
                SDL_Log("Image can't load. Incorrect filename in key - %s", iter->second.getPropertyName().data());
                continue;
            }
            if (iter->second.isArray()) {
                std::list<std::string> __names;
                for (auto x = iter->second.toStrings()->begin(); x != iter->second.toStrings()->end(); ++x) {
                    __names.emplace_back(*x);
                }
                _assocMultiFiles->emplace(make_pair(iter->first, __names));
            } else {
                _assocSingleFile->emplace(make_pair(iter->first, iter->second.toString()));
            }
        }
    }

    // Для автоматического уничтожения ресурса, обязательно его нужно скинуть на
    //  ResourceManager::Unload()
    std::list<SDL_Surface*>* ResourceManager::LoadSurfaces(const std::string& packName)
    {
        std::list<SDL_Surface*>* surfs = nullptr;
        std::string path = getDataFrom(FolderKind::SPRITES);
        std::string cat;
        int hash = jno::jno_string_to_hash(packName.c_str());
        auto iter = _assocMultiLoadedImages->find(hash);
        if (iter == end(*_assocMultiLoadedImages)) {
            auto iBits = _assocMultiFiles->find(hash);
            if (iBits != end(*_assocMultiFiles)) {
                RoninMemory::alloc_self(surfs);
                for (auto f = begin(iBits->second); f != end(iBits->second); ++f) {
                    cat = path;
                    cat += *f;
                    surfs->emplace_back(IMG_Load(cat.c_str()));
                }

                _assocMultiLoadedImages->insert(make_pair(hash, surfs));
            }
        } else {
            surfs = iter->second;
        }
        return surfs;
    }

    // Для автоматического уничтожения ресурса, обязательно его нужно скинуть на
    //  ResourceManager::Unload()
    std::list<Texture*>* ResourceManager::LoadTextures(const std::string& packName, bool autoUnload)
    {
        std::list<Texture*>* _textures = nullptr;
        std::list<SDL_Surface*>* surfaces = LoadSurfaces(packName);
        Texture* p;
        if (!surfaces || !surfaces->size())
            return _textures;

        auto iter = _assocMultiCacheTextures->find(surfaces);
        if (iter == end(*_assocMultiCacheTextures)) {
            RoninMemory::alloc_self(_textures);
            for (auto i = begin(*surfaces); i != end(*surfaces); ++i) {
                gc_alloc_texture_from(&p, SDL_CreateTextureFromSurface(Application::get_renderer(), *i));
                _textures->emplace_back(p);
            }
            _assocMultiCacheTextures->emplace(make_pair(surfaces, _textures));
        } else {
            _textures = iter->second;
        }

        return _textures;
    }

    //  ResourceManager::Unload()
    SDL_Surface* ResourceManager::GetSurface(const std::string& surfaceName) { return resource_bitmap(surfaceName); }

    // Для автоматического уничтожения ресурса, обязательно его нужно скинуть на
    //  ResourceManager::Unload()
    Texture* ResourceManager::GetTexture(const std::string& resourceName, bool autoUnload)
    {
        SDL_Surface* surf;
        Texture* texture;

        surf = resource_bitmap(resourceName);

        if (!surf)
            return nullptr;

        auto cache = _assocCacheTextures->find(surf);
        if (cache == end(*_assocCacheTextures)) {
            gc_alloc_texture_from(&texture, surf);
            auto i = _assocCacheTextures->insert(std::make_pair(surf, texture));
            texture->_name = resourceName;
        } else
            texture = cache->second;

        return texture;
    }

    // Create texture format RGBA 8888
    Texture* ResourceManager::GetTexture(const int w, const int h) { return GetTexture(w, h, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888); }
    Texture* ResourceManager::GetTexture(const int w, const int h, const ::SDL_PixelFormatEnum format) { return GetTexture(w, h, format, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING); }
    Texture* ResourceManager::GetTexture(const int w, const int h, const ::SDL_PixelFormatEnum format, const ::SDL_TextureAccess access)
    {
        Texture* tex;
        gc_alloc_texture(&tex, w, h, format, access);
        if (tex == nullptr)
            throw std::bad_alloc();

        return tex;
    }

    // Для автоматического уничтожения ресурса, обязательно его нужно скинуть на
    //  ResourceManager::Unload()
    SDL_Cursor* ResourceManager::GetCursor(const std::string& resourceName, const Vec2Int& hotspot, bool autoUnload) { return GetCursor(GetSurface(resourceName), hotspot); }

    // Для автоматического уничтожения ресурса, обязательно его нужно скинуть на
    //  ResourceManager::Unload()
    SDL_Cursor* ResourceManager::GetCursor(SDL_Surface* texture, const Vec2Int& hotspot)
    {
        SDL_Cursor* cur = nullptr;
        auto find = _assocCacheCursors->find(texture);
        if (texture) {
            if (find != _assocCacheCursors->end())
                return find->second;
            else {
                cur = SDL_CreateColorCursor(texture, hotspot.x, hotspot.y);
                if (cur)
                    _assocCacheCursors->emplace(std::make_pair(texture, cur));
            }
        }
        return cur;
    }

    int ResourceManager::resource_bitmap(const std::string& resourceName, SDL_Surface** sdlsurfacePtr)
    {
        SDL_Surface* surf = nullptr;
        std::string path = getDataFrom(FolderKind::SPRITES);
        std::string cat;

        int hash = jno_string_to_hash(resourceName.c_str());
        auto iter = _resourceLoaded_surfaces->find(hash);
        if (iter == end(*_resourceLoaded_surfaces)) {
            auto iBitsource = _assocSingleFile->find(hash);

            if (iBitsource == end(*_assocSingleFile))
                return -1;

            cat = path + iBitsource->second;

            if (!std::filesystem::exists(cat)) {
                SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Texture not found. %s", cat.c_str());
                return -1;
            }

            surf = IMG_Load(cat.c_str());
            if (!surf) {
                SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Invalid texture. %s", cat.c_str());
                return -1;
            }

            _resourceLoaded_surfaces->insert(std::make_pair(hash, surf));
        } else {
            // finded
            surf = iter->second;
        }

        if (sdlsurfacePtr != nullptr)
            *sdlsurfacePtr = surf;

        return 1;
    }

    SDL_Surface* ResourceManager::resource_bitmap(const std::string& resourceName)
    {
        SDL_Surface* sdlsurf;

        if (resource_bitmap(resourceName, &sdlsurf) == -1) {
            sdlsurf = nullptr;
            Application::fail("Surface name " + resourceName + " not registered");
        }
        return sdlsurf;
    }

    // GC SDL Surface ---------------------------------------------------

    int ResourceManager::gc_alloc_sdl_surface(SDL_Surface** sdlsurfacePtr, const int& w, const int& h) { return gc_alloc_sdl_surface(sdlsurfacePtr, w, h, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888); }

    int ResourceManager::gc_alloc_sdl_surface(SDL_Surface** sdlsurfacePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format)
    {
        int id;
        SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, format);
        throw std::exception();
        return id;
    }

    // GC Texture---------------------

    int ResourceManager::gc_alloc_sdl_texture(SDL_Texture** sdltexturePtr, const int& w, const int& h) { return gc_alloc_sdl_texture(sdltexturePtr, w, h, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888); }

    int ResourceManager::gc_alloc_sdl_texture(SDL_Texture** sdltexturePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format) { return gc_alloc_sdl_texture(sdltexturePtr, w, h, format, SDL_TextureAccess::SDL_TEXTUREACCESS_STATIC); }

    int ResourceManager::gc_alloc_sdl_texture(SDL_Texture** sdltexturePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format, const SDL_TextureAccess& access)
    {
        int id;

        auto&& gc_ptr = reinterpret_cast<SDL_Texture*>(SDL_CreateTexture(Application::get_renderer(), format, access, w, h));

        if (sdltexturePtr != nullptr)
            (*sdltexturePtr) = gc_ptr;

        return id;
    }

    int ResourceManager::gc_alloc_sdl_texture(SDL_Texture** sdltexturePtr, SDL_Surface* from)
    {
        SDL_Texture* texture;

        if (sdltexturePtr == nullptr) {
            Application::fail("init");
        }

        texture = SDL_CreateTextureFromSurface(Application::get_renderer(), from);

        if (texture == nullptr)
            Application::fail_oom_kill();

        (*sdltexturePtr) = texture;

        return 1;
    }

    int ResourceManager::gc_alloc_texture_empty(Texture** texturePtr) { return gc_alloc_texture(texturePtr, 32, 32); }

    int ResourceManager::gc_alloc_texture(Texture** texturePtr, const int& w, const int& h) { return gc_alloc_texture(texturePtr, w, h, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888); }
    int ResourceManager::gc_alloc_texture(Texture** texturePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format)
    {
        return gc_alloc_texture(texturePtr, w, h, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STATIC);
    }
    int ResourceManager::gc_alloc_texture(Texture** texturePtr, const int& w, const int& h, const SDL_PixelFormatEnum& format, const SDL_TextureAccess& access)
    {
        Texture* texture;

        if (texturePtr == nullptr)
            Application::fail("init");

        RoninMemory::alloc_self(texture);
        (*texturePtr) = texture;

        // create SDL Texture (Native) and set to Texture
        gc_alloc_sdl_texture(&texture->m_native, w, h, format, access);
        return 1;
    }

    int ResourceManager::gc_alloc_texture_from(Texture** texturePtr, SDL_Surface* from)
    {
        if (!from || !texturePtr) {
            Application::fail("init");
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(Application::get_renderer(), from);
        gc_alloc_texture_from(texturePtr, texture);

        return 1;
    }

    int ResourceManager::gc_alloc_texture_from(Texture** texturePtr, SDL_Texture* sdltexture)
    {
        if (!sdltexture || !texturePtr) {
            Application::fail("init");
        }
        Texture* texture;
        RoninMemory::alloc_self(texture);
        texture->m_native = sdltexture;

        if (texturePtr != nullptr)
            (*texturePtr) = texture;

        return 1;
    }

    // GC Sprite---------------------

    int ResourceManager::gc_alloc_sprite_empty(Sprite** spritePtr) { return gc_alloc_sprite_empty(spritePtr, { 0, 0, 0, 0 }); }

    int ResourceManager::gc_alloc_sprite_empty(Sprite** spritePtr, const Rect& rect) { return gc_alloc_sprite_with(spritePtr, nullptr, rect, Vec2::half); }

    int ResourceManager::gc_alloc_sprite_with(Sprite** spritePtr, SDL_Surface* src) { return gc_alloc_sprite_with(spritePtr, src, Vec2::half); }

    int ResourceManager::gc_alloc_sprite_with(Sprite** spritePtr, SDL_Surface* src, const Vec2& center)
    {
        Rect rect {};
        if (src == nullptr) {
            static_assert(true, "source arg is null");
        } else {
            rect.w = src->w;
            rect.h = src->h;
        }

        return gc_alloc_sprite_with(spritePtr, src, rect, center);
    }

    int ResourceManager::gc_alloc_sprite_with(Sprite** spritePtr, SDL_Surface* src, const Rect& rect, const Vec2& center)
    {
        if (spritePtr == nullptr) {
            Application::fail("init");
        }

        Sprite* sprite = RoninMemory::alloc<Sprite>();

        sprite->source = src;
        sprite->m_center = center;
        sprite->m_rect = rect;

        *spritePtr = sprite;

        return 1;
    }

} // namespace RoninEngine::Runtime
