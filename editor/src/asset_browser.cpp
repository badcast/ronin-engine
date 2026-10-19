#include "asset_browser.h"
#include "project_manager.h"
#include "asset_database.h"
#include "editor_icons.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <nlohmann/json.hpp>

namespace RoninEditor
{
    AssetBrowser::AssetBrowser() = default;
    AssetBrowser::~AssetBrowser() = default;

    AssetType AssetBrowser::determineAssetType(const std::filesystem::path &path) const
    {
        return MetaManager::DetermineAssetType(path);
    }

    void AssetBrowser::createPrefabFromObject(EditorState &state, const EditorObject &obj, const std::filesystem::path &targetDir)
    {
        std::string baseName = obj.name.empty() ? "GameObject" : obj.name;
        std::string cleanName;
        for (char c : baseName)
        {
            if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
                cleanName += '_';
            else
                cleanName += c;
        }
        if (cleanName.empty()) cleanName = "GameObject";

        std::filesystem::path prefabPath = targetDir / (cleanName + ".prefab");
        int counter = 1;
        while (std::filesystem::exists(prefabPath))
        {
            prefabPath = targetDir / (cleanName + "_" + std::to_string(counter++) + ".prefab");
        }

        if (state.savePrefab(obj, prefabPath.string()))
        {
            refreshCurrentDirectory();
            selectPath(prefabPath, false);
            state.logInfo("Created Prefab '" + prefabPath.filename().string() + "' from GameObject '" + obj.name + "'");
        }
    }

    const char *AssetBrowser::getAssetTypeName(AssetType type) const
    {
        switch (type)
        {
            case AssetType::Folder: return "Folder";
            case AssetType::World: return "World";
            case AssetType::Sprite: return "Sprite";
            case AssetType::Texture: return "Texture";
            case AssetType::Audio: return "Audio";
            case AssetType::Code: return "Code";
            case AssetType::TextStream: return "Text";
            case AssetType::Prefab: return "Prefab";
            case AssetType::Terrain: return "Terrain";
            case AssetType::Video: return "Video";
            case AssetType::Font: return "Font";
            case AssetType::Config: return "Config";
            default: return "File";
        }
    }

    IconType AssetBrowser::getAssetIconType(AssetType type, bool isDirectory) const
    {
        if (isDirectory || type == AssetType::Folder) return IconType::Folder;
        switch (type)
        {
            case AssetType::World: return IconType::World;
            case AssetType::Sprite: return IconType::Sprite;
            case AssetType::Texture: return IconType::Texture;
            case AssetType::Audio: return IconType::Audio;
            case AssetType::Code: return IconType::Code;
            case AssetType::TextStream: return IconType::TextStream;
            case AssetType::Prefab: return IconType::Prefab;
            case AssetType::Terrain: return IconType::Terrain;
            case AssetType::Video: return IconType::Video;
            case AssetType::Font: return IconType::Font;
            case AssetType::Config: return IconType::Config;
            default: return IconType::File;
        }
    }

    ImU32 AssetBrowser::getAssetIconColor(AssetType type, bool isDirectory) const
    {
        if (isDirectory || type == AssetType::Folder) return EditorPalette::IconFolder;
        switch (type)
        {
            case AssetType::World: return EditorPalette::IconWorld;
            case AssetType::Sprite: return EditorPalette::IconSprite;
            case AssetType::Texture: return EditorPalette::IconTexture;
            case AssetType::Audio: return EditorPalette::IconAudio;
            case AssetType::Code: return EditorPalette::IconScript;
            case AssetType::TextStream: return EditorPalette::TextNormal;
            case AssetType::Prefab: return EditorPalette::IconPrefab;
            case AssetType::Terrain: return EditorPalette::IconTerrain;
            case AssetType::Video: return EditorPalette::IconVideo;
            case AssetType::Font: return EditorPalette::IconFont;
            case AssetType::Config: return EditorPalette::Warning;
            default: return EditorPalette::TextMuted;
        }
    }

    const char *AssetBrowser::getAssetTypeIcon(AssetType type) const
    {
        return getAssetTypeName(type);
    }

    bool AssetBrowser::isPathSelected(const std::filesystem::path &p) const
    {
        return std::find(m_selectedPaths.begin(), m_selectedPaths.end(), p) != m_selectedPaths.end();
    }

    void AssetBrowser::selectPath(const std::filesystem::path &p, bool addToSelection)
    {
        if (!addToSelection)
        {
            m_selectedPaths.clear();
            m_selectedPaths.push_back(p);
            m_lastSelectedPath = p;
        }
        else
        {
            auto it = std::find(m_selectedPaths.begin(), m_selectedPaths.end(), p);
            if (it != m_selectedPaths.end())
            {
                m_selectedPaths.erase(it);
            }
            else
            {
                m_selectedPaths.push_back(p);
                m_lastSelectedPath = p;
            }
        }
    }

    void AssetBrowser::clearSelection()
    {
        m_selectedPaths.clear();
        m_lastSelectedPath.clear();
    }

    void AssetBrowser::SetCurrentDirectory(const std::filesystem::path &dir)
    {
        std::error_code ec;
        std::filesystem::path canonicalDir = std::filesystem::weakly_canonical(dir, ec);
        std::filesystem::path canonicalRoot = std::filesystem::weakly_canonical(m_assetsRoot, ec);

        std::string sDir = canonicalDir.string();
        std::string sRoot = canonicalRoot.string();

        if (sDir.rfind(sRoot, 0) == 0 && std::filesystem::exists(canonicalDir))
        {
            m_currentDir = canonicalDir;
        }
        else
        {
            m_currentDir = m_assetsRoot;
        }
        refreshCurrentDirectory();
    }

    uint32_t AssetBrowser::CalculateAssetId(const std::string &relPath)
    {
        uint32_t hash = 2166136261u;
        for (char c : relPath)
        {
            hash ^= static_cast<uint8_t>(std::tolower(c));
            hash *= 16777619u;
        }
        return hash ? hash : 1;
    }

    void AssetBrowser::PingAsset(const std::filesystem::path &path)
    {
        std::filesystem::path fullPath = path;
        if (!std::filesystem::exists(fullPath))
        {
            auto root = ProjectManager::GetInstance().GetRootPath();
            fullPath = root / path;
        }

        if (std::filesystem::exists(fullPath))
        {
            if (std::filesystem::is_directory(fullPath))
            {
                SetCurrentDirectory(fullPath);
            }
            else
            {
                SetCurrentDirectory(fullPath.parent_path());
                m_selectedPaths.clear();
                m_selectedPaths.push_back(fullPath);
                m_lastSelectedPath = fullPath;
            }
        }
    }

    void AssetBrowser::OpenNativeImportDialog(EditorState &state)
    {
        std::vector<std::filesystem::path> selectedFiles;
#if defined(__linux__)
        std::string title = "Select Assets to Import into RoninEngine";
        std::string cmd = "zenity --file-selection --multiple --separator=\"|\" --title=\"" + title + "\" 2>/dev/null";
        FILE *fp = popen(cmd.c_str(), "r");
        if (fp)
        {
            char buffer[4096];
            std::string output;
            while (fgets(buffer, sizeof(buffer), fp) != nullptr)
            {
                output += buffer;
            }
            pclose(fp);
            while (!output.empty() && (output.back() == '\n' || output.back() == '\r'))
                output.pop_back();

            if (!output.empty())
            {
                std::stringstream ss(output);
                std::string item;
                while (std::getline(ss, item, '|'))
                {
                    if (!item.empty() && std::filesystem::exists(item))
                    {
                        selectedFiles.push_back(item);
                    }
                }
            }
        }
        if (selectedFiles.empty())
        {
            cmd = "kdialog --multiple --separate-output --getopenfilename :allfiles 2>/dev/null";
            fp = popen(cmd.c_str(), "r");
            if (fp)
            {
                char buffer[4096];
                while (fgets(buffer, sizeof(buffer), fp) != nullptr)
                {
                    std::string line = buffer;
                    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
                    if (!line.empty() && std::filesystem::exists(line))
                        selectedFiles.push_back(line);
                }
                pclose(fp);
            }
        }
#endif
        if (!selectedFiles.empty())
        {
            int importedCount = 0;
            std::error_code ec;
            for (const auto &src : selectedFiles)
            {
                std::filesystem::path dest = m_currentDir / src.filename();
                std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing, ec);
                if (!ec)
                {
                    importedCount++;
                    state.logInfo("Imported asset: " + dest.filename().string() + " into " + m_currentDir.string());
                }
                else
                {
                    state.logError("Failed to import " + src.filename().string() + ": " + ec.message());
                }
            }
            if (importedCount > 0)
            {
                refreshCurrentDirectory();
                state.isDirty = true;
            }
        }
        else
        {
            m_showImportDialog = true;
            m_importErrorMessage.clear();
        }
    }

    void AssetBrowser::refreshCurrentDirectory()
    {
        m_entries.clear();
        if (m_currentDir.empty() || !std::filesystem::exists(m_currentDir))
            return;

        std::error_code ec;
        for (const auto &it : std::filesystem::directory_iterator(m_currentDir, ec))
        {
            AssetEntry entry;
            entry.filename = it.path().filename().string();
            entry.fullPath = it.path();
            entry.isDirectory = it.is_directory();
            entry.type = determineAssetType(it.path());

            std::error_code relEc;
            entry.relativePath = std::filesystem::relative(it.path(), m_assetsRoot, relEc);
            if (relEc || entry.relativePath.empty()) entry.relativePath = it.path().filename();
            // Hide hidden files starting with . or .meta files
            if (!entry.filename.empty() && entry.filename[0] == '.')
                continue;
            if (it.path().extension() == ".meta")
                continue;

            if (!entry.isDirectory)
            {
                entry.fileSize = it.file_size(ec);
                MetaFile mf;
                if (MetaManager::EnsureMetaFile(it.path(), mf))
                {
                    entry.assetId = mf.assetId;
                }
            }

            m_entries.push_back(entry);
        }

        // Sort: directories first, then alphabetically
        std::sort(m_entries.begin(), m_entries.end(), [](const AssetEntry &a, const AssetEntry &b) {
            if (a.isDirectory != b.isDirectory)
                return a.isDirectory > b.isDirectory;
            return a.filename < b.filename;
        });
    }

    void AssetBrowser::renderDirectoryTree(const std::filesystem::path &dir)
    {
        std::string dirName = dir.filename().string();
        if (dirName.empty()) dirName = dir.string();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (m_currentDir == dir)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (dir == m_assetsRoot)
        {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        int subDirCount = 0;
        std::error_code ec;
        for (const auto &it : std::filesystem::directory_iterator(dir, ec))
        {
            if (it.is_directory() && it.path().filename().string()[0] != '.')
                subDirCount++;
        }

        if (subDirCount == 0)
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        std::string label = "    " + dirName;
        bool open = ImGui::TreeNodeEx(dir.string().c_str(), flags, "%s", label.c_str());

        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemMax = ImGui::GetItemRectMax();
        float iconSize = 13.0f;
        float iconX = itemMin.x + (subDirCount > 0 ? 20.0f : 8.0f);
        float iconY = itemMin.y + (itemMax.y - itemMin.y - iconSize) * 0.5f;

        EditorIcons::DrawIcon(ImGui::GetWindowDrawList(), ImVec2(iconX, iconY), ImVec2(iconX + iconSize, iconY + iconSize), IconType::Folder, EditorPalette::IconFolder);

        if (ImGui::IsItemClicked(0))
        {
            SetCurrentDirectory(dir);
        }

        if (open && subDirCount > 0)
        {
            std::vector<std::filesystem::path> subdirs;
            for (const auto &it : std::filesystem::directory_iterator(dir, ec))
            {
                if (it.is_directory() && it.path().filename().string()[0] != '.')
                {
                    subdirs.push_back(it.path());
                }
            }
            std::sort(subdirs.begin(), subdirs.end());
            for (const auto &sub : subdirs)
            {
                renderDirectoryTree(sub);
            }
            ImGui::TreePop();
        }
    }

    void AssetBrowser::renderBreadcrumbs(EditorState &state)
    {
        if (m_currentDir.empty()) return;

        // Up button
        if (m_currentDir != m_assetsRoot && m_currentDir.has_parent_path())
        {
            if (EditorIcons::IconButton("##UpDir", IconType::UpDir, ImVec2(24.0f, 22.0f), false, EditorPalette::TextNormal))
            {
                SetCurrentDirectory(m_currentDir.parent_path());
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Go to parent folder");
            ImGui::SameLine();
        }

        // Refresh button
        if (EditorIcons::IconButton("##RefreshAssets", IconType::Refresh, ImVec2(24.0f, 22.0f), false, EditorPalette::TextNormal))
        {
            refreshCurrentDirectory();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Refresh current folder");
        ImGui::SameLine();

        // Import Asset button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.50f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.60f, 0.42f, 1.0f));
        if (ImGui::Button("+ Import Asset..."))
        {
            OpenNativeImportDialog(state);
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Open native file dialog to import assets into this folder");
        ImGui::SameLine();

        // Create Dropdown Menu button
        if (ImGui::Button("+ Create v"))
        {
            ImGui::OpenPopup("CreateAssetQuickMenu");
        }
        if (ImGui::BeginPopup("CreateAssetQuickMenu"))
        {
            renderContextMenu(state, nullptr);
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        // Root breadcrumb: "Assets"
        if (ImGui::SmallButton("Assets"))
        {
            SetCurrentDirectory(m_assetsRoot);
        }
        ImGui::SameLine();
        ImGui::TextDisabled(">");
        ImGui::SameLine();

        // Subdirectories relative to m_assetsRoot
        std::error_code ec;
        std::filesystem::path rel = std::filesystem::relative(m_currentDir, m_assetsRoot, ec);
        if (!rel.empty() && rel != ".")
        {
            std::filesystem::path accum = m_assetsRoot;
            for (const auto &part : rel)
            {
                accum /= part;
                if (ImGui::SmallButton(part.string().c_str()))
                {
                    SetCurrentDirectory(accum);
                }
                ImGui::SameLine();
                ImGui::TextDisabled(">");
                ImGui::SameLine();
            }
        }

        // Right side: Search, View Mode, Thumbnail Zoom
        float rightControlsWidth = 330.0f;
        float rightOffset = ImGui::GetWindowWidth() - rightControlsWidth - 10.0f;
        if (rightOffset > ImGui::GetCursorPosX())
        {
            ImGui::SameLine(rightOffset);
        }

        ImGui::SetNextItemWidth(140.0f);
        ImGui::InputTextWithHint("##SearchAssets", "Search...", m_searchFilter, sizeof(m_searchFilter));
        ImGui::SameLine();

        if (EditorIcons::IconButton("##ViewGrid", IconType::ViewGrid, ImVec2(24.0f, 22.0f), m_isGridView, m_isGridView ? EditorPalette::PrimaryHover : EditorPalette::TextMuted))
        {
            m_isGridView = true;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Grid View");
        ImGui::SameLine();

        if (EditorIcons::IconButton("##ViewList", IconType::ViewList, ImVec2(24.0f, 22.0f), !m_isGridView, !m_isGridView ? EditorPalette::PrimaryHover : EditorPalette::TextMuted))
        {
            m_isGridView = false;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("List View");
        ImGui::SameLine();

        if (m_isGridView)
        {
            ImGui::SetNextItemWidth(80.0f);
            ImGui::SliderFloat("##ThumbSize", &m_thumbnailSize, 56.0f, 120.0f, "%.0f px");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Thumbnail Zoom");
        }
    }

    void AssetBrowser::renderFileGrid(EditorState &state)
    {
        std::filesystem::path pendingDirChange;
        if (m_isGridView)
        {
            float availWidth = ImGui::GetContentRegionAvail().x;
            float cardWidth = m_thumbnailSize;
            float spacing = ImGui::GetStyle().ItemSpacing.x;

            for (size_t i = 0; i < m_entries.size(); ++i)
            {
                const auto &entry = m_entries[i];

                if (m_searchFilter[0] != '\0')
                {
                    std::string fnameLower = entry.filename;
                    std::string filterLower = m_searchFilter;
                    std::transform(fnameLower.begin(), fnameLower.end(), fnameLower.begin(), ::tolower);
                    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
                    if (fnameLower.find(filterLower) == std::string::npos)
                        continue;
                }

                ImGui::PushID(static_cast<int>(i));

                bool isSelected = isPathSelected(entry.fullPath);
                IconType iconKind = getAssetIconType(entry.type, entry.isDirectory);
                ImU32 iconCol = getAssetIconColor(entry.type, entry.isDirectory);
                ImVec4 cardBg = isSelected ? ImVec4(0.24f, 0.42f, 0.72f, 0.95f) : ImVec4(0.18f, 0.20f, 0.24f, 0.65f);

                if (!isSelected)
                {
                    if (entry.isDirectory || entry.type == AssetType::Folder) cardBg = ImVec4(0.25f, 0.20f, 0.12f, 0.65f);
                    else if (entry.type == AssetType::World) cardBg = ImVec4(0.12f, 0.22f, 0.28f, 0.65f);
                    else if (entry.type == AssetType::Sprite) cardBg = ImVec4(0.22f, 0.15f, 0.28f, 0.65f);
                    else if (entry.type == AssetType::Texture) cardBg = ImVec4(0.12f, 0.25f, 0.18f, 0.65f);
                    else if (entry.type == AssetType::Audio) cardBg = ImVec4(0.28f, 0.18f, 0.12f, 0.65f);
                    else if (entry.type == AssetType::Prefab) cardBg = ImVec4(0.14f, 0.22f, 0.32f, 0.65f);
                    else if (entry.type == AssetType::Code) cardBg = ImVec4(0.12f, 0.24f, 0.18f, 0.65f);
                    else if (entry.type == AssetType::Terrain) cardBg = ImVec4(0.16f, 0.25f, 0.14f, 0.65f);
                    else if (entry.type == AssetType::Font) cardBg = ImVec4(0.26f, 0.16f, 0.20f, 0.65f);
                    else if (entry.type == AssetType::Video) cardBg = ImVec4(0.28f, 0.14f, 0.16f, 0.65f);
                }

                // Group representing card
                ImGui::BeginGroup();

                ImGui::PushStyleColor(ImGuiCol_Button, cardBg);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.38f, 0.55f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.48f, 0.70f, 1.0f));

                std::string btnId = "##CardBtn_" + std::to_string(i);
                if (ImGui::Button(btnId.c_str(), ImVec2(cardWidth, m_thumbnailSize * 0.65f)))
                {
                }

                bool isHovered = ImGui::IsItemHovered();
                bool isDoubleClicked = isHovered && ImGui::IsMouseDoubleClicked(0);

                if (ImGui::IsItemClicked(0))
                {
                    ImGuiIO &io = ImGui::GetIO();
                    bool ctrl = io.KeyCtrl;
                    bool shift = io.KeyShift;

                    if (shift && !m_lastSelectedPath.empty())
                    {
                        int idx1 = -1, idx2 = -1;
                        for (int j = 0; j < static_cast<int>(m_entries.size()); ++j)
                        {
                            if (m_entries[j].fullPath == m_lastSelectedPath) idx1 = j;
                            if (m_entries[j].fullPath == entry.fullPath) idx2 = j;
                        }
                        if (idx1 != -1 && idx2 != -1)
                        {
                            int start = std::min(idx1, idx2);
                            int end = std::max(idx1, idx2);
                            if (!ctrl) m_selectedPaths.clear();
                            for (int j = start; j <= end; ++j)
                            {
                                if (!isPathSelected(m_entries[j].fullPath))
                                    m_selectedPaths.push_back(m_entries[j].fullPath);
                            }
                        }
                    }
                    else
                    {
                        selectPath(entry.fullPath, ctrl);
                        if (!entry.isDirectory)
                        {
                            state.selectAsset(entry.fullPath);
                        }
                    }
                }

                // Drag & Drop Target for Folders (Drop GameObject from hierarchy to create prefab in folder)
                if (entry.isDirectory || entry.type == AssetType::Folder)
                {
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_GAMEOBJECT_REPARENT"))
                        {
                            int objId = *(const int *)payload->Data;
                            EditorObject *obj = state.findObject(objId);
                            if (obj)
                            {
                                createPrefabFromObject(state, *obj, entry.fullPath);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                // Drag & Drop Source
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    std::string pathStr = entry.fullPath.string();
                    ImGui::SetDragDropPayload("RONIN_ASSET_PATH", pathStr.c_str(), pathStr.size() + 1);

                    if (entry.type == AssetType::Sprite)
                        ImGui::SetDragDropPayload("ASSET_SPRITE", pathStr.c_str(), pathStr.size() + 1);
                    else if (entry.type == AssetType::Texture)
                        ImGui::SetDragDropPayload("ASSET_TEXTURE", pathStr.c_str(), pathStr.size() + 1);
                    else if (entry.type == AssetType::Audio)
                        ImGui::SetDragDropPayload("ASSET_AUDIO", pathStr.c_str(), pathStr.size() + 1);
                    else if (entry.type == AssetType::World)
                        ImGui::SetDragDropPayload("ASSET_WORLD", pathStr.c_str(), pathStr.size() + 1);
                    else if (entry.type == AssetType::Prefab)
                        ImGui::SetDragDropPayload("ASSET_PREFAB", pathStr.c_str(), pathStr.size() + 1);

                    ImGui::Text("Asset: %s", entry.filename.c_str());
                    ImGui::EndDragDropSource();
                }

                // Context Menu on Card
                if (ImGui::BeginPopupContextItem())
                {
                    renderContextMenu(state, &entry);
                    ImGui::EndPopup();
                }

                // Draw vector icon centered inside card thumbnail
                ImVec2 bMin = ImGui::GetItemRectMin();
                float iconDim = std::min(cardWidth * 0.44f, 36.0f);
                float icx = bMin.x + (cardWidth - iconDim) * 0.5f;
                float icy = bMin.y + (cardWidth - iconDim) * 0.5f;

                EditorIcons::DrawIcon(ImGui::GetWindowDrawList(), ImVec2(icx, icy), ImVec2(icx + iconDim, icy + iconDim), iconKind, iconCol);

                ImGui::PopStyleColor(3);

                // Label below card
                std::string shortName = entry.filename;
                int maxChars = std::max(6, static_cast<int>(cardWidth / 7.5f));
                if (static_cast<int>(shortName.size()) > maxChars)
                {
                    shortName = shortName.substr(0, maxChars - 2) + "..";
                }
                ImVec2 labelSz = ImGui::CalcTextSize(shortName.c_str());
                float textIndent = std::max(0.0f, (cardWidth - labelSz.x) * 0.5f);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textIndent);
                ImGui::TextUnformatted(shortName.c_str());

                ImGui::EndGroup();

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s\nType: %s\nSize: %zu bytes",
                                      entry.filename.c_str(),
                                      getAssetTypeName(entry.type),
                                      entry.fileSize);
                }

                // Handle Double Click
                if (isDoubleClicked)
                {
                    if (entry.isDirectory)
                    {
                        pendingDirChange = entry.fullPath;
                    }
                    else if (entry.type == AssetType::World)
                    {
                        state.loadWorldFromFile(entry.fullPath.string());
                    }
                    else if (entry.type == AssetType::Sprite || entry.type == AssetType::Texture)
                    {
                        if (onOpenSpriteEditor)
                        {
                            onOpenSpriteEditor(entry.fullPath);
                        }
                    }
                }

                // Flow to next card or wrap
                float nextX = ImGui::GetItemRectMax().x + spacing;
                if (nextX + cardWidth < availWidth + ImGui::GetWindowPos().x)
                {
                    ImGui::SameLine();
                }

                ImGui::PopID();
            }
        }
        else
        {
            // Clean Table View (List)
            if (ImGui::BeginTable("AssetListTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < m_entries.size(); ++i)
                {
                    const auto &entry = m_entries[i];

                    if (m_searchFilter[0] != '\0')
                    {
                        std::string fnameLower = entry.filename;
                        std::string filterLower = m_searchFilter;
                        std::transform(fnameLower.begin(), fnameLower.end(), fnameLower.begin(), ::tolower);
                        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
                        if (fnameLower.find(filterLower) == std::string::npos)
                            continue;
                    }

                    ImGui::PushID(static_cast<int>(i));
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    IconType iconKind = getAssetIconType(entry.type, entry.isDirectory);
                    ImU32 iconCol = getAssetIconColor(entry.type, entry.isDirectory);

                    bool isSelected = isPathSelected(entry.fullPath);
                    std::string label = "    " + entry.filename;
                    if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        ImGuiIO &io = ImGui::GetIO();
                        bool ctrl = io.KeyCtrl;
                        selectPath(entry.fullPath, ctrl);
                        if (!entry.isDirectory)
                        {
                            state.selectAsset(entry.fullPath);
                        }

                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            if (entry.isDirectory)
                            {
                                pendingDirChange = entry.fullPath;
                            }
                            else if (entry.type == AssetType::World)
                            {
                                state.loadWorldFromFile(entry.fullPath.string());
                            }
                            else if ((entry.type == AssetType::Sprite || entry.type == AssetType::Texture) && onOpenSpriteEditor)
                            {
                                onOpenSpriteEditor(entry.fullPath);
                            }
                        }
                    }

                    ImVec2 itemMin = ImGui::GetItemRectMin();
                    ImVec2 itemMax = ImGui::GetItemRectMax();
                    float iconSize = 14.0f;
                    float iconX = itemMin.x + 4.0f;
                    float iconY = itemMin.y + (itemMax.y - itemMin.y - iconSize) * 0.5f;
                    EditorIcons::DrawIcon(ImGui::GetWindowDrawList(), ImVec2(iconX, iconY), ImVec2(iconX + iconSize, iconY + iconSize), iconKind, iconCol);

                    // Drag & Drop Target for Folders
                    if (entry.isDirectory || entry.type == AssetType::Folder)
                    {
                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_GAMEOBJECT_REPARENT"))
                            {
                                int objId = *(const int *)payload->Data;
                                EditorObject *obj = state.findObject(objId);
                                if (obj)
                                {
                                    createPrefabFromObject(state, *obj, entry.fullPath);
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }

                    // Drag & Drop Source
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        std::string pathStr = entry.fullPath.string();
                        ImGui::SetDragDropPayload("RONIN_ASSET_PATH", pathStr.c_str(), pathStr.size() + 1);
                        if (entry.type == AssetType::Sprite)
                            ImGui::SetDragDropPayload("ASSET_SPRITE", pathStr.c_str(), pathStr.size() + 1);
                        else if (entry.type == AssetType::Texture)
                            ImGui::SetDragDropPayload("ASSET_TEXTURE", pathStr.c_str(), pathStr.size() + 1);
                        else if (entry.type == AssetType::Audio)
                            ImGui::SetDragDropPayload("ASSET_AUDIO", pathStr.c_str(), pathStr.size() + 1);
                        else if (entry.type == AssetType::World)
                            ImGui::SetDragDropPayload("ASSET_WORLD", pathStr.c_str(), pathStr.size() + 1);
                        else if (entry.type == AssetType::Prefab)
                            ImGui::SetDragDropPayload("ASSET_PREFAB", pathStr.c_str(), pathStr.size() + 1);
                        ImGui::Text("Asset: %s", entry.filename.c_str());
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginPopupContextItem())
                    {
                        renderContextMenu(state, &entry);
                        ImGui::EndPopup();
                    }

                    ImGui::TableSetColumnIndex(1);
                    EditorIcons::IconLabel(iconKind, getAssetTypeName(entry.type), iconCol, 13.0f);

                    ImGui::TableSetColumnIndex(2);
                    if (entry.isDirectory)
                        ImGui::TextDisabled("--");
                    else
                        ImGui::Text("%zu B", entry.fileSize);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextDisabled("%s", entry.fullPath.filename().string().c_str());

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }

        if (!pendingDirChange.empty())
        {
            SetCurrentDirectory(pendingDirChange);
            clearSelection();
        }
    }

    void AssetBrowser::renderContextMenu(EditorState &state, const AssetEntry *targetItem)
    {
        if (targetItem)
        {
            if (targetItem->type == AssetType::World && ImGui::MenuItem("Open World"))
            {
                state.loadWorldFromFile(targetItem->fullPath.string());
            }
            if ((targetItem->type == AssetType::Sprite || targetItem->type == AssetType::Texture) && ImGui::MenuItem("Open in Sprite Editor"))
            {
                if (onOpenSpriteEditor)
                {
                    onOpenSpriteEditor(targetItem->fullPath);
                }
            }
            if (targetItem->type == AssetType::Texture && ImGui::MenuItem("Create Sprite (.sprite) from Texture"))
            {
                std::filesystem::path spritePath = targetItem->fullPath.parent_path() / (targetItem->fullPath.stem().string() + ".sprite");
                int count = 1;
                while (std::filesystem::exists(spritePath))
                {
                    spritePath = targetItem->fullPath.parent_path() / (targetItem->fullPath.stem().string() + "_" + std::to_string(count++) + ".sprite");
                }
                std::string texGuid = AssetDatabase::GetInstance().GetGuidByPath(targetItem->fullPath);
                nlohmann::json j = nlohmann::json::object();
                j["formatVersion"] = 1;
                j["name"] = spritePath.stem().string();
                std::error_code ec;
                j["texturePath"] = std::filesystem::relative(targetItem->fullPath, m_assetsRoot, ec).string();
                if (!texGuid.empty()) j["textureGuid"] = texGuid;
                nlohmann::json jr = nlohmann::json::object();
                jr["x"] = 0; jr["y"] = 0; jr["w"] = 64; jr["h"] = 64;
                j["rect"] = jr;
                nlohmann::json jp = nlohmann::json::object();
                jp["x"] = 0.5; jp["y"] = 0.5;
                j["pivot"] = jp;
                j["pixelsPerUnit"] = 100.0;
                std::ofstream ofs(spritePath);
                ofs << j.dump(2) << "\n";
                ofs.close();

                MetaFile mf;
                MetaManager::EnsureMetaFile(spritePath, mf);
                AssetDatabase::GetInstance().Refresh();
                refreshCurrentDirectory();
                if (onOpenSpriteEditor) onOpenSpriteEditor(spritePath);
            }
            if (targetItem->type == AssetType::Prefab && ImGui::MenuItem("Instantiate in World"))
            {
                state.instantiatePrefab(targetItem->fullPath.string(), {0.0f, 0.0f});
            }
            if (targetItem->type == AssetType::Terrain && ImGui::MenuItem("Create Terrain Object in World"))
            {
                auto &newObj = state.createObject(targetItem->fullPath.stem().string());
                state.loadTerrainAsset(newObj.terrain, targetItem->fullPath.string());
                newObj.terrain.enabled = true;
                state.selectObject(newObj.id);
            }

            if (ImGui::MenuItem("Show in System File Manager##Item"))
            {
#if defined(__linux__)
                std::string cmd = "xdg-open \"" + targetItem->fullPath.parent_path().string() + "\" &";
                std::system(cmd.c_str());
#endif
            }

            if (ImGui::MenuItem("Delete", "Del"))
            {
                AssetDatabase::GetInstance().DeleteAsset(targetItem->fullPath);
                refreshCurrentDirectory();
                return;
            }
        }
        else
        {
            if (ImGui::MenuItem("Import New Asset..."))
            {
                OpenNativeImportDialog(state);
            }

            if (ImGui::BeginMenu("Create"))
            {
                if (ImGui::MenuItem("Folder"))
                {
                    std::filesystem::path newDir = m_currentDir / "New Folder";
                    int count = 1;
                    while (std::filesystem::exists(newDir))
                    {
                        newDir = m_currentDir / ("New Folder " + std::to_string(count++));
                    }
                    std::error_code ec;
                    std::filesystem::create_directory(newDir, ec);
                    refreshCurrentDirectory();
                }

                if (ImGui::MenuItem("World"))
                {
                    std::filesystem::path newWorld = m_currentDir / "NewWorld.world";
                    int count = 1;
                    while (std::filesystem::exists(newWorld))
                    {
                        newWorld = m_currentDir / ("NewWorld" + std::to_string(count++) + ".world");
                    }
                    std::ofstream ofs(newWorld);
                    ofs << "{\n  \"formatVersion\": 1,\n  \"worldName\": \"NewWorld\",\n  \"objects\": []\n}\n";
                    ofs.close();

                    MetaFile mf;
                    MetaManager::EnsureMetaFile(newWorld, mf);
                    AssetDatabase::GetInstance().Refresh();
                    refreshCurrentDirectory();
                }

                if (ImGui::MenuItem("Sprite"))
                {
                    std::filesystem::path newSprite = m_currentDir / "NewSprite.sprite";
                    int count = 1;
                    while (std::filesystem::exists(newSprite))
                    {
                        newSprite = m_currentDir / ("NewSprite" + std::to_string(count++) + ".sprite");
                    }
                    std::ofstream ofs(newSprite);
                    ofs << "{\n  \"formatVersion\": 1,\n  \"name\": \"NewSprite\",\n  \"texturePath\": \"\",\n  \"rect\": {\"x\": 0, \"y\": 0, \"w\": 64, \"h\": 64},\n  \"pivot\": {\"x\": 0.5, \"y\": 0.5},\n  \"pixelsPerUnit\": 100.0\n}\n";
                    ofs.close();

                    MetaFile mf;
                    MetaManager::EnsureMetaFile(newSprite, mf);
                    AssetDatabase::GetInstance().Refresh();
                    refreshCurrentDirectory();

                    if (onOpenSpriteEditor)
                    {
                        onOpenSpriteEditor(newSprite);
                    }
                }

                if (ImGui::MenuItem("Prefab"))
                {
                    std::filesystem::path newPrefab = m_currentDir / "NewPrefab.prefab";
                    int count = 1;
                    while (std::filesystem::exists(newPrefab))
                    {
                        newPrefab = m_currentDir / ("NewPrefab" + std::to_string(count++) + ".prefab");
                    }
                    std::ofstream ofs(newPrefab);
                    ofs << "{\n  \"format\": \"RoninEnginePrefab\",\n  \"prefabVersion\": 1,\n  \"name\": \"NewPrefab\",\n  \"rootObject\": {\n    \"name\": \"NewPrefab\",\n    \"tag\": \"Untagged\",\n    \"layer\": 0,\n    \"active\": true,\n    \"components\": []\n  },\n  \"children\": []\n}\n";
                    ofs.close();

                    MetaFile mf;
                    MetaManager::EnsureMetaFile(newPrefab, mf);
                    AssetDatabase::GetInstance().Refresh();
                    refreshCurrentDirectory();
                }

                if (ImGui::MenuItem("Terrain"))
                {
                    std::filesystem::path newTerrain = m_currentDir / "NewTerrain.terrain";
                    int count = 1;
                    while (std::filesystem::exists(newTerrain))
                    {
                        newTerrain = m_currentDir / ("NewTerrain" + std::to_string(count++) + ".terrain");
                    }
                    Terrain2DData defaultTer;
                    defaultTer.enabled = true;
                    defaultTer.width = 16;
                    defaultTer.length = 8;
                    defaultTer.cellScale = 1.0f;
                    defaultTer.ensureGridInitialized(0);
                    state.saveTerrainAsset(defaultTer, newTerrain.string());
                    refreshCurrentDirectory();
                }

                if (ImGui::MenuItem("Text Document (.txt)"))
                {
                    std::filesystem::path newTxt = m_currentDir / "NewText.txt";
                    int count = 1;
                    while (std::filesystem::exists(newTxt))
                    {
                        newTxt = m_currentDir / ("NewText" + std::to_string(count++) + ".txt");
                    }
                    std::ofstream ofs(newTxt);
                    ofs << "Text data stream\n";
                    ofs.close();

                    MetaFile mf;
                    MetaManager::EnsureMetaFile(newTxt, mf);
                    AssetDatabase::GetInstance().Refresh();
                    refreshCurrentDirectory();
                }

                if (ImGui::MenuItem("JSON Data (.json)"))
                {
                    std::filesystem::path newJson = m_currentDir / "NewData.json";
                    int count = 1;
                    while (std::filesystem::exists(newJson))
                    {
                        newJson = m_currentDir / ("NewData" + std::to_string(count++) + ".json");
                    }
                    std::ofstream ofs(newJson);
                    ofs << "{\n  \"version\": 1\n}\n";
                    ofs.close();

                    MetaFile mf;
                    MetaManager::EnsureMetaFile(newJson, mf);
                    AssetDatabase::GetInstance().Refresh();
                    refreshCurrentDirectory();
                }

                if (ImGui::MenuItem("C++ Script"))
                {
                    std::filesystem::path newH = m_currentDir / "CustomComponent.h";
                    std::filesystem::path newCpp = m_currentDir / "CustomComponent.cpp";
                    std::ofstream ofsH(newH);
                    ofsH << "#pragma once\n#include <ronin/ronin.h>\n\nclass CustomComponent : public RoninEngine::Runtime::Component {\npublic:\n    void OnStart() override {}\n    void OnUpdate() override {}\n};\n";
                    ofsH.close();
                    std::ofstream ofsCpp(newCpp);
                    ofsCpp << "#include \"CustomComponent.h\"\n";
                    ofsCpp.close();

                    MetaFile mfH, mfCpp;
                    MetaManager::EnsureMetaFile(newH, mfH);
                    MetaManager::EnsureMetaFile(newCpp, mfCpp);
                    AssetDatabase::GetInstance().Refresh();
                    refreshCurrentDirectory();
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Show in System File Manager##Folder"))
            {
#if defined(__linux__)
                std::string cmd = "xdg-open \"" + m_currentDir.string() + "\" &";
                std::system(cmd.c_str());
#endif
            }

            if (ImGui::MenuItem("Refresh"))
            {
                refreshCurrentDirectory();
            }
        }
    }

    void AssetBrowser::renderImportModal(EditorState &state)
    {
        if (!m_showImportDialog)
            return;

        ImGui::OpenPopup("Import New Asset");

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(520.0f, 260.0f));

        if (ImGui::BeginPopupModal("Import New Asset", &m_showImportDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Destination Folder:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%s", m_currentDir.string().c_str());
            ImGui::Separator();

            ImGui::Text("Source File Path (on system):");
            ImGui::SetNextItemWidth(480.0f);
            ImGui::InputTextWithHint("##ImportSrc", "/path/to/texture.png or sound.wav...", m_importSourcePath, sizeof(m_importSourcePath));

            ImGui::Spacing();
            ImGui::TextDisabled("Quick Demo Imports:");
            if (ImGui::SmallButton("Engine Logo"))
            {
                std::filesystem::path logoPath = std::filesystem::current_path() / "media" / "RoninEngine-logo.png";
                std::snprintf(m_importSourcePath, sizeof(m_importSourcePath), "%s", logoPath.string().c_str());
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("SDL2 Logo"))
            {
                std::filesystem::path sdlPath = std::filesystem::current_path() / "media" / "libsdl-logo.png";
                std::snprintf(m_importSourcePath, sizeof(m_importSourcePath), "%s", sdlPath.string().c_str());
            }

            if (!m_importErrorMessage.empty())
            {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: %s", m_importErrorMessage.c_str());
            }

            ImGui::Spacing();
            ImGui::Separator();

            if (ImGui::Button("Import", ImVec2(120.0f, 0.0f)))
            {
                std::string src = m_importSourcePath;
                if (src.empty())
                {
                    m_importErrorMessage = "Source path cannot be empty.";
                }
                else if (!std::filesystem::exists(src))
                {
                    m_importErrorMessage = "Source file does not exist.";
                }
                else
                {
                    std::error_code ec;
                    std::filesystem::path srcPath(src);
                    std::filesystem::path destPath = m_currentDir / srcPath.filename();

                    std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing, ec);
                    if (ec)
                    {
                        m_importErrorMessage = "Copy failed: " + ec.message();
                    }
                    else
                    {
                        state.logInfo("Imported asset: " + destPath.filename().string() + " into " + m_currentDir.string());
                        refreshCurrentDirectory();
                        m_showImportDialog = false;
                        m_importSourcePath[0] = '\0';
                        m_importErrorMessage.clear();
                        ImGui::CloseCurrentPopup();
                    }
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f)))
            {
                m_showImportDialog = false;
                m_importErrorMessage.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void AssetBrowser::Render(EditorState &state)
    {
        if (ImGui::Begin("Project"))
        {
            const auto &proj = ProjectManager::GetInstance().GetCurrentProject();
            if (proj.rootPath.empty())
            {
                ImGui::TextDisabled("No project open.");
                ImGui::End();
                return;
            }

            std::filesystem::path expectedAssets = proj.rootPath / "Assets";
            if (m_assetsRoot != expectedAssets)
            {
                m_assetsRoot = expectedAssets;
                m_currentDir = m_assetsRoot;
                refreshCurrentDirectory();
            }

            // Top Breadcrumbs & Control Bar
            renderBreadcrumbs(state);
            ImGui::Separator();

            // Two-pane layout with clean resizable splitter
            float availWidth = ImGui::GetContentRegionAvail().x;
            float availHeight = ImGui::GetContentRegionAvail().y;

            if (availWidth > 260.0f)
            {
                m_treeWidth = std::clamp(m_treeWidth, 120.0f, availWidth - 140.0f);
            }
            else
            {
                m_treeWidth = std::max(60.0f, availWidth * 0.35f);
            }

            // Left Pane: Folder Tree
            ImGui::BeginChild("AssetDirectoryTree", ImVec2(m_treeWidth, availHeight), true);
            if (std::filesystem::exists(m_assetsRoot))
            {
                renderDirectoryTree(m_assetsRoot);
            }
            ImGui::EndChild();

            ImGui::SameLine();

            // Splitter
            ImGui::Button("##Splitter", ImVec2(4.0f, availHeight));
            if (ImGui::IsItemActive())
            {
                m_treeWidth += ImGui::GetIO().MouseDelta.x;
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            ImGui::SameLine();

            // Right Pane: Files Display
            ImGui::BeginChild("AssetFilesArea", ImVec2(0, availHeight), true);

            // Empty context menu
            if (ImGui::BeginPopupContextWindow("AssetFilesEmptyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                renderContextMenu(state, nullptr);
                ImGui::EndPopup();
            }

            renderFileGrid(state);

            // Drag-and-drop GameObject from Hierarchy into empty Asset area -> create Prefab!
            if (ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->ContentRegionRect, ImGui::GetID("AssetFilesAreaDropTarget")))
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_GAMEOBJECT_REPARENT"))
                {
                    int objId = *(const int *)payload->Data;
                    EditorObject *obj = state.findObject(objId);
                    if (obj)
                    {
                        createPrefabFromObject(state, *obj, m_currentDir);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndChild();

            renderImportModal(state);
        }
        ImGui::End();
    }
} // namespace RoninEditor
