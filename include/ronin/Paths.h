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

    extern const char* applicationPath();

    extern const char* prefDataPath();

    extern const std::string dataPath();

    extern const std::string getDataFrom(FolderKind type);

    extern void release_sdlpaths();
} // namespace RoninEngine::Runtime
