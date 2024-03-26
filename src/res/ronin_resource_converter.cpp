#include <filesystem>
#include <json/json.h>

#include "ronin.h"

using namespace RoninEngine::Runtime;

constexpr const char resource_types[][16] {
    "Sprite",
    "AudioClip",
    "MusicClip",
    // Atlas is Object serialize
    "Atlas"};

constexpr int resource_types_n = sizeof(resource_types) / sizeof(resource_types[0]);

static std::hash<std::string> string_hasher;

void RoninEngine::Runtime::internal_free_loaded_assets()
{
    for(Asset &asset : loaded_assets)
    {
        if(asset.__ref->atlasRef)
        {
            RoninMemory::free(asset.__ref->atlasRef);
        }
        RoninMemory::free(asset.__ref); // free asset
    }

    loaded_assets.clear();
}

bool AssetManager::LoadAsset(const std::string &assetFile, Asset **asset)
{
    int types, status;
    if(asset == nullptr)
    {
        return false;
    }

    Json::Reader json_reader;
    Json::Value json;
    std::ifstream file(assetFile);

    status = json_reader.parse(file, json, false);
    file.close();

    if(!status)
    {
        RoninSimulator::Log("Can not load asset file");
        return false;
    }

    std::string resource_type = json["resource_type"].asString();

    for(types = 0; types < resource_types_n;)
    {
        if(resource_type == resource_types[types])
            break;
        ++types;
    }

    if(types >= resource_types_n)
    {
        RoninSimulator::Log("Can not load asset file");
        return false;
    }

    ResId rcid;
    Json::Value param1, param2, param3;
    std::vector<std::pair<std::size_t, std::string>> __files;
    std::filesystem::path dir_of_asset {assetFile};

    dir_of_asset = dir_of_asset.parent_path();
    switch(types)
    {
        case 0:
        case 1:
        case 2:
        {
            Json::Value resources = json["resources"];

            if(!resources.isArray())
            {
                RoninSimulator::Log("Can not load asset file");
                return false;
            }

            for(auto &resource : resources)
            {
                param1 = resource["name"];
                param2 = resource["ref"];
                if(!param1.isString() || !param2.isString())
                {
                    RoninSimulator::Log(("Invalid element on \"resources\". Incorrect file " + assetFile).c_str());
                    continue;
                }
                size_t hash_it = string_hasher(param1.asString());
                if(std::end(__files) !=
                   std::find_if(std::begin(__files), std::end(__files), [&hash_it](auto &ref) { return hash_it == ref.first; }))
                {
                    RoninSimulator::Log(("Conflict resource name \"" + param1.asString() + "\"").c_str());
                    continue;
                }

                __files.emplace_back(std::make_pair(hash_it, (dir_of_asset / param2.asString()).string()));
            }

            *asset = &loaded_assets.emplace_back();
            RoninMemory::alloc_self((*asset)->__ref);
            for(auto &pair : __files)
            {
                switch(types)
                {
                    /*Sprites*/
                    case 0:
                        rcid = Resources::LoadImage(pair.second, false);
                        break;
                    /*AudioClip*/
                    case 1:
                        rcid = Resources::LoadAudioClip(pair.second, false);
                        break;
                    /*MusicClip*/
                    case 2:
                        rcid = Resources::LoadMusicClip(pair.second, false);
                        break;
                    default:
                        rcid = RES_INVALID;
                        break;
                }

                if(rcid == RES_INVALID)
                {
                    RoninSimulator::Log(("Invalid resource " + pair.second).c_str());
                    continue;
                }

                switch(types)
                {
                    /*Sprites*/
                    case 0:
                        (*asset)->__ref->image_clips[pair.first] = rcid;
                        break;
                    /*AudioClip*/
                    case 1:
                        (*asset)->__ref->audio_clips[pair.first] = rcid;
                        break;
                    /*MusicClip*/
                    case 2:
                        (*asset)->__ref->music_clips[pair.first] = rcid;
                        break;
                }
            }
        }
        break;
        case 3:
            // ATLAS RESOURCE TYPE
            Atlas *atlas;
            Sprite *spr;
            Rect rect;
            std::string _src;

            Json::Value map = json["map"];
            if(!map.isArray())
            {
                RoninSimulator::Log("Can not load asset file. Map key is not entry");
                return false;
            }

            param1 = json["resource_ref"];
            if(!param1.isString())
            {
                RoninSimulator::Log(("Invalid atlas key \"resource_ref\" is not valid from " + assetFile).c_str());
                return false;
            }

            _src = dir_of_asset;
            _src += Paths::GetPathSeperatorOS();
            _src += param1.asString();

            rcid = Resources::LoadImage(_src, false);
            if(rcid == RES_INVALID)
            {
                RoninSimulator::Log(("Atlas image is invalid " + param1.asString()).c_str());
                return false;
            }

            *asset = &loaded_assets.emplace_back();
            RoninMemory::alloc_self((*asset)->__ref);

            atlas = (*asset)->__ref->atlasRef = RoninMemory::alloc<Atlas>();
            atlas->src = Resources::GetImageSource(rcid);

            // Map entry
            for(auto &m : map)
            {
                param1 = m["rect"];
                if(!param1.isString())
                {
                    RoninSimulator::Log("Is not valid key entry \"rect\"");
                    continue;
                }
                rect = {}; // set flush
                std::sscanf(param1.asString().c_str(), "%d %d %d %d", &rect.x, &rect.y, &rect.w, &rect.h);
                spr = Primitive::CreateEmptySprite(false);

                // TODO: make sprite name spr->name = param1.empty()
                param1 = m["sprite"];
                if(!param1.empty())
                {
                    if(!param1.isString())
                    {
                        RoninSimulator::Log("Is not valid key entry \"sprite\"");
                    }
                    else
                    {
                        spr->name = param1.asString();
                    }
                }

                spr->m_rect = rect;
                spr->surface = atlas->src;

                atlas->_sprites.emplace_back(spr);
            }

            break;
    }
    return true;
}

Cursor *AssetManager::ConvertImageToCursor(Image *imageSrc, Vec2Int cursorHotspot)
{
    Cursor *cursor = SDL_CreateColorCursor(imageSrc, cursorHotspot.x, cursorHotspot.y);
    return cursor;
}

Image *Asset::GetImage(const std::string &name)
{
    auto f = __ref->image_clips.find(string_hasher(name));

    if(f == std::end(__ref->image_clips))
        return nullptr;

    return Resources::GetImageSource(f->second);
}

AudioClip *Asset::GetAudioClip(const std::string &name)
{
    auto f = __ref->audio_clips.find(string_hasher(name));
    if(f == std::end(__ref->audio_clips))
        return nullptr;
    return Resources::GetAudioClipSource(f->second);
}

Atlas *Asset::GetAtlasObject()
{
    return __ref->atlasRef;
}
