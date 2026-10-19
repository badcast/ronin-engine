#pragma once

#include "meta_manager.h"
#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>

namespace RoninEditor
{
    struct AssetMetadata
    {
        std::string guid;
        uint32_t assetId = 0;
        AssetType type = AssetType::Unknown;
        std::filesystem::path fullPath;
        std::filesystem::path relativePath;
        std::filesystem::path metaPath;
        uintmax_t fileSize = 0;
        std::string lastModified;
        std::string filename;
        std::string stem;
        nlohmann::json importerSettings;
    };

    class AssetDatabase
    {
    public:
        static AssetDatabase &GetInstance();

        void Initialize(const std::filesystem::path &projectRoot);
        void Refresh();

        bool HasAssetByGuid(const std::string &guid) const;
        const AssetMetadata *GetMetadataByGuid(const std::string &guid) const;
        const AssetMetadata *GetMetadataById(uint32_t assetId) const;
        const AssetMetadata *GetMetadataByPath(const std::filesystem::path &path) const;

        std::string GetGuidByPath(const std::filesystem::path &path) const;
        std::filesystem::path GetPathByGuid(const std::string &guid) const;
        std::string GetStemByGuid(const std::string &guid) const;
        std::string GetStemByPath(const std::filesystem::path &path) const;

        bool RenameAsset(const std::filesystem::path &oldPath, const std::filesystem::path &newPath);
        bool DeleteAsset(const std::filesystem::path &path);
        bool ImportAsset(const std::filesystem::path &sourceFile, const std::filesystem::path &targetDir);

        const std::unordered_map<std::string, AssetMetadata> &GetAllAssets() const { return m_guidToAsset; }
        const std::filesystem::path &GetProjectRoot() const { return m_projectRoot; }
        const std::filesystem::path &GetAssetsRoot() const { return m_assetsRoot; }

    private:
        AssetDatabase() = default;
        ~AssetDatabase() = default;
        AssetDatabase(const AssetDatabase &) = delete;
        AssetDatabase &operator=(const AssetDatabase &) = delete;

        std::filesystem::path normalizeRelPath(const std::filesystem::path &p) const;

        std::filesystem::path m_projectRoot;
        std::filesystem::path m_assetsRoot;

        std::unordered_map<std::string, AssetMetadata> m_guidToAsset;
        std::unordered_map<uint32_t, std::string> m_idToGuid;
        std::unordered_map<std::string, std::string> m_pathToGuid;
    };

} // namespace RoninEditor
