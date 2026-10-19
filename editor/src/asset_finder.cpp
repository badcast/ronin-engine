#include "asset_finder.h"
#include "project_manager.h"
#include "meta_manager.h"
#include "editor_icons.h"
#include <imgui.h>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace RoninEditor
{
    AssetFinder::AssetFinder() = default;
    AssetFinder::~AssetFinder() = default;

    void AssetFinder::Open()
    {
        m_isOpen = true;
        m_shouldOpenPopupTrigger = true;
        m_shouldFocusInput = true;
        m_selectedIndex = -1;
        const auto &proj = ProjectManager::GetInstance().GetCurrentProject();
        rescan(proj.rootPath);
    }

    void AssetFinder::Close()
    {
        m_isOpen = false;
    }

    AssetType AssetFinder::determineType(const std::filesystem::path &path) const
    {
        return MetaManager::DetermineAssetType(path);
    }

    void AssetFinder::rescan(const std::filesystem::path &projectRoot)
    {
        m_items.clear();
        m_scannedRoot = projectRoot;
        if (projectRoot.empty() || !std::filesystem::exists(projectRoot))
            return;

        std::vector<std::filesystem::path> scanDirs;
        std::filesystem::path assetsDir = projectRoot / "Assets";
        std::filesystem::path scriptsDir = projectRoot / "Scripts";

        if (std::filesystem::exists(assetsDir)) scanDirs.push_back(assetsDir);
        if (std::filesystem::exists(scriptsDir)) scanDirs.push_back(scriptsDir);
        if (scanDirs.empty()) scanDirs.push_back(projectRoot);

        for (const auto &dir : scanDirs)
        {
            std::error_code ec;
            for (const auto &it : std::filesystem::recursive_directory_iterator(dir, ec))
            {
                if (ec) break;
                if (!it.is_directory())
                {
                    AssetFinderItem item;
                    item.fullPath = it.path();
                    item.filename = it.path().filename().string();
                    std::error_code relEc;
                    item.relativePath = std::filesystem::relative(it.path(), projectRoot, relEc);
                    if (relEc || item.relativePath.empty())
                        item.relativePath = item.filename;

                    item.type = determineType(it.path());
                    item.fileSize = it.file_size(ec);
                    item.assetId = AssetBrowser::CalculateAssetId(item.relativePath.string());

                    m_items.push_back(item);
                }
            }
        }

        // Sort alphabetically by filename
        std::sort(m_items.begin(), m_items.end(), [](const AssetFinderItem &a, const AssetFinderItem &b) {
            return a.filename < b.filename;
        });
    }

    void AssetFinder::Render(EditorState &state, AssetBrowser &assetBrowser, std::function<void(const std::filesystem::path &)> onOpenSpriteEditor)
    {
        if (!m_isOpen)
            return;

        if (m_shouldOpenPopupTrigger)
        {
            ImGui::OpenPopup("Asset Finder (Quick Search)###AssetFinderPopup");
            m_shouldOpenPopupTrigger = false;
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(760.0f, 480.0f), ImGuiCond_Appearing);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
        if (ImGui::BeginPopupModal("Asset Finder (Quick Search)###AssetFinderPopup", &m_isOpen, flags))
        {
            // Category Buttons
            const char *categories[] = {"All", "Worlds", "Sprites", "Textures", "Audio", "Scripts", "Prefabs"};
            for (int i = 0; i < 7; ++i)
            {
                if (i > 0) ImGui::SameLine();
                bool selected = (m_selectedCategory == i);
                if (selected)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.45f, 0.85f, 1.0f));
                }
                if (ImGui::SmallButton(categories[i]))
                {
                    m_selectedCategory = i;
                }
                if (selected)
                {
                    ImGui::PopStyleColor();
                }
            }

            ImGui::Spacing();

            // Search Bar
            if (m_shouldFocusInput)
            {
                ImGui::SetKeyboardFocusHere();
                m_shouldFocusInput = false;
            }
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 90.0f);
            ImGui::InputTextWithHint("##AssetFinderInput", "Search by name, relative path, or Asset ID (e.g. 0x... or 12345)...", m_filter, sizeof(m_filter));
            ImGui::SameLine();
            if (ImGui::Button("Refresh", ImVec2(80.0f, 0.0f)))
            {
                const auto &proj = ProjectManager::GetInstance().GetCurrentProject();
                rescan(proj.rootPath);
            }

            ImGui::Separator();

            // Filter entries
            std::string query = m_filter;
            std::transform(query.begin(), query.end(), query.begin(), ::tolower);

            std::vector<size_t> matchedIndices;
            for (size_t i = 0; i < m_items.size(); ++i)
            {
                const auto &item = m_items[i];

                // Category filter
                if (m_selectedCategory == 1 && item.type != AssetType::World) continue;
                if (m_selectedCategory == 2 && item.type != AssetType::Sprite) continue;
                if (m_selectedCategory == 3 && item.type != AssetType::Texture) continue;
                if (m_selectedCategory == 4 && item.type != AssetType::Audio) continue;
                if (m_selectedCategory == 5 && item.type != AssetType::Code && item.type != AssetType::TextStream) continue;
                if (m_selectedCategory == 6 && item.type != AssetType::Prefab) continue;

                if (!query.empty())
                {
                    std::string fLower = item.filename;
                    std::transform(fLower.begin(), fLower.end(), fLower.begin(), ::tolower);
                    std::string rLower = item.relativePath.string();
                    std::transform(rLower.begin(), rLower.end(), rLower.begin(), ::tolower);

                    char hexBuf[32];
                    std::snprintf(hexBuf, sizeof(hexBuf), "%x", item.assetId);
                    std::string hexStr = hexBuf;
                    std::string decStr = std::to_string(item.assetId);

                    bool matchName = (fLower.find(query) != std::string::npos);
                    bool matchPath = (rLower.find(query) != std::string::npos);
                    bool matchHex = (hexStr.find(query) != std::string::npos);
                    bool matchDec = (decStr.find(query) != std::string::npos);

                    if (!matchName && !matchPath && !matchHex && !matchDec)
                        continue;
                }

                matchedIndices.push_back(i);
            }

            // Results List
            ImGui::BeginChild("AssetFinderResultsScroll", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - 42.0f), true);

            int activateIndex = -1;

            if (ImGui::BeginTable("AssetFinderTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.40f);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Asset ID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Relative Path", ImGuiTableColumnFlags_WidthStretch, 0.40f);
                ImGui::TableHeadersRow();

                for (size_t mi = 0; mi < matchedIndices.size(); ++mi)
                {
                    size_t itemIdx = matchedIndices[mi];
                    const auto &item = m_items[itemIdx];

                    ImGui::PushID(static_cast<int>(itemIdx));
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    bool isSelected = (m_selectedIndex == static_cast<int>(itemIdx));

                    IconType iconKind = IconType::File;
                    ImU32 iconCol = EditorPalette::TextMuted;
                    const char *typeStr = "File";
                    switch (item.type)
                    {
                    case AssetType::World:
                        iconKind = IconType::World;
                        iconCol = EditorPalette::IconWorld;
                        typeStr = "World";
                        break;
                    case AssetType::Sprite:
                        iconKind = IconType::Sprite;
                        iconCol = EditorPalette::IconSprite;
                        typeStr = "Sprite";
                        break;
                    case AssetType::Texture:
                        iconKind = IconType::Sprite;
                        iconCol = EditorPalette::IconSprite;
                        typeStr = "Texture";
                        break;
                    case AssetType::Audio:
                        iconKind = IconType::Audio;
                        iconCol = EditorPalette::IconAudio;
                        typeStr = "Audio";
                        break;
                    case AssetType::Code:
                        iconKind = IconType::File;
                        iconCol = EditorPalette::IconScript;
                        typeStr = "Code";
                        break;
                    case AssetType::TextStream:
                        iconKind = IconType::File;
                        iconCol = EditorPalette::TextMuted;
                        typeStr = "Text";
                        break;
                    case AssetType::Prefab:
                        iconKind = IconType::World;
                        iconCol = EditorPalette::Warning;
                        typeStr = "Prefab";
                        break;
                    default:
                        break;
                    }

                    if (ImGui::Selectable(item.filename.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        m_selectedIndex = static_cast<int>(itemIdx);
                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            activateIndex = static_cast<int>(itemIdx);
                        }
                    }

                    // Drag & Drop
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        std::string pathStr = item.fullPath.string();
                        ImGui::SetDragDropPayload("RONIN_ASSET_PATH", pathStr.c_str(), pathStr.size() + 1);
                        if (item.type == AssetType::Sprite)
                            ImGui::SetDragDropPayload("ASSET_SPRITE", pathStr.c_str(), pathStr.size() + 1);
                        else if (item.type == AssetType::Texture)
                            ImGui::SetDragDropPayload("ASSET_TEXTURE", pathStr.c_str(), pathStr.size() + 1);
                        else if (item.type == AssetType::Audio)
                            ImGui::SetDragDropPayload("ASSET_AUDIO", pathStr.c_str(), pathStr.size() + 1);
                        else if (item.type == AssetType::World)
                            ImGui::SetDragDropPayload("ASSET_WORLD", pathStr.c_str(), pathStr.size() + 1);
                        else if (item.type == AssetType::Prefab)
                            ImGui::SetDragDropPayload("ASSET_PREFAB", pathStr.c_str(), pathStr.size() + 1);
                        ImGui::Text("Asset: %s", item.filename.c_str());
                        ImGui::EndDragDropSource();
                    }

                    ImGui::TableSetColumnIndex(1);
                    EditorIcons::IconLabel(iconKind, typeStr, iconCol, 13.0f);

                    ImGui::TableSetColumnIndex(2);
                    char hexId[32];
                    std::snprintf(hexId, sizeof(hexId), "0x%08X", item.assetId);
                    ImGui::TextColored(ImVec4(0.35f, 0.75f, 0.95f, 1.0f), "%s", hexId);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextDisabled("%s", item.relativePath.string().c_str());

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::EndChild();

            // Bottom action row
            ImGui::TextDisabled("Found %zu asset%s", matchedIndices.size(), matchedIndices.size() == 1 ? "" : "s");

            const AssetFinderItem *selectedItem = nullptr;
            if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size()))
            {
                selectedItem = &m_items[m_selectedIndex];
            }

            EditorObject *activeObj = state.findObject(state.selectedObjectId);

            auto applyOrActivateAsset = [&](const AssetFinderItem &item) {
                if (activeObj != nullptr && (item.type == AssetType::Sprite || item.type == AssetType::Texture))
                {
                    activeObj->sprite.spritePath = item.relativePath.string();
                    activeObj->sprite.assetId = item.assetId;
                    activeObj->sprite.enabled = true;
                    activeObj->sprite.primitiveShape = 3; // Custom Image
                    state.isDirty = true;
                    state.logInfo("Assigned Sprite to '" + activeObj->name + "': " + item.relativePath.string());
                    Close();
                    ImGui::CloseCurrentPopup();
                }
                else if (activeObj != nullptr && item.type == AssetType::Audio)
                {
                    activeObj->audio.clipPath = item.relativePath.string();
                    activeObj->audio.clipAssetId = item.assetId;
                    activeObj->audio.enabled = true;
                    state.isDirty = true;
                    state.logInfo("Assigned Audio Clip to '" + activeObj->name + "': " + item.relativePath.string());
                    Close();
                    ImGui::CloseCurrentPopup();
                }
                else if (item.type == AssetType::World)
                {
                    state.loadWorldFromFile(item.fullPath.string());
                    state.logInfo("Loaded World from Asset Finder: " + item.filename);
                    Close();
                    ImGui::CloseCurrentPopup();
                }
                else if ((item.type == AssetType::Sprite || item.type == AssetType::Texture) && onOpenSpriteEditor)
                {
                    onOpenSpriteEditor(item.fullPath);
                    Close();
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    assetBrowser.PingAsset(item.fullPath);
                    Close();
                    ImGui::CloseCurrentPopup();
                }
            };

            ImGui::Spacing();
            ImGui::Separator();

            if (selectedItem != nullptr)
            {
                char hexId[32];
                std::snprintf(hexId, sizeof(hexId), "0x%08X", selectedItem->assetId);
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Selected: %s [ID: %s]", selectedItem->relativePath.string().c_str(), hexId);
            }
            else
            {
                ImGui::TextDisabled("No asset selected in list.");
            }

            ImGui::Spacing();

            if (activeObj != nullptr && selectedItem != nullptr &&
                (selectedItem->type == AssetType::Sprite || selectedItem->type == AssetType::Texture || selectedItem->type == AssetType::Audio))
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.35f, 1.0f));
                std::string btnLabel = "Apply to " + activeObj->name;
                if (ImGui::Button(btnLabel.c_str(), ImVec2(170.0f, 28.0f)))
                {
                    applyOrActivateAsset(*selectedItem);
                }
                ImGui::PopStyleColor();
                ImGui::SameLine();
            }

            ImGui::BeginDisabled(selectedItem == nullptr);
            if (ImGui::Button("Open Asset", ImVec2(100.0f, 28.0f)))
            {
                if (selectedItem)
                {
                    applyOrActivateAsset(*selectedItem);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Ping in Project", ImVec2(120.0f, 28.0f)))
            {
                if (selectedItem)
                {
                    assetBrowser.PingAsset(selectedItem->fullPath);
                    Close();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(80.0f, 28.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                Close();
                ImGui::CloseCurrentPopup();
            }

            // Keyboard Enter to activate
            if (selectedItem != nullptr && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)))
            {
                applyOrActivateAsset(*selectedItem);
            }

            // Double click activation
            if (activateIndex >= 0 && activateIndex < static_cast<int>(m_items.size()))
            {
                applyOrActivateAsset(m_items[activateIndex]);
            }

            ImGui::EndPopup();
        }
    }
} // namespace RoninEditor
