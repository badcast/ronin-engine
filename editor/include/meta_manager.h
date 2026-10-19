#pragma once

#include <string>
#include <filesystem>
#include <cstdint>
#include <SimpleJSON/json.hpp>

namespace RoninEditor
{
    enum class AssetType
    {
        Unknown,
        Folder,
        World,
        Texture,
        Sprite,
        Audio,
        Script,
        Prefab,
        Font,
        Config,
        Terrain,
        Video
    };

    struct MetaFile
    {
        int formatVersion = 1;
        std::string guid;
        uint32_t assetId = 0;
        AssetType type = AssetType::Unknown;
        std::string sourceFilename;
        json::JSON importerSettings;
    };

    class MetaManager
    {
    public:
        // Generate persistent 128-bit GUID (32 hex characters lowercase)
        static std::string GenerateGuid();

        // 32-bit integer ID for runtime lookups
        static uint32_t CalculateAssetId(const std::string &pathOrGuid);

        // Type conversion
        static std::string AssetTypeToString(AssetType type);
        static AssetType StringToAssetType(const std::string &str);
        static AssetType DetermineAssetType(const std::filesystem::path &path);
        static const char *GetAssetTypeBadge(AssetType type);

        // File operations
        static bool SaveMetaFile(const std::filesystem::path &metaPath, const MetaFile &meta);
        static bool LoadMetaFile(const std::filesystem::path &metaPath, MetaFile &outMeta);

        // Ensures that <assetPath>.meta exists, creates it if missing, or loads it
        static bool EnsureMetaFile(const std::filesystem::path &assetPath, MetaFile &outMeta);
    };

} // namespace RoninEditor
