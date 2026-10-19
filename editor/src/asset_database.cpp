#include "asset_database.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace RoninEditor
{
    AssetDatabase &AssetDatabase::GetInstance()
    {
        static AssetDatabase instance;
        return instance;
    }

    std::filesystem::path AssetDatabase::normalizeRelPath(const std::filesystem::path &p) const
    {
        std::error_code ec;
        std::filesystem::path rel = p;
        if (p.is_absolute() && !m_projectRoot.empty())
        {
            rel = std::filesystem::relative(p, m_projectRoot, ec);
            if (ec) rel = p;
        }

        std::string s = rel.generic_string();
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    void AssetDatabase::Initialize(const std::filesystem::path &projectRoot)
    {
        m_projectRoot = projectRoot;
        m_assetsRoot = projectRoot / "Assets";
        Refresh();
    }

    void AssetDatabase::Refresh()
    {
        m_guidToAsset.clear();
        m_idToGuid.clear();
        m_pathToGuid.clear();

        if (m_assetsRoot.empty() || !std::filesystem::exists(m_assetsRoot))
            return;

        std::error_code ec;
        for (const auto &entry : std::filesystem::recursive_directory_iterator(m_assetsRoot, ec))
        {
            if (entry.is_regular_file())
            {
                const std::filesystem::path &filePath = entry.path();
                if (filePath.extension() == ".meta")
                    continue;

                MetaFile meta;
                if (MetaManager::EnsureMetaFile(filePath, meta))
                {
                    AssetMetadata md;
                    md.guid = meta.guid;
                    md.assetId = meta.assetId;
                    md.type = meta.type;
                    md.fullPath = filePath;
                    md.metaPath = filePath.string() + ".meta";
                    md.filename = filePath.filename().string();
                    md.stem = filePath.stem().string();
                    md.fileSize = entry.file_size(ec);
                    md.importerSettings = meta.importerSettings;

                    std::error_code ecRel;
                    md.relativePath = std::filesystem::relative(filePath, m_projectRoot, ecRel);
                    if (ecRel) md.relativePath = filePath;

                    m_guidToAsset[md.guid] = md;
                    m_idToGuid[md.assetId] = md.guid;

                    std::string normRel = normalizeRelPath(filePath).generic_string();
                    m_pathToGuid[normRel] = md.guid;
                }
            }
        }
    }

    bool AssetDatabase::HasAssetByGuid(const std::string &guid) const
    {
        return m_guidToAsset.find(guid) != m_guidToAsset.end();
    }

    const AssetMetadata *AssetDatabase::GetMetadataByGuid(const std::string &guid) const
    {
        auto it = m_guidToAsset.find(guid);
        if (it != m_guidToAsset.end())
            return &it->second;
        return nullptr;
    }

    const AssetMetadata *AssetDatabase::GetMetadataById(uint32_t assetId) const
    {
        auto itId = m_idToGuid.find(assetId);
        if (itId != m_idToGuid.end())
            return GetMetadataByGuid(itId->second);
        return nullptr;
    }

    const AssetMetadata *AssetDatabase::GetMetadataByPath(const std::filesystem::path &path) const
    {
        std::string normRel = normalizeRelPath(path).generic_string();
        auto it = m_pathToGuid.find(normRel);
        if (it != m_pathToGuid.end())
            return GetMetadataByGuid(it->second);

        // Try direct filename lookup if relative path did not match
        for (const auto &pair : m_guidToAsset)
        {
            if (pair.second.filename == path.filename().string())
                return &pair.second;
        }

        return nullptr;
    }

    std::string AssetDatabase::GetGuidByPath(const std::filesystem::path &path) const
    {
        const AssetMetadata *md = GetMetadataByPath(path);
        return md ? md->guid : "";
    }

    std::filesystem::path AssetDatabase::GetPathByGuid(const std::string &guid) const
    {
        const AssetMetadata *md = GetMetadataByGuid(guid);
        return md ? md->fullPath : std::filesystem::path{};
    }

    std::string AssetDatabase::GetStemByGuid(const std::string &guid) const
    {
        const AssetMetadata *md = GetMetadataByGuid(guid);
        return md ? md->stem : "";
    }

    std::string AssetDatabase::GetStemByPath(const std::filesystem::path &path) const
    {
        const AssetMetadata *md = GetMetadataByPath(path);
        if (md && !md->stem.empty())
            return md->stem;
        return path.stem().string();
    }

    bool AssetDatabase::RenameAsset(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
    {
        std::error_code ec;
        if (!std::filesystem::exists(oldPath, ec))
            return false;

        std::filesystem::rename(oldPath, newPath, ec);
        if (ec) return false;

        std::filesystem::path oldMeta = oldPath.string() + ".meta";
        std::filesystem::path newMeta = newPath.string() + ".meta";
        if (std::filesystem::exists(oldMeta, ec))
        {
            std::filesystem::rename(oldMeta, newMeta, ec);
        }

        Refresh();
        return true;
    }

    bool AssetDatabase::DeleteAsset(const std::filesystem::path &path)
    {
        std::error_code ec;
        if (std::filesystem::exists(path, ec))
        {
            std::filesystem::remove(path, ec);
        }

        std::filesystem::path metaPath = path.string() + ".meta";
        if (std::filesystem::exists(metaPath, ec))
        {
            std::filesystem::remove(metaPath, ec);
        }

        Refresh();
        return true;
    }

    bool AssetDatabase::ImportAsset(const std::filesystem::path &sourceFile, const std::filesystem::path &targetDir)
    {
        std::error_code ec;
        if (!std::filesystem::exists(sourceFile, ec) || !std::filesystem::exists(targetDir, ec))
            return false;

        std::filesystem::path targetFile = targetDir / sourceFile.filename();
        std::filesystem::copy_file(sourceFile, targetFile, std::filesystem::copy_options::overwrite_existing, ec);
        if (ec) return false;

        MetaFile meta;
        MetaManager::EnsureMetaFile(targetFile, meta);
        Refresh();
        return true;
    }

} // namespace RoninEditor
