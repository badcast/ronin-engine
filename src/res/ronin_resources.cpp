#include <json/json.h>

#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime
{
    extern GidResources *external_global_resources;

    enum : resource_id
    {
        RES_INVALID = 0xffffffff,
        RES_LOCAL_FLAG = 0x80000000
    };

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
        return (id & RES_LOCAL_FLAG) ? World::self()->internal_resources->external_local_resources : external_global_resources;
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

    inline std::vector<std::int8_t> stream_to_mem(std::istream &stream)
    {
        std::size_t delta;
        std::vector<std::int8_t> memory;

        stream.seekg(0, std::ios_base::end);
        delta = stream.tellg();
        stream.seekg(0, std::ios_base::beg);

        if(delta > 0)
        {
            memory.resize(delta);
            stream.read((char *) memory.data(), memory.size());
        }
        else
        {
            constexpr int block_size = 4096;
            char *buffer = new char[block_size];

            if(buffer == nullptr)
                RoninSimulator::Kill();

            delta = 0;
            while(stream.peek() != std::ios_base::eofbit)
            {
                // read from
                std::size_t pos = stream.readsome(buffer, block_size);
                // resize
                memory.resize(delta + pos);
                // copy to
                memcpy(memory.data() + delta, buffer, pos);
                delta += pos;
            }
            delete[] buffer;
        }
        return memory;
    }

    resource_id Resources::LoadImageFromStream(std::istream &stream, bool local)
    {
        resource_id id;
        GidResources *gid;

        auto memory = stream_to_mem(stream);

        SDL_Surface *surf = IMG_Load_RW(SDL_RWFromConstMem(memory.data(), memory.size()), SDL_TRUE);

        if(surf == nullptr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Surface: can not load");
            return RES_INVALID;
        }

        gid = make_resource(&id, local);
        id |= gid->gid_surfaces.size();

        gid->gid_surfaces.emplace_back(surf);
        return id;
    }

    resource_id Resources::LoadAudioClipFromStream(std::istream &stream, bool local)
    {
        resource_id id;
        GidResources *gid;

        auto memory = stream_to_mem(stream);

        Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(memory.data(), memory.size()), SDL_TRUE);

        if(chunk == nullptr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Sound: can not load");
            return RES_INVALID;
        }

        gid = make_resource(&id, local);
        id |= gid->gid_audio_clips.size();

        gid->gid_audio_clips.emplace_back(RoninMemory::alloc<AudioClip>(chunk));
        return id;
    }

    resource_id Resources::LoadMusicClipFromStream(std::istream &stream, bool local)
    {
        resource_id id;
        GidResources *gid;

        auto memory = stream_to_mem(stream);

        Mix_Music *music = Mix_LoadMUSType_RW(SDL_RWFromConstMem(memory.data(), memory.size()), MUS_OGG, SDL_TRUE);

        if(music == nullptr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music: can not load");
            return RES_INVALID;
        }

        gid = make_resource(&id, local);
        id |= gid->gid_music_clips.size();

        gid->gid_music_clips.emplace_back(RoninMemory::alloc<MusicClip>(music));
        return id;
    }

    resource_id Resources::LoadImage(const std::string &path, bool local)
    {
        std::ifstream file(path);

        if(!file)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music: \"%s\" can not load", path.c_str());
            return RES_INVALID;
        }

        return LoadImageFromStream(file, local);
    }

    resource_id Resources::LoadAudioClip(const std::string &path, bool local)
    {
        std::ifstream file(path);

        if(!file)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music: \"%s\" can not load", path.c_str());
            return RES_INVALID;
        }

        return LoadAudioClipFromStream(file, local);
    }

    resource_id Resources::LoadMusicClip(const std::string &path, bool local)
    {
        std::ifstream file(path);

        if(!file)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music: \"%s\" can not load", path.c_str());
            return RES_INVALID;
        }

        return LoadMusicClipFromStream(file, local);
    }

    native_surface_t *Resources::GetImageSource(resource_id resource)
    {
        if(resource == RES_INVALID)
            return nullptr;
        GidResources *gid;
        gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_surfaces.size())
            return nullptr;
        return gid->gid_surfaces[resource];
    }

    AudioClip *Resources::GetAudioClipSource(resource_id resource)
    {
        if(resource == RES_INVALID)
            return nullptr;
        GidResources *gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_audio_clips.size())
            return nullptr;
        return gid->gid_audio_clips[resource];
    }

    MusicClip *Resources::GetMusicClipSource(resource_id resource)
    {
        if(resource == RES_INVALID)
            return nullptr;
        GidResources *gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_music_clips.size())
            return nullptr;
        return gid->gid_music_clips[resource];
    }
} // namespace RoninEngine::Runtime
