#include <filesystem>

#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::Runtime::RoninMemory;
using namespace RoninEngine::UI;
using namespace RoninEngine::AIPathFinder;
using namespace jno;

namespace RoninEngine::Runtime
{
    AudioClip* Resources::load_clip(const std::string& path)
    {

        Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
        if (chunk == nullptr) {
            return nullptr;
        }

        return &(World::self()->internal_resources->offload_audioclips.emplace_back(AudioClip { 0, chunk }));
    }


} // namespace RoninEngine::Runtime
