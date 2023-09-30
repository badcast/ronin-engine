#include <filesystem>

#include <json/json.h>

#include "ronin.h"

using namespace RoninEngine::Runtime;

constexpr int resources_types_n = 3;
constexpr const char resource_types[resources_types_n][16] {"Sprite", "AudioClip", "MusicClip"};
static std::hash<std::string> string_hasher;

void RoninEngine::Runtime::internal_free_loaded_assets(){
    for(Asset & asset : loaded_assets){
        RoninMemory::free(asset.__ref); // free asset
    }

    loaded_assets.clear();
}

bool AssetManager::LoadAsset(const std::string &assetFile, Asset **asset)
{
    if(asset == nullptr)
    {
        return false;
    }

    Json::Reader json_reader;
    Json::Value root;
    std::ifstream file(assetFile);

    if(!json_reader.parse(file, root))
    {
        RoninSimulator::Log("Can not load asset file");
        return false;
    }

    int types;
    std::string resource_type = root["resource_type"].asString();
    Json::Value resources = root["resources"];

    if(!resources.isArray())
    {
        RoninSimulator::Log("Can not load asset file");
        return false;
    }

    for(types = 0; types < resources_types_n;)
    {
        if(resource_type == resource_types[types])
            break;
        ++types;
    }

    if(types >= resources_types_n)
    {
        RoninSimulator::Log("Can not load asset file");
        return false;
    }

    std::vector<std::pair<std::size_t, std::string>> __files;
    Json::Value param1, param2;
    std::filesystem::path dir_of_asset {assetFile};
    dir_of_asset = dir_of_asset.parent_path();
    for(auto &resource : resources)
    {
        param1 = resource["name"];
        param2 = resource["ref"];
        if(!param1.isString() || !param2.isString())
        {
            RoninSimulator::Log("Invalid element on \"resources\". Incorrect file " + assetFile);
            continue;
        }

        __files.emplace_back(std::make_pair(string_hasher(param1.asString()), (dir_of_asset / param2.asString()).string()));
    }

    resource_id rcid;
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
            RoninSimulator::Log("Invalid resource " + pair.second);
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
    return true;
}

Image *Asset::GetImage(const std::string &key)
{
    auto f = __ref->image_clips.find(string_hasher(key));

    if(f == std::end(__ref->image_clips))
        return nullptr;

    return Resources::GetImageSource(f->second);
}
