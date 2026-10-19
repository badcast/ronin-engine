#include "meta_manager.h"
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>

namespace RoninEditor
{
    std::string MetaManager::GenerateGuid()
    {
        // 128-bit random hex string (32 characters, Unity format)
        static std::random_device rd;
        static std::mt19937_64 gen1(rd() ^ std::chrono::high_resolution_clock::now().time_since_epoch().count());
        static std::mt19937_64 gen2(rd() ^ (std::chrono::high_resolution_clock::now().time_since_epoch().count() >> 16));

        uint64_t part1 = gen1();
        uint64_t part2 = gen2();

        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << part1
           << std::setfill('0') << std::setw(16) << part2;
        return ss.str();
    }

    uint32_t MetaManager::CalculateAssetId(const std::string &pathOrGuid)
    {
        uint32_t hash = 2166136261u;
        for (char c : pathOrGuid)
        {
            hash ^= static_cast<uint8_t>(std::tolower(static_cast<unsigned char>(c)));
            hash *= 16777619u;
        }
        return hash ? hash : 1;
    }

    std::string MetaManager::AssetTypeToString(AssetType type)
    {
        switch (type)
        {
        case AssetType::World:      return "World";
        case AssetType::Texture:    return "Texture2D";
        case AssetType::Sprite:     return "Sprite";
        case AssetType::Audio:      return "AudioClip";
        case AssetType::Code:       return "Code";
        case AssetType::TextStream: return "TextStream";
        case AssetType::Prefab:     return "Prefab";
        case AssetType::Terrain:    return "Terrain";
        case AssetType::Video:      return "VideoClip";
        case AssetType::Font:       return "Font";
        case AssetType::Config:     return "Config";
        case AssetType::Folder:     return "Folder";
        default:                    return "Unknown";
        }
    }

    AssetType MetaManager::StringToAssetType(const std::string &str)
    {
        if (str == "World")      return AssetType::World;
        if (str == "Texture2D")  return AssetType::Texture;
        if (str == "Sprite")     return AssetType::Sprite;
        if (str == "AudioClip")  return AssetType::Audio;
        if (str == "Code" || str == "Script") return AssetType::Code;
        if (str == "TextStream") return AssetType::TextStream;
        if (str == "Prefab")     return AssetType::Prefab;
        if (str == "Terrain")    return AssetType::Terrain;
        if (str == "VideoClip")  return AssetType::Video;
        if (str == "Font")       return AssetType::Font;
        if (str == "Config")     return AssetType::Config;
        if (str == "Folder")     return AssetType::Folder;
        return AssetType::Unknown;
    }

    AssetType MetaManager::DetermineAssetType(const std::filesystem::path &path)
    {
        if (std::filesystem::is_directory(path))
            return AssetType::Folder;

        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".world") return AssetType::World;
        if (ext == ".sprite") return AssetType::Sprite;
        if (ext == ".prefab") return AssetType::Prefab;
        if (ext == ".terrain") return AssetType::Terrain;
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") return AssetType::Texture;
        if (ext == ".wav" || ext == ".ogg" || ext == ".mp3" || ext == ".flac") return AssetType::Audio;
        if (ext == ".mp4" || ext == ".avi" || ext == ".mkv" || ext == ".webm") return AssetType::Video;
        if (ext == ".ttf" || ext == ".otf") return AssetType::Font;
        if (ext == ".ronin") return AssetType::Config;

        // .h, .hpp, .c, .cpp, .cxx, .inc files are strictly Code (System source code)
        if (ext == ".h" || ext == ".hpp" || ext == ".c" || ext == ".cpp" || ext == ".cxx" || ext == ".inc")
            return AssetType::Code;

        // Text data streams
        if (ext == ".txt" || ext == ".json" || ext == ".log" || ext == ".csv" || ext == ".tsv" || ext == ".lua" || ext == ".shader")
            return AssetType::TextStream;

        return AssetType::Unknown;
    }

    const char *MetaManager::GetAssetTypeBadge(AssetType type)
    {
        switch (type)
        {
        case AssetType::World:      return "World";
        case AssetType::Texture:    return "Texture2D";
        case AssetType::Sprite:     return "Sprite";
        case AssetType::Audio:      return "AudioClip";
        case AssetType::Code:       return "Code";
        case AssetType::TextStream: return "TextStream";
        case AssetType::Prefab:     return "Prefab";
        case AssetType::Terrain:    return "Terrain";
        case AssetType::Video:      return "VideoClip";
        case AssetType::Font:       return "Font";
        case AssetType::Config:     return "Config";
        case AssetType::Folder:     return "Folder";
        default:                    return "Asset";
        }
    }

    bool MetaManager::SaveMetaFile(const std::filesystem::path &metaPath, const MetaFile &meta)
    {
        nlohmann::json j = nlohmann::json::object();
        j["fileFormatVersion"] = meta.formatVersion;
        j["guid"] = meta.guid;
        j["assetId"] = static_cast<long long>(meta.assetId);
        j["assetType"] = AssetTypeToString(meta.type);

        nlohmann::json imp = nlohmann::json::object();
        imp["version"] = 1;
        imp["sourceFile"] = meta.sourceFilename;
        if (meta.importerSettings.is_object())
        {
            imp["settings"] = meta.importerSettings;
        }
        else
        {
            imp["settings"] = nlohmann::json::object();
        }
        j["importer"] = imp;

        std::ofstream ofs(metaPath);
        if (!ofs.is_open())
            return false;

        ofs << j.dump(2) << "\n";
        return ofs.good();
    }

    bool MetaManager::LoadMetaFile(const std::filesystem::path &metaPath, MetaFile &outMeta)
    {
        std::ifstream ifs(metaPath);
        if (!ifs.is_open())
            return false;

        nlohmann::json j = nlohmann::json::parse(ifs, nullptr, false);
        if (j.is_discarded() || !j.is_object())
            return false;

        if (j.contains("fileFormatVersion") && j["fileFormatVersion"].is_number())
            outMeta.formatVersion = j["fileFormatVersion"].get<int>();
        else
            outMeta.formatVersion = 1;

        if (j.contains("guid") && j["guid"].is_string())
            outMeta.guid = j["guid"].get<std::string>();
        else
            outMeta.guid = GenerateGuid();

        if (j.contains("assetId") && j["assetId"].is_number())
            outMeta.assetId = static_cast<uint32_t>(j["assetId"].get<uint64_t>());
        else
            outMeta.assetId = CalculateAssetId(outMeta.guid);

        if (j.contains("assetType") && j["assetType"].is_string())
            outMeta.type = StringToAssetType(j["assetType"].get<std::string>());
        else
            outMeta.type = AssetType::Unknown;

        if (j.contains("importer") && j["importer"].is_object())
        {
            const auto &imp = j["importer"];
            if (imp.contains("sourceFile") && imp["sourceFile"].is_string())
                outMeta.sourceFilename = imp["sourceFile"].get<std::string>();
            if (imp.contains("settings") && imp["settings"].is_object())
                outMeta.importerSettings = imp["settings"];
            else
                outMeta.importerSettings = nlohmann::json::object();
        }
        else
        {
            outMeta.importerSettings = nlohmann::json::object();
        }

        return true;
    }

    bool MetaManager::EnsureMetaFile(const std::filesystem::path &assetPath, MetaFile &outMeta)
    {
        if (assetPath.extension() == ".meta")
            return false;

        std::filesystem::path metaPath = assetPath.string() + ".meta";
        if (std::filesystem::exists(metaPath))
        {
            if (LoadMetaFile(metaPath, outMeta))
            {
                outMeta.sourceFilename = assetPath.filename().string();
                return true;
            }
        }

        // Generate new .meta file
        outMeta.formatVersion = 1;
        outMeta.guid = GenerateGuid();
        outMeta.type = DetermineAssetType(assetPath);
        outMeta.assetId = CalculateAssetId(outMeta.guid);
        outMeta.sourceFilename = assetPath.filename().string();

        nlohmann::json settings = nlohmann::json::object();
        switch (outMeta.type)
        {
        case AssetType::Texture:
            settings["filterMode"] = "Point";
            settings["wrapMode"] = "Clamp";
            settings["pixelsPerUnit"] = 100.0;
            settings["sRGB"] = true;
            break;
        case AssetType::Audio:
            settings["loadType"] = "DecompressOnLoad";
            settings["sampleRate"] = 44100;
            settings["forceToMono"] = false;
            break;
        case AssetType::Sprite:
            settings["pixelsPerUnit"] = 100.0;
            break;
        case AssetType::Code:
            settings["language"] = "C++";
            settings["encoding"] = "UTF-8";
            settings["systemSource"] = true;
            break;
        case AssetType::TextStream:
            settings["encoding"] = "UTF-8";
            break;
        case AssetType::Prefab:
            settings["version"] = 1;
            break;
        case AssetType::Terrain:
            settings["gridWidth"] = 32;
            settings["gridHeight"] = 32;
            settings["cellSize"] = 1.0;
            break;
        default:
            break;
        }
        outMeta.importerSettings = settings;

        return SaveMetaFile(metaPath, outMeta);
    }

} // namespace RoninEditor
