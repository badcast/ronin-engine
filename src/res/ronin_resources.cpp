#include <filesystem>

#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::Runtime::RoninMemory;
using namespace RoninEngine::UI;
using namespace RoninEngine::AI;
using namespace jno;

enum : resource_id
{
    RES_INVALID = 0xffffffff,
    RES_LOCAL_FLAG = 0x80000000
};

namespace RoninEngine::Runtime
{
    extern GidResources *external_global_resources;

    void gid_resources_free(GidResources *gid)
    {
        if(gid == nullptr)
        {
            return;
        }

        for(AudioClip *ac_res : gid->gid_audio_clips)
        {
            // Handle free
            Mix_FreeChunk(ac_res->mix_chunk);

            // 2ndHandle free
            RoninMemory::free(ac_res);
        }

        for(MusicClip *mus_res : gid->gid_music_clips)
        {
            // Handle free
            Mix_FreeMusic(mus_res->mix_music);

            // 2ndHandle free
            RoninMemory::free(mus_res);
        }

        for(SDL_Surface *surf_res : gid->gid_surfaces)
        {
            SDL_FreeSurface(surf_res);
        }

        RoninMemory::free(gid);
    }

    inline GidResources *get_resource(resource_id id)
    {
        return (id & RES_LOCAL_FLAG) ? World::self()->internal_resources->external_local_resources
                                     : external_global_resources;
    }

    inline GidResources *make_resource(resource_id *resultId, bool local)
    {
        GidResources **resources;

        if(local)
        {
            if(World::self() == nullptr)
            {
                throw ronin_load_world_error();
            }
            resources = &World::self()->internal_resources->external_local_resources;
            (*resultId) = RES_LOCAL_FLAG;
        }
        else
        {
            resources = &external_global_resources;
            (*resultId) = 0;
        }

        if((*resources) == nullptr)
        {
            RoninMemory::alloc_self((*resources));
            (*resources)->gid_surfaces.reserve(16);
            (*resources)->gid_audio_clips.reserve(16);
            (*resources)->gid_music_clips.reserve(16);
        }

        return *resources;
    }

    SDL_Surface *private_load_surface(const void *memres, int length)
    {
        resource_id __result;
        SDL_Surface *surf = IMG_Load_RW(SDL_RWFromConstMem(memres, length), SDL_TRUE);
        if(surf != nullptr)
        {
            make_resource(&__result, false)->gid_surfaces.emplace_back(surf);
        }

        return surf;
    }

    resource_id Resources::load_surface(const std::string &path, bool local)
    {
        resource_id id;
        GidResources *gid;

        SDL_Surface *surf = IMG_Load(path.c_str());

        if(surf == nullptr)
        {
            RoninSimulator::fail("img: \"" + path + "\" can not load");
        }

        gid = make_resource(&id, local);
        id |= gid->gid_surfaces.size();

        gid->gid_surfaces.emplace_back(surf);
        return id;
    }

    resource_id Resources::load_audio_clip(const std::string &path, bool local)
    {
        resource_id id;
        GidResources *gid;

        Mix_Chunk *chunk = Mix_LoadWAV(path.c_str());

        if(chunk == nullptr)
        {
            RoninSimulator::fail("sound: \"" + path + "\" can not load");
        }

        gid = make_resource(&id, local);
        id |= gid->gid_audio_clips.size();

        gid->gid_audio_clips.emplace_back(RoninMemory::alloc<AudioClip>(chunk));
        return id;
    }

    resource_id Resources::load_music_clip(const std::string &path, bool local)
    {
        resource_id id;
        GidResources *gid;

        Mix_Music *music = Mix_LoadMUS(path.c_str());

        if(music == nullptr)
        {
            RoninSimulator::fail("music: \"" + path + "\" can not load");
        }

        gid = make_resource(&id, local);
        id |= gid->gid_music_clips.size();

        gid->gid_music_clips.emplace_back(RoninMemory::alloc<MusicClip>(music));
        return id;
    }

    native_surface_t *Resources::get_surface(resource_id resource)
    {
        GidResources *gid;
        gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_surfaces.size())
            return nullptr;
        return gid->gid_surfaces[resource];
    }

    AudioClip *Resources::get_audio_clip(resource_id resource)
    {
        GidResources *gid;
        gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_audio_clips.size())
            return nullptr;
        return gid->gid_audio_clips[resource];
    }

    MusicClip *Resources::get_music_clip(resource_id resource)
    {
        GidResources *gid;
        gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_music_clips.size())
            return nullptr;
        return gid->gid_music_clips[resource];
    }
} // namespace RoninEngine::Runtime
