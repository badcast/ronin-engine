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
    for(AssetRef &asset : loaded_assets)
    {
        RefMarkNull(asset);
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

AssetRef AssetManager::LoadAsset(const std::string &filename)
{
    using namespace json;

    ResId rsid;
    int num1, num2, num3, num4;
    std::string str1;
    JSON json, param1, param2, param3, map;
    std::vector<std::pair<std::size_t, std::vector<std::string>>> __files;
    std::filesystem::path dir_of_asset;
    Asset* asset;
    std::ifstream input(filename);
    str1.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    input.close();

    json = JSON::Load(str1);
    if(json.IsNull())
    {
        ronin_err("Can not load asset file");
        return nullptr;
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
        ronin_err("Can not load asset file");
        return nullptr;
    }
    dir_of_asset = filename;
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
                ronin_err("Can not load asset file");
                return nullptr;
            }
            for(auto &source0 : sources.ArrayRange())
            {
                // #1. Check Resources
                param1 = source0["name"];
                param2 = source0["ref"];
                num2 = 0;
                if(param1.JSONType() != JSON::Class::String ||
                   (param2.JSONType() == JSON::Class::Array && (++num2) && std::any_of(param2.ArrayRange().begin(), param2.ArrayRange().end(), [](const auto &test) -> bool const { return test.JSONType() != JSON::Class::String; }) || !num2 && param2.JSONType() != JSON::Class::String))
                {
                    ronin_err(("Invalid resource \"" + param1.ToString() + "\" in Asset file " + filename).c_str());
                    continue;
                }

                // #2. Check conflicts.
                size_t hash_it = stringHasher(param1.ToString());
                if(std::end(__files) != std::find_if(std::begin(__files), std::end(__files), [&hash_it](auto &ref) { return hash_it == ref.first; }))
                {
                    ronin_err(("Conflict resource name \"" + param1.ToString() + "\"").c_str());
                    continue;
                }

                std::vector<std::string> _strings;
                if(param2.JSONType() == JSON::Class::Array)
                {
                    for(JSON &arv : param2.ArrayRange())
                    {
                        _strings.emplace_back((dir_of_asset / arv.ToString()).string());
                    }
                }
                else
                {
                    _strings.emplace_back((dir_of_asset / param2.ToString()).string());
                }

                __files.emplace_back(std::make_pair(hash_it, _strings));
            }

            RoninMemory::alloc_self(asset);
            RoninMemory::alloc_self(asset->impl);
            for(auto &pair : __files)
            {
                switch(num1)
                {
                    /*Sprites*/
                    case 0:
                        rsid = Resources::LoadImage(pair.second.front(), false);
                        break;
                    /*AudioClip*/
                    case 1:
                        rsid = Resources::LoadAudioClip(pair.second.front(), false);
                        break;
                    /*MusicClip*/
                    case 2:
                        rsid = Resources::LoadMusicClip(pair.second.front(), false);
                        break;
                    default:
                        rsid = RES_INVALID;
                        break;
                }

                if(rsid == RES_INVALID)
                {
                    ronin_warn(("Invalid resource " + pair.second.front()).c_str());
                    continue;
                }

                switch(num1)
                {
                    /*Sprites*/
                    case 0:
                        asset->impl->index = AssetIndex::AS_SPRITE;

                        if(asset->impl->udata._sprites == nullptr)
                            RoninMemory::alloc_self(asset->impl->udata._sprites);

                        (*asset->impl->udata._sprites)[pair.first].push_back(RefNoFree(Primitive::CreateSpriteFrom(Resources::GetImageSource(rsid))));
                        break;
                    /*AudioClip*/
                    case 1:
                        asset->impl->index = AssetIndex::AS_AUDIO;

                        if(asset->impl->udata._audioclips == nullptr)
                            RoninMemory::alloc_self(asset->impl->udata._audioclips);

                        (*asset->impl->udata._audioclips)[pair.first].push_back(rsid);
                        break;
                    /*MusicClip*/
                    case 2:
                        asset->impl->index = AssetIndex::AS_MUSIC;

                        if(asset->impl->udata._mus == nullptr)
                            RoninMemory::alloc_self(asset->impl->udata._mus);

                        (*asset->impl->udata._mus)[pair.first].push_back(rsid);
                        break;
                    default:;
                }
            }
        }
        break;
        case 3:
            // ATLAS RESOURCE TYPE
            std::string _src;
            AtlasRef atlas;
            SpriteRef spr;
            Rect rect;

            param1 = json["mode"];
            if(!param1.IsNull())
            {
                if(param1.JSONType() != JSON::Class::String)
                {
                    ronin_err("mode param is invalid and not contains atlas mode");
                    return nullptr;
                }

                // Identify
                str1 = param1.ToString();
                for(num3 = 0, num2 = sizeof(modeAtlas) / sizeof(modeAtlas[0]); num3 < num2;)
                {
                    if(modeAtlas[num3] == str1)
                        break;
                    ++num3;
                }

                if(num3 == num2)
                {
                    ronin_err("Atlas mode is not identified.");
                    return nullptr;
                }
            }
            else
            {
                num3 = 0; // Default Atlas mode Rect
            }

            // Load Atlas Map

            map = json["map"];
            switch(num3)
            {
                case 0: // RECT
                    if(map.JSONType() != JSON::Class::Array)
                    {
                        ronin_err("Can not load asset file. Map key is not contains Array maps");
                        return nullptr;
                    }
                    num2 = map.size();
                    break;
                case 1: // TILED
                    if(map.JSONType() != JSON::Class::String)
                    {
                        ronin_err("Can not assign mode is Tiled. Map key is not contains Rect value");
                        return nullptr;
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
                            ronin_err("Invalid key value in \"direction\"");
                        }
                    }

                    break;
                default:
                    ronin_err("Invalid atlas mode");
                    return nullptr;
                    break;
            }

            param1 = json["source"];
            if(param1.JSONType() != JSON::Class::String)
            {
                ronin_err(("Invalid atlas key \"source\" is not valid from " + filename).c_str());
                return nullptr;
            }

            _src = dir_of_asset.generic_string();
            _src += Paths::GetPathSeperatorOS();
            _src += param1.ToString();

            rsid = Resources::LoadImage(_src, false);
            if(rsid == RES_INVALID)
            {
                ronin_err(("Atlas image is invalid " + param1.ToString()).c_str());
                return nullptr;
            }

            RoninMemory::alloc_self(asset);
            RoninMemory::alloc_self(asset->impl);
            asset->impl->index = AssetIndex::AS_ATLAS;
            asset->impl->_atlas = std::move(AtlasRef{RoninMemory::alloc<Atlas>()});
            atlas = asset->impl->_atlas;
            atlas->src = Resources::GetImageSource(rsid);

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
                        ronin_err("Is not valid key entry value is Rect");
                        continue;
                    }
                    rect = {}; // set flush
                    std::sscanf(param1.ToString().c_str(), "%d %d %d %d", &rect.x, &rect.y, &rect.w, &rect.h);

                    spr = Primitive::CreateEmptySprite();

                    // TODO: make sprite name spr->name = param1.empty()
                    param1 = param3["sprite"];
                    if(!param1.IsNull())
                    {
                        if(param1.JSONType() != JSON::Class::String)
                        {
                            ronin_warn("Is not valid key entry \"sprite\"");
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
                    spr = Primitive::CreateEmptySprite();
                }

                spr->m_rect = rect;
                spr->surface = atlas->src;

                atlas->_sprites.emplace_back(RefNoFree(spr));

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
    return loaded_assets.emplace_back(asset);
}

bool AssetManager::UnloadAsset(AssetRef asset)
{
    bool retval;
    if(asset.isNull())
        return false;
    RefMarkNull(asset);
    if((retval = asset.isNull()))
    {
        loaded_assets.remove(asset);
    }
    return retval;
}

SpriteRef Asset::GetSprite(const std::string &name)
{
    if(impl->index != AssetIndex::AS_SPRITE || impl->udata._sprites == nullptr)
        return nullptr;
    auto test = impl->udata._sprites->find(stringHasher(name));
    if(test == std::end(*impl->udata._sprites) || test->second.empty() || test->second.size() > 1)
        return nullptr;
    return test->second.front();
}

AudioClip *Asset::GetAudioClip(const std::string &name)
{
    if(impl->index != AssetIndex::AS_AUDIO || impl->udata._audioclips == nullptr)
        return nullptr;
    auto test = impl->udata._audioclips->find(stringHasher(name));
    if(test == std::end(*impl->udata._audioclips) || test->second.empty() || test->second.size() > 1)
        return nullptr;
    return Resources::GetAudioClipSource(test->second.front());
}

AtlasRef Asset::GetAtlasObject()
{
    if(impl->index != AssetIndex::AS_ATLAS)
        return nullptr;
    return impl->_atlas;
}

std::vector<SpriteRef> Asset::GetSprites(const std::string &name)
{
    if(impl->index != AssetIndex::AS_SPRITE || impl->udata._sprites == nullptr)
        return {};
    auto test = impl->udata._sprites->find(stringHasher(name));
    if(test == std::end(*impl->udata._sprites))
        return {};
    return test->second;
}

std::vector<AudioClip *> Asset::GetAudioClips(const std::string &name)
{
    if(impl->index != AssetIndex::AS_AUDIO || impl->udata._audioclips == nullptr)
        return {};
    auto test = impl->udata._audioclips->find(stringHasher(name));
    if(test == std::end(*impl->udata._audioclips))
        return {};
    std::vector<AudioClip *> sources {};
    for(ResId id : test->second)
    {
        sources.push_back(Resources::GetAudioClipSource(id));
    }
    return sources;
}
