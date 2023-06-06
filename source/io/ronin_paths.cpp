#include "ronin.h"

char* basePath = nullptr;
char* prefPath = nullptr;
namespace RoninEngine::Runtime
{

    static const char seperator =
#ifdef __WIN32__
        '\\'
#else
        '/'
#endif
        ;
    static char data_path[] = "data";
    static char sound_path[] = "sound";
    static char music_path[] = "music";
    static char loader_path[] = "loader";
    static char levels_path[] = "levels";
    static char sprites_path[] = "sprites";

    const char* applicationPath()
    {
        if (!basePath)
            basePath = SDL_GetBasePath();
        return basePath;
    }

    const char* prefDataPath()
    {
        if (!prefPath)
            prefPath = SDL_GetPrefPath("RoninEngine", "application");
        return prefPath;
    }

    const std::string dataPath()
    {
        std::string s = applicationPath();
        s += data_path;
        return s;
    }

    const std::string getDataFrom(FolderKind type)
    {
        std::string p = dataPath();
        p += seperator;
        switch (type) {
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
        case FolderKind::SPRITES:
            p += sprites_path;
            break;
        }
        return p += seperator;
    }

    void release_sdlpaths()
    {
        if (basePath)
            SDL_free(basePath);
        if (prefPath)
            SDL_free(prefPath);
        basePath = nullptr;
        prefPath = nullptr;
    }
} // namespace RoninEngine::Runtime
