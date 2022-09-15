#include "framework.h"

char* basePath = nullptr;
char* prefPath = nullptr;
namespace RoninEngine::Runtime {

static const char seperator =
#ifdef __WIN32__
    '\\'
#else
    '/'
#endif
    ;
constexpr char data_path[] = "data";
constexpr char gfx_path[] = "gfx";
constexpr char sound_path[] = "sound";
constexpr char music_path[] = "music";
constexpr char loader_path[] = "loader";
constexpr char levels_path[] = "levels";
constexpr char textures_path[] = "textures";

const char* applicationPath() {
    if (!basePath) basePath = SDL_GetBasePath();
    return basePath;
}

const char* prefDataPath() {
    if (!prefPath) prefPath = SDL_GetPrefPath("RoninEngine", "prefDir");
    return prefPath;
}

const std::string dataPath() {
    std::string s = applicationPath();
    s += data_path;
    return s;
}

const std::string getDataFrom(FolderKind type) {
    std::string p = dataPath();
    p += seperator;
    switch (type) {
        case FolderKind::GFX:
            p += gfx_path;
            break;
        case FolderKind::SOUND:
            p += sound_path;
            break;
        case FolderKind::MUSIC:
            p += music_path;
            break;
        case FolderKind::LOADER:
            p += loader_path;
            break;
        case FolderKind::LEVELS:
            p += levels_path;
            break;
        case FolderKind::TEXTURES:
            p += textures_path;
            break;
    }
    return p += seperator;
}

void release_sdlpaths() {
    if (basePath) SDL_free(basePath);
    if (prefPath) SDL_free(prefPath);
    basePath = nullptr;
    prefPath = nullptr;
}
}  // namespace RoninEngine::Runtime
