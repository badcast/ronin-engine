#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <functional>
#include "editor_state.h"
#include "meta_manager.h"
#include "editor_icons.h"

namespace RoninEditor
{

    struct AssetEntry
    {
        uint32_t assetId = 0;
        std::string filename;
        std::filesystem::path fullPath;
        std::filesystem::path relativePath;
        AssetType type = AssetType::Unknown;
        bool isDirectory = false;
        std::uintmax_t fileSize = 0;
    };

    class AssetBrowser
    {
    public:
        AssetBrowser();
        ~AssetBrowser();

        void Render(EditorState &state);

        void SetCurrentDirectory(const std::filesystem::path &dir);
        const std::filesystem::path &GetCurrentDirectory() const { return m_currentDir; }

        void PingAsset(const std::filesystem::path &path);
        void OpenNativeImportDialog(EditorState &state);
        static uint32_t CalculateAssetId(const std::string &relPath);

        // Callback when user requests to open Sprite Editor for a sprite/texture
        std::function<void(const std::filesystem::path &)> onOpenSpriteEditor;

    private:
        void refreshCurrentDirectory();
        void renderDirectoryTree(const std::filesystem::path &dir);
        void renderBreadcrumbs(EditorState &state);
        void renderFileGrid(EditorState &state);
        void renderContextMenu(EditorState &state, const AssetEntry *targetItem);
        void renderImportModal(EditorState &state);

        AssetType determineAssetType(const std::filesystem::path &path) const;
        const char *getAssetTypeIcon(AssetType type) const;
        const char *getAssetTypeName(AssetType type) const;
        IconType getAssetIconType(AssetType type, bool isDirectory) const;
        ImU32 getAssetIconColor(AssetType type, bool isDirectory) const;
        void createPrefabFromObject(EditorState &state, const EditorObject &obj, const std::filesystem::path &targetDir);

        std::filesystem::path m_assetsRoot;
        std::filesystem::path m_currentDir;
        std::vector<AssetEntry> m_entries;

        char m_searchFilter[128] = "";
        char m_renameBuffer[128] = "";
        std::string m_renamingFile;

        // View options
        bool m_isGridView = true;
        float m_thumbnailSize = 80.0f;
        float m_treeWidth = 200.0f;

        // Import Asset Dialog state
        bool m_showImportDialog = false;
        char m_importSourcePath[512] = "";
        std::string m_importErrorMessage;

        // Multiselect state
        std::vector<std::filesystem::path> m_selectedPaths;
        std::filesystem::path m_lastSelectedPath;
        bool isPathSelected(const std::filesystem::path &p) const;
        void selectPath(const std::filesystem::path &p, bool addToSelection = false);
        void clearSelection();

        std::vector<std::filesystem::path> m_pendingDeletions;
    };
} // namespace RoninEditor
