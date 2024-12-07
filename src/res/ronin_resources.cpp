#include "ronin.h"
#include "ronin_debug.h"

using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

namespace RoninEngine::Runtime
{
    extern GroupResources *external_global_resources;

    void gid_resources_free(GroupResources *gid)
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
        for(void *mem : gid->gid_privates)
        {
            RoninMemory::mem_free(mem);
        }
    }

    GroupResources *gid_get(bool local)
    {
        return (local ? &(currentWorld->irs->externalLocalResources) : external_global_resources);
    }

    constexpr GroupResources *get_resource(ResId id)
    {
        return (id & RES_LOCAL_FLAG) ? &(currentWorld->irs->externalLocalResources) : external_global_resources;
    }

    void *make_private_resource(void *memory, bool local)
    {
        if(memory != nullptr)
        {
            GroupResources *resources;
            if(local)
            {
                if(currentWorld == nullptr)
                {
                    throw ronin_null_error();
                }
                resources = &(currentWorld->irs->externalLocalResources);
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

    GroupResources *make_resource(ResId *resultId, bool local)
    {
        GroupResources *resources;

        if(local)
        {
            if(currentWorld == nullptr)
            {
                throw ronin_load_world_error();
            }
            resources = &(currentWorld->irs->externalLocalResources);
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

    SDL_Surface *private_load_surface(const void *memres, int size, bool local)
    {
        ResId __result;
        SDL_Surface *surf = IMG_Load_RW(SDL_RWFromConstMem(memres, size), SDL_TRUE);
        if(surf != nullptr)
        {
            make_resource(&__result, local)->gid_surfaces.emplace_back(surf);
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
        if(sz == 0)
        {
            memory = static_cast<char *>(RoninMemory::mem_alloc(sz));

            if(memory == nullptr)
                throw Exception::ronin_out_of_mem();

            std::size_t reads = stream.read((char *) memory, sz).gcount();
            if(reads != sz)
            {
                ronin_warn("peek size is not an equals reads != peekSize");
                sz = reads;
            }
        }
        else
        {
            // 4 KiB block/per
            constexpr int block_size = 1024 * 4;
            char *buffer = new char[block_size];

            if(buffer == nullptr)
                throw ronin_out_of_mem();

            sz = 0;
            while(!stream.eof())
            {
                // read from
                std::size_t pos = stream.read(buffer, block_size).gcount();
                // resize
                memory = static_cast<char *>(RoninMemory::mem_realloc(memory, sz + pos));
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

    ResId Resources::LoadImageFromStream(std::istream &stream, bool local)
    {
        ResId id;
        GroupResources *gid;
        SDL_Surface *surf, *conv;
        std::pair<std::size_t, void *> memory;

        memory = stream_to_mem(stream);
        surf = IMG_Load_RW(SDL_RWFromConstMem(memory.second, memory.first), SDL_TRUE);

        // free loaded stream buffer
        RoninMemory::mem_free(memory.second);

        if(surf == nullptr)
        {
            ronin_err("image stream can not load");
            return RES_INVALID;
        }

        // Convert surface if possible.
        if(surf->format->format != defaultPixelFormat)
        {
            conv = SDL_ConvertSurfaceFormat(surf, defaultPixelFormat, 0);
            SDL_FreeSurface(surf);
            if(conv == nullptr)
            {
                ronin_err("invalid converting surface to format");
                return RES_INVALID;
            }
            surf = conv;
        }

        gid = make_resource(&id, local);

        id |= gid->gid_surfaces.size();

        gid->gid_surfaces.emplace_back(surf);
        return id;
    }

    Image *Resources::LoadImageFromStreamRight(std::istream &stream, bool local)
    {
        return GetImageSource(LoadImageFromStream(stream, local));
    }

    ResId Resources::LoadAudioClipFromStream(std::istream &stream, bool local)
    {
        ResId id;
        GroupResources *gid;

        auto memory = stream_to_mem(stream);

        Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromMem(memory.second, memory.first), SDL_TRUE);

        // free loaded stream buffer
        RoninMemory::mem_free(memory.second);

        if(chunk == nullptr)
        {
            ronin_err("sound stream can not load");
            return RES_INVALID;
        }

        gid = make_resource(&id, local);
        id |= gid->gid_audio_clips.size();

        gid->gid_audio_clips.emplace_back(RoninMemory::alloc<AudioClip>(chunk));
        return id;
    }

    ResId Resources::LoadMusicClipFromStream(std::istream &stream, bool local)
    {
        ResId id;
        GroupResources *gid;

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

    ResId Resources::LoadImage(const std::string &path, bool local)
    {
        std::ifstream file(path, std::ios_base::binary);

        if(!file)
        {
            ronin_err(std::string("Music: \"" + path + "\" can not load").c_str());
            return RES_INVALID;
        }

        return LoadImageFromStream(file, local);
    }

    Image *Resources::LoadImageRight(const std::string &path, bool local)
    {
        return GetImageSource(LoadImage(path, local));
    }

    ResId Resources::LoadAudioClip(const std::string &path, bool local)
    {
        std::ifstream file(path, std::ios_base::binary);

        if(!file)
        {
            ronin_err(std::string("music stream \"" + path + "\" can not load").c_str());
            return RES_INVALID;
        }

        return LoadAudioClipFromStream(file, local);
    }

    ResId Resources::LoadMusicClip(const std::string &path, bool local)
    {
        std::ifstream file(path, std::ios_base::binary);

        if(!file)
        {
            ronin_err(std::string("Music: \"" + path + "\" can not load").c_str());
            return RES_INVALID;
        }

        return LoadMusicClipFromStream(file, local);
    }

    Image *Resources::GetImageSource(ResId resource)
    {
        if(resource == RES_INVALID)
            return nullptr;
        GroupResources *gid;
        gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_surfaces.size())
            return nullptr;
        return gid->gid_surfaces[resource];
    }

    AudioClip *Resources::GetAudioClipSource(ResId resource)
    {
        if(resource == RES_INVALID)
            return nullptr;
        GroupResources *gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_audio_clips.size())
            return nullptr;
        return gid->gid_audio_clips[resource];
    }

    MusicClip *Resources::GetMusicClipSource(ResId resource)
    {
        if(resource == RES_INVALID)
            return nullptr;
        GroupResources *gid = get_resource(resource);
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_music_clips.size())
            return nullptr;
        return gid->gid_music_clips[resource];
    }
} // namespace RoninEngine::Runtime
