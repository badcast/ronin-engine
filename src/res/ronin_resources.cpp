#include "ronin.h"
#include "ronin_debug.h"
#include "ronin_audio.h"
#include "miniaudio/miniaudio.h"

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
            RoninMemory::free(ac_res);
        }
        gid->gid_audio_clips.clear();
        for(MusicClip *mus_res : gid->gid_music_clips)
        {
            RoninMemory::free(mus_res);
        }
        gid->gid_music_clips.clear();
        for(SDL_Surface *surf_res : gid->gid_surfaces)
        {
            SDL_FreeSurface(surf_res);
        }
        gid->gid_surfaces.clear();
        for(void *mem : gid->gid_privates)
        {
            RoninMemory::mem_free(mem);
        }
        gid->gid_privates.clear();
    }

    GroupResources *gid_get(bool local)
    {
        return (local ? (currentWorld && currentWorld->irs ? &(currentWorld->irs->externalLocalResources) : nullptr) : external_global_resources);
    }

    inline GroupResources *get_resource(ResId id)
    {
        return (id & RES_LOCAL_FLAG) ? (currentWorld && currentWorld->irs ? &(currentWorld->irs->externalLocalResources) : nullptr) : external_global_resources;
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
        std::size_t sz = 0;
        char *memory = nullptr;
        stream.seekg(0, std::ios_base::end);
        std::streampos endPos = stream.tellg();
        stream.seekg(0, std::ios_base::beg);
        if(endPos > 0)
        {
            sz = static_cast<std::size_t>(endPos);
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
        else if(endPos == 0)
        {
            return std::make_pair(0, nullptr);
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
        if (memory.first == 0 || memory.second == nullptr)
            return RES_INVALID;

        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
        ma_uint64 frameCount = 0;
        void *pFrames = nullptr;
        ma_result res = ronin_decode_memory(memory.second, memory.first, &config, &frameCount, &pFrames);

        // free loaded stream buffer
        RoninMemory::mem_free(memory.second);

        if (res != MA_SUCCESS || pFrames == nullptr || frameCount == 0)
        {
            ronin_err("sound stream can not load");
            return RES_INVALID;
        }

        gid = make_resource(&id, local);
        id |= gid->gid_audio_clips.size();

        AudioClip *clip = RoninMemory::alloc<AudioClip>();
        clip->initDecodedData(pFrames, frameCount, config.channels, config.sampleRate, config.format);
        ma_free(pFrames, NULL);

        gid->gid_audio_clips.emplace_back(clip);
        return id;
    }

    ResId Resources::LoadMusicClipFromStream(std::istream &stream, bool local)
    {
        ResId id;
        GroupResources *gid;

        auto memory = stream_to_mem(stream);
        if (memory.first == 0 || memory.second == nullptr)
            return RES_INVALID;

        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
        ma_uint64 frameCount = 0;
        void *pFrames = nullptr;
        ma_result res = ronin_decode_memory(memory.second, memory.first, &config, &frameCount, &pFrames);

        // free loaded stream buffer
        RoninMemory::mem_free(memory.second);

        if (res != MA_SUCCESS || pFrames == nullptr || frameCount == 0)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music: can not load");
            return RES_INVALID;
        }

        gid = make_resource(&id, local);
        id |= gid->gid_music_clips.size();

        MusicClip *music = RoninMemory::alloc<MusicClip>();
        music->channels = config.channels;
        music->sampleRate = config.sampleRate;
        music->format = config.format;
        music->totalFrames = frameCount;
        size_t bpf = ma_get_bytes_per_frame(config.format, config.channels);
        size_t totalBytes = frameCount * bpf;
        music->lengthInBytes = static_cast<uint32_t>(totalBytes);
        music->duration = (config.sampleRate > 0) ? (static_cast<double>(frameCount) / config.sampleRate) : 0.0;
        music->pcmData.resize(totalBytes);
        std::memcpy(music->pcmData.data(), pFrames, totalBytes);
        ma_free(pFrames, NULL);

        gid->gid_music_clips.emplace_back(music);
        return id;
    }

    ResId Resources::LoadImage(const std::string &path, bool local)
    {
        std::ifstream file(path, std::ios_base::binary);

        if(!file)
        {
            ronin_err(std::string("Image: \"" + path + "\" can not load").c_str());
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
            ronin_err(std::string("AudioClip: \"" + path + "\" can not load").c_str());
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
        GroupResources *gid = get_resource(resource);
        if(!gid)
            return nullptr;
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
        if(!gid)
            return nullptr;
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
        if(!gid)
            return nullptr;
        resource &= ~RES_LOCAL_FLAG;
        if(resource >= gid->gid_music_clips.size())
            return nullptr;
        return gid->gid_music_clips[resource];
    }
} // namespace RoninEngine::Runtime
