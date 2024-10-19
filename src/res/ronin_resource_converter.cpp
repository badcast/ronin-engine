#include <filesystem>

#include "ronin.h"
#include "ronin_debug.h"

#include "../3rdparty/SimpleJSON/json.hpp"

using namespace RoninEngine::Runtime;

constexpr const char resClass[][16] {
    "Sprite",
    "AudioClip",
    "MusicClip",
    // Atlas is Object serialize
    "Atlas"};

constexpr char modeAtlas[][6] {"Rect", "Tiled"};

constexpr char atlasDirections[][6] {"Right", "Down"};

std::hash<std::string> stringHasher;

void RoninEngine::Runtime::internal_free_loaded_assets()
{
    for(Asset &asset : loaded_assets)
    {
        if(asset.ref->atlasRef)
        {
            RoninMemory::free(asset.ref->atlasRef);
        }
        RoninMemory::free(asset.ref); // free asset
    }
    loaded_assets.clear();

    // System Cursors
    for(Cursor *cur : gscope.sysCursors)
    {
        if(cur)
            SDL_FreeCursor(cur);
    }
    gscope.sysCursors.clear();
    gscope.sysCursors.shrink_to_fit();
}

Cursor *AssetManager::ConvertImageToCursor(Image *imageSrc, Vec2Int cursorHotspot)
{
    Cursor *cursor = SDL_CreateColorCursor(imageSrc, cursorHotspot.x, cursorHotspot.y);
    return cursor;
}

void AssetManager::DeleteAsset(Asset *asset)
{

}

bool AssetManager::LoadAsset(const std::string &loaderFile, Asset **asset)
{
    using namespace json;

    if(asset == nullptr)
    {
        ronin_log("asset argument is null pointer");
        return false;
    }

    ResId rcid;
    int num1, num2, num3, num4;
    std::string str1;
    JSON json, param1, param2, param3, map;
    std::vector<std::pair<std::size_t, std::string>> __files;
    std::filesystem::path dir_of_asset;
    std::ifstream input(loaderFile);
    str1.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    input.close();
    json = JSON::Load(str1);
    if(json.IsNull())
    {
        ronin_log("Can not load asset file");
        return false;
    }

    num2 = sizeof(resClass) / sizeof(resClass[0]);

    str1 = json["class"].ToString();

    for(num1 = 0; num1 < num2;)
    {
        if(str1 == resClass[num1])
            break;
        ++num1;
    }

    if(num1 >= num2)
    {
        ronin_log("Can not load asset file");
        return false;
    }

    dir_of_asset = loaderFile;
    dir_of_asset = dir_of_asset.parent_path();

    switch(num1)
    {
        case 0:
        case 1:
        case 2:
        {
            JSON sources = json["source"];

            if(sources.JSONType() != JSON::Class::Array)
            {
                ronin_log("Can not load asset file");
                return false;
            }

            for(auto &source0 : sources.ArrayRange())
            {
                param1 = source0["name"];
                param2 = source0["ref"];
                if(param1.JSONType() != JSON::Class::String || param2.JSONType() != JSON::Class::String)
                {
                    ronin_log(("Invalid element on \"resources\". Incorrect file " + loaderFile).c_str());
                    continue;
                }
                size_t hash_it = stringHasher(param1.ToString());
                if(std::end(__files) != std::find_if(std::begin(__files), std::end(__files), [&hash_it](auto &ref) { return hash_it == ref.first; }))
                {
                    ronin_log(("Conflict resource name \"" + param1.ToString() + "\"").c_str());
                    continue;
                }

                __files.emplace_back(std::make_pair(hash_it, (dir_of_asset / param2.ToString()).string()));
            }

            *asset = &loaded_assets.emplace_back();
            RoninMemory::alloc_self((*asset)->ref);
            for(auto &pair : __files)
            {
                switch(num1)
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
                    ronin_log(("Invalid resource " + pair.second).c_str());
                    continue;
                }

                switch(num1)
                {
                    /*Sprites*/
                    case 0:
                        (*asset)->ref->bindSprites[pair.first] = Primitive::CreateSpriteFrom(Resources::GetImageSource(rcid), false);
                        break;
                    /*AudioClip*/
                    case 1:
                        (*asset)->ref->bindAudioClips[pair.first] = rcid;
                        break;
                    /*MusicClip*/
                    case 2:
                        (*asset)->ref->bindMusicClips[pair.first] = rcid;
                        break;
                    default:;
                }
            }
        }
        break;
        case 3:
            // ATLAS RESOURCE TYPE
            std::string _src;
            Atlas *atlas;
            Sprite *spr;
            Rect rect;

            param1 = json["mode"];
            if(!param1.IsNull())
            {
                if(param1.JSONType() != JSON::Class::String)
                {
                    ronin_log("mode param is invalid and not contains atlas mode");
                    return false;
                }
                str1 = param1.ToString();
                for(num3 = 0, num2 = sizeof(modeAtlas) / sizeof(modeAtlas[0]); num3 < num2;)
                {
                    if(modeAtlas[num3] == str1)
                        break;
                    ++num3;
                }

                if(num3 >= num2)
                {
                    ronin_log("Can not load asset file");
                    return false;
                }
            }
            else
            {
                num3 = 0;
            }

            // Load Atlas Map

            map = json["map"];
            switch(num3)
            {
                case 0: // RECT
                    if(map.JSONType() != JSON::Class::Array)
                    {
                        ronin_log("Can not load asset file. Map key is not contains Array maps");
                        return false;
                    }
                    num2 = map.size();
                    break;
                case 1: // TILED
                    if(map.JSONType() != JSON::Class::String)
                    {
                        ronin_log("Can not assign mode is Tiled. Map key is not contains Rect value");
                        return false;
                    }

                    rect = {0, 0, -1, -1}; // set flush
                    std::sscanf(map.ToString().c_str(), "%d %d", &rect.w, &rect.h);

                    num4 = 0; // Set default direction (Right)
                    param1 = json["direction"];
                    if(param1.JSONType() == JSON::Class::String)
                    {
                        str1 = param1.ToString();
                        num2 = sizeof(atlasDirections) / sizeof(atlasDirections[0]);
                        for(num1 = 0; num1 < num2;)
                        {
                            if(str1 == atlasDirections[num1])
                                break;
                            ++num1;
                        }

                        if(num1 < num2)
                        {
                            // Assign new direction
                            num4 = num1;
                        }
                        else
                        {
                            ronin_log("Invalid key value in \"direction\"");
                        }
                    }

                    break;
                default:
                    ronin_log("Invalid atlas mode");
                    return false;
                    break;
            }

            param1 = json["source"];
            if(param1.JSONType() != JSON::Class::String)
            {
                ronin_log(("Invalid atlas key \"source\" is not valid from " + loaderFile).c_str());
                return false;
            }

            _src = dir_of_asset.generic_string();
            _src += Paths::GetPathSeperatorOS();
            _src += param1.ToString();

            rcid = Resources::LoadImage(_src, false);
            if(rcid == RES_INVALID)
            {
                ronin_log(("Atlas image is invalid " + param1.ToString()).c_str());
                return false;
            }

            *asset = &loaded_assets.emplace_back();
            RoninMemory::alloc_self((*asset)->ref);

            atlas = (*asset)->ref->atlasRef = RoninMemory::alloc<Atlas>();
            atlas->src = Resources::GetImageSource(rcid);

            // Post settings Tiled Mode
            int x, y;
            if(num3 == 1)
            {
                SDL_Surface *surf = atlas->src;

                if(rect.w == -1)
                    rect.w = surf->w;
                if(rect.h == -1)
                    rect.h = surf->h;

                x = surf->w / rect.w;
                y = surf->h / rect.h;
                num2 = x * y;
                --x;
                --y;
            }

            for(num1 = 0; num1 < num2; ++num1)
            {
                // RECT
                if(num3 == 0)
                {
                    param3 = map[num1];
                    param1 = param3["value"];
                    if(param1.JSONType() != JSON::Class::String)
                    {
                        ronin_log("Is not valid key entry value is Rect");
                        continue;
                    }
                    rect = {}; // set flush
                    std::sscanf(param1.ToString().c_str(), "%d %d %d %d", &rect.x, &rect.y, &rect.w, &rect.h);

                    spr = Primitive::CreateEmptySprite(false);

                    // TODO: make sprite name spr->name = param1.empty()
                    param1 = param3["sprite"];
                    if(!param1.IsNull())
                    {
                        if(param1.JSONType() != JSON::Class::String)
                        {
                            ronin_log("Is not valid key entry \"sprite\"");
                        }
                        else
                        {
                            spr->name = param1.ToString();
                        }
                    }
                }
                // TILED
                else
                {
                    spr = Primitive::CreateEmptySprite(false);
                }

                spr->m_rect = rect;
                spr->surface = atlas->src;

                atlas->_sprites.emplace_back(spr);

                // Post setting Tiled
                if(num3 == 1)
                {
                    // num4 - direction property
                    switch(num4)
                    {
                        case 0: // Right
                            if(rect.x >= x * rect.w)
                                rect.x = 0;
                            else
                                rect.x += rect.w;

                            if(rect.y >= y * rect.h)
                                rect.y = 0;
                            else
                                rect.y += rect.h;
                            break;

                        case 1: // Down
                            // TODO: Make direction to down
                            num4 = 0;
                            continue;

                        default:;
                    }
                }
            }
            break;
    }
    return true;
}

Sprite *Asset::GetSprite(const std::string &name)
{
    auto f = ref->bindSprites.find(stringHasher(name));

    if(f == std::end(ref->bindSprites))
        return nullptr;

    return f->second;
}

AudioClip *Asset::GetAudioClip(const std::string &name)
{
    auto f = ref->bindAudioClips.find(stringHasher(name));

    if(f == std::end(ref->bindAudioClips))
        return nullptr;

    return Resources::GetAudioClipSource(f->second);
}

Atlas *Asset::GetAtlasObject()
{
    return ref->atlasRef;
}
