#pragma once

#include "dependency.h"

namespace RoninEngine::Runtime {

// serialize
enum FolderKind {
    // graphics
    GFX,
    // SOUND
    SOUND,
    // MUSIC
    MUSIC,
    // LOADER
    LOADER,
    // LEVELS
    LEVELS,
    // TEXTURES
    TEXTURES
};

const char* applicationPath();

const char* prefDataPath();

const std::string dataPath();

const std::string getDataFrom(FolderKind type);

void release_sdlpaths();
}  // namespace RoninEngine::Runtime
