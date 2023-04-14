#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{

    // serialize
    enum FolderKind {
        // graphics
        SPRITES,
        // SOUND
        SOUND,
        // MUSIC
        MUSIC,
        // LOADER
        LOADER,
        // LEVELS
        LEVELS
    };

    RONIN_API const char* applicationPath();

    RONIN_API const char* prefDataPath();

    RONIN_API const std::string dataPath();

    RONIN_API const std::string getDataFrom(FolderKind type);

    RONIN_API void release_sdlpaths();
} // namespace RoninEngine::Runtime
