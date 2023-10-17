#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

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

        for(Sprite *sprite : gid->gid_sprites)
        {
            RoninMemory::free(sprite);
        }

        for(void *mem : gid->gid_privates)
        {
            RoninMemory::ronin_memory_free(mem);
        }
    }

    GidResources *gid_get(bool local)
    {
        return (local ? &(switched_world->irs->external_local_resources) : external_global_resources);
    }

    inline GidResources *get_resource(resource_id id)
    {
        return (id & RES_LOCAL_FLAG) ? &(switched_world->irs->external_local_resources) : external_global_resources;
    }

    void *make_private_resource(void *memory, bool local)
    {
        if(memory != nullptr)
        {
            GidResources *resources;
            if(local)
            {
                if(switched_world == nullptr)
                {
                    throw ronin_null_error();
                }
                resources = &(switched_world->irs->external_local_resources);
            }
            else
            {
                if(external_global_resources == nullptr)
                    RoninMemory::alloc_self(external_global_resources);

                resources = external_global_resources;
            }

            (resources)->gid_privates.emplace_back(memory);
        }
        return memory;
    }

    GidResources *make_resource(resource_id *resultId, bool local)
    {
        GidResources *resources;

        if(local)
        {
            if(switched_world == nullptr)
            {
                throw ronin_load_world_error();
            }
            resources = &(switched_world->irs->external_local_resources);
            (*resultId) = RES_LOCAL_FLAG;
        }
        else
        {
            if(external_global_resources == nullptr)
                RoninMemory::alloc_self(external_global_resources);

            resources = external_global_resources;
            (*resultId) = 0;
        }

        return resources;
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

    std::pair<std::size_t, void *> stream_to_mem(std::istream &stream)
    {
        std::size_t sz;
        char *memory = nullptr;

        stream.seekg(0, std::ios_base::end);
        sz = stream.tellg();
        stream.seekg(0, std::ios_base::beg);

        if(sz > 0)
        {
            memory = static_cast<char *>(RoninMemory::ronin_memory_alloc(sz));

            if(memory == nullptr)
                RoninSimulator::Kill();

            stream.read((char *) memory, sz);
        }
        else
        {
            // 1 MB block/per
            constexpr int block_size = 1024 * 1024;
            char *buffer = new char[block_size];

            if(buffer == nullptr)
                RoninSimulator::Kill();

            sz = 0;
            while(!stream.eof())
            {
                // read from
                std::size_t pos = stream.read(buffer, block_size).gcount();
                // resize
                memory = static_cast<char *>(RoninMemory::ronin_memory_realloc(memory, sz + pos));
                if(memory == nullptr)
                {
                    sz = 0;
                    break;
                }
                // copy to
                memcpy(memory + sz, buffer, pos);
                sz += pos;
            }
            delete[] buffer;
        }
        return std::make_pair(sz, memory);
    }

    resource_id Resources::LoadImageFromStream(std::istream &stream, bool local)
    {
        resource_id id;
        GidResources *gid;

        auto memory = stream_to_mem(stream);
        SDL_Surface *surf = IMG_Load_RW(SDL_RWFromMem(memory.second, memory.first), SDL_TRUE);
        RoninMemory::ronin_memory_free(memory.second);
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

        Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromMem(memory.second, memory.first), SDL_TRUE);
        RoninMemory::ronin_memory_free(memory.second);
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
        Mix_Music *music = Mix_LoadMUS_RW(SDL_RWFromMem(memory.second, memory.first), SDL_TRUE);

        // register to src for free
        make_private_resource(memory.second, local);

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
        std::ifstream file(path, std::ios_base::binary);

        if(!file)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music: \"%s\" can not load", path.c_str());
            return RES_INVALID;
        }

        return LoadImageFromStream(file, local);
    }

    resource_id Resources::LoadAudioClip(const std::string &path, bool local)
    {
        std::ifstream file(path, std::ios_base::binary);

        if(!file)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music: \"%s\" can not load", path.c_str());
            return RES_INVALID;
        }

        return LoadAudioClipFromStream(file, local);
    }

    resource_id Resources::LoadMusicClip(const std::string &path, bool local)
    {
        std::ifstream file(path, std::ios_base::binary);

        if(!file)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music: \"%s\" can not load", path.c_str());
            return RES_INVALID;
        }

        return LoadMusicClipFromStream(file, local);
    }

    Image *Resources::GetImageSource(resource_id resource)
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
