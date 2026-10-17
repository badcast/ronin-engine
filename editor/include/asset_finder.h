#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <functional>
#include "editor_state.h"
#include "asset_browser.h"

namespace RoninEditor
{
    struct AssetFinderItem
    {
        uint32_t assetId = 0;
        std::string filename;
        std::filesystem::path fullPath;
        std::filesystem::path relativePath;
        AssetType type = AssetType::Unknown;
        std::uintmax_t fileSize = 0;
    };

    class AssetFinder
    {
    public:
        AssetFinder();
        ~AssetFinder();

        void Open();
        void Close();
        bool IsOpen() const { return m_isOpen; }
        void Toggle() { if (m_isOpen) Close(); else Open(); }

        void Render(EditorState &state, AssetBrowser &assetBrowser, std::function<void(const std::filesystem::path &)> onOpenSpriteEditor);

    private:
        void rescan(const std::filesystem::path &projectRoot);
        AssetType determineType(const std::filesystem::path &path) const;

        bool m_isOpen = false;
        bool m_shouldOpenPopupTrigger = false;
        bool m_shouldFocusInput = false;
        char m_filter[256] = "";
        int m_selectedCategory = 0; // 0 = All, 1 = Worlds, 2 = Sprites, 3 = Textures, 4 = Audio, 5 = Scripts, 6 = Prefabs
        int m_selectedIndex = -1;

        std::filesystem::path m_scannedRoot;
        std::vector<AssetFinderItem> m_items;
    };
} // namespace RoninEditor
