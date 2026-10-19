#include <filesystem>

#include "ronin.h"
#include "ronin_debug.h"

#include <nlohmann/json.hpp>

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
    using json = nlohmann::json;

    ResId rsid;
    int num1, num2, num3, num4;
    std::string str1;
    json jRoot, param1, param2, param3, map;
    std::vector<std::pair<std::size_t, std::vector<std::string>>> __files;
    std::filesystem::path dir_of_asset;
    Asset* asset;
    std::ifstream input(filename);
    str1.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    input.close();

    jRoot = json::parse(str1, nullptr, false);
    if(jRoot.is_discarded() || jRoot.is_null() || !jRoot.contains("class") || !jRoot["class"].is_string())
    {
        ronin_err("Can not load asset file");
        return nullptr;
    }

    num2 = sizeof(resClass) / sizeof(resClass[0]);
    str1 = jRoot["class"].get<std::string>();
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
            if(!jRoot.contains("source") || !jRoot["source"].is_array())
            {
                ronin_err("Can not load asset file");
                return nullptr;
            }
            json &sources = jRoot["source"];
            for(auto &source0 : sources)
            {
                // #1. Check Resources
                if(!source0.is_object() || !source0.contains("name") || !source0.contains("ref"))
                    continue;
                param1 = source0["name"];
                param2 = source0["ref"];
                num2 = 0;
                if(!param1.is_string() ||
                   (param2.is_array() && (++num2) && std::any_of(param2.begin(), param2.end(), [](const auto &test) -> bool const { return !test.is_string(); }) || !num2 && !param2.is_string()))
                {
                    ronin_err(("Invalid resource \"" + (param1.is_string() ? param1.get<std::string>() : std::string()) + "\" in Asset file " + filename).c_str());
                    continue;
                }

                // #2. Check conflicts.
                std::string param1Str = param1.get<std::string>();
                size_t hash_it = stringHasher(param1Str);
                if(std::end(__files) != std::find_if(std::begin(__files), std::end(__files), [&hash_it](auto &ref) { return hash_it == ref.first; }))
                {
                    ronin_err(("Conflict resource name \"" + param1Str + "\"").c_str());
                    continue;
                }

                std::vector<std::string> _strings;
                if(param2.is_array())
                {
                    for(const auto &arv : param2)
                    {
                        if(arv.is_string())
                            _strings.emplace_back((dir_of_asset / arv.get<std::string>()).string());
                    }
                }
                else if(param2.is_string())
                {
                    _strings.emplace_back((dir_of_asset / param2.get<std::string>()).string());
                }

                __files.emplace_back(std::make_pair(hash_it, _strings));
            }

            RoninMemory::alloc_self(asset);
            RoninMemory::alloc_self(asset->impl);
            for(auto &pair : __files)
            {
                for(const auto &filePath : pair.second)
                {
                    switch(num1)
                    {
                        /*Sprites*/
                        case 0:
                            rsid = Resources::LoadImage(filePath, false);
                            break;
                        /*AudioClip*/
                        case 1:
                            rsid = Resources::LoadAudioClip(filePath, false);
                            break;
                        /*MusicClip*/
                        case 2:
                            rsid = Resources::LoadMusicClip(filePath, false);
                            break;
                        default:
                            rsid = RES_INVALID;
                            break;
                    }

                    if(rsid == RES_INVALID)
                    {
                        ronin_warn(("Invalid resource " + filePath).c_str());
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
        }
        break;
        case 3:
            // ATLAS RESOURCE TYPE
            std::string _src;
            AtlasRef atlas;
            SpriteRef spr;
            Rect rect;

            if(jRoot.contains("mode") && !jRoot["mode"].is_null())
            {
                param1 = jRoot["mode"];
                if(!param1.is_string())
                {
                    ronin_err("mode param is invalid and not contains atlas mode");
                    return nullptr;
                }

                // Identify
                str1 = param1.get<std::string>();
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
            if(!jRoot.contains("map"))
            {
                ronin_err("Can not load asset file. Missing map key");
                return nullptr;
            }
            map = jRoot["map"];
            switch(num3)
            {
                case 0: // RECT
                    if(!map.is_array())
                    {
                        ronin_err("Can not load asset file. Map key is not contains Array maps");
                        return nullptr;
                    }
                    num2 = static_cast<int>(map.size());
                    break;
                case 1: // TILED
                    if(!map.is_string())
                    {
                        ronin_err("Can not assign mode is Tiled. Map key is not contains Rect value");
                        return nullptr;
                    }

                    rect = {0, 0, -1, -1}; // set flush
                    std::sscanf(map.get<std::string>().c_str(), "%d %d", &rect.w, &rect.h);

                    num4 = 0; // Set default direction (Right)
                    if(jRoot.contains("direction") && jRoot["direction"].is_string())
                    {
                        param1 = jRoot["direction"];
                        str1 = param1.get<std::string>();
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

            if(!jRoot.contains("source") || !jRoot["source"].is_string())
            {
                ronin_err(("Invalid atlas key \"source\" is not valid from " + filename).c_str());
                return nullptr;
            }
            param1 = jRoot["source"];

            _src = dir_of_asset.generic_string();
            _src += Paths::GetPathSeperatorOS();
            _src += param1.get<std::string>();

            rsid = Resources::LoadImage(_src, false);
            if(rsid == RES_INVALID)
            {
                ronin_err(("Atlas image is invalid " + param1.get<std::string>()).c_str());
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
                    if(!param3.contains("value") || !param3["value"].is_string())
                    {
                        ronin_err("Is not valid key entry value is Rect");
                        continue;
                    }
                    param1 = param3["value"];
                    rect = {}; // set flush
                    std::sscanf(param1.get<std::string>().c_str(), "%d %d %d %d", &rect.x, &rect.y, &rect.w, &rect.h);

                    spr = Primitive::CreateEmptySprite();

                    // TODO: make sprite name spr->name = param1.empty()
                    if(param3.contains("sprite") && !param3["sprite"].is_null())
                    {
                        param1 = param3["sprite"];
                        if(!param1.is_string())
                        {
                            ronin_warn("Is not valid key entry \"sprite\"");
                        }
                        else
                        {
                            spr->name = param1.get<std::string>();
                        }
                    }
                    if(spr->name.empty())
                    {
                        spr->name = "sprite-" + std::to_string(num1);
                    }
                }
                // TILED
                else
                {
                    spr = Primitive::CreateEmptySprite();
                    spr->name = "sprite-" + std::to_string(num1);
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
                            rect.x += rect.w;
                            if(rect.x > x * rect.w)
                            {
                                rect.x = 0;
                                rect.y += rect.h;
                            }
                            break;

                        case 1: // Down
                            rect.y += rect.h;
                            if(rect.y > y * rect.h)
                            {
                                rect.y = 0;
                                rect.x += rect.w;
                            }
                            break;

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
    if(!impl || impl->index != AssetIndex::AS_SPRITE || impl->udata._sprites == nullptr)
        return nullptr;
    auto test = impl->udata._sprites->find(stringHasher(name));
    if(test == std::end(*impl->udata._sprites) || test->second.empty() || test->second.size() > 1)
        return nullptr;
    return test->second.front();
}

AudioClip *Asset::GetAudioClip(const std::string &name)
{
    if(!impl || impl->index != AssetIndex::AS_AUDIO || impl->udata._audioclips == nullptr)
        return nullptr;
    auto test = impl->udata._audioclips->find(stringHasher(name));
    if(test == std::end(*impl->udata._audioclips) || test->second.empty() || test->second.size() > 1)
        return nullptr;
    return Resources::GetAudioClipSource(test->second.front());
}

AtlasRef Asset::GetAtlasObject()
{
    if(!impl || impl->index != AssetIndex::AS_ATLAS)
        return nullptr;
    return impl->_atlas;
}

std::vector<SpriteRef> Asset::GetSprites(const std::string &name)
{
    if(!impl || impl->index != AssetIndex::AS_SPRITE || impl->udata._sprites == nullptr)
        return {};
    auto test = impl->udata._sprites->find(stringHasher(name));
    if(test == std::end(*impl->udata._sprites))
        return {};
    return test->second;
}

std::vector<AudioClip *> Asset::GetAudioClips(const std::string &name)
{
    if(!impl || impl->index != AssetIndex::AS_AUDIO || impl->udata._audioclips == nullptr)
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
