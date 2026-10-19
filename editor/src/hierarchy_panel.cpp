#include "hierarchy_panel.h"
#include "editor_icons.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>
#include <algorithm>

namespace RoninEditor
{
    HierarchyPanel::HierarchyPanel() = default;
    HierarchyPanel::~HierarchyPanel() = default;

    void HierarchyPanel::renderObjectNode(EditorState &state, EditorObject &obj)
    {
        // Check filter
        if (m_searchFilter[0] != '\0')
        {
            std::string nameLower = obj.name;
            std::string filterLower = m_searchFilter;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
            if (nameLower.find(filterLower) == std::string::npos)
            {
                return;
            }
        }

        // Count direct children
        int childCount = 0;
        for (const auto &o : state.activeWorld().objects)
        {
            if (o.parentId == obj.id)
                childCount++;
        }

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (childCount == 0)
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        if (state.isObjectSelected(obj.id))
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // Active indicator / toggle
        ImGui::PushID(obj.id);
        if (ImGui::Checkbox("##Active", &obj.active))
        {
            state.isDirty = true;
        }
        ImGui::SameLine();

        // Node
        bool isRenaming = (m_renamingObjId == obj.id);
        bool nodeOpen = false;

        if (isRenaming)
        {
            ImGui::SetNextItemWidth(140.0f);
            if (ImGui::InputText("##Rename", m_renameBuf, sizeof(m_renameBuf), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (m_renameBuf[0] != '\0')
                {
                    obj.name = m_renameBuf;
                    state.isDirty = true;
                }
                m_renamingObjId = -1;
            }
            if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
            {
                m_renamingObjId = -1;
            }
        }
        else
        {
            std::string label = "    " + obj.name;
            nodeOpen = ImGui::TreeNodeEx((void *)(intptr_t)obj.id, flags, "%s", label.c_str());

            ImVec2 itemMin = ImGui::GetItemRectMin();
            ImVec2 itemMax = ImGui::GetItemRectMax();
            float iconSize = 13.0f;
            float iconX = itemMin.x + (childCount > 0 ? 20.0f : 8.0f);
            float iconY = itemMin.y + (itemMax.y - itemMin.y - iconSize) * 0.5f;

            IconType it = IconType::File;
            ImU32 col = EditorPalette::TextMuted;
            if (obj.camera.enabled) { it = IconType::Camera; col = EditorPalette::IconCamera; }
            else if (obj.light.enabled) { it = IconType::Light; col = EditorPalette::IconLight; }
            else if (obj.terrain.enabled) { it = IconType::Terrain; col = EditorPalette::Success; }
            else if (obj.sprite.enabled) { it = IconType::Sprite; col = EditorPalette::IconSprite; }
            else if (obj.audio.enabled) { it = IconType::Audio; col = EditorPalette::IconAudio; }

            EditorIcons::DrawIcon(ImGui::GetWindowDrawList(), ImVec2(iconX, iconY), ImVec2(iconX + iconSize, iconY + iconSize), it, col);

            if (ImGui::IsItemClicked(0))
            {
                ImGuiIO &io = ImGui::GetIO();
                bool ctrl = io.KeyCtrl;
                bool shift = io.KeyShift;

                if (shift && m_lastSelectedId != -1)
                {
                    auto &objs = state.activeWorld().objects;
                    int idx1 = -1, idx2 = -1;
                    for (int i = 0; i < static_cast<int>(objs.size()); ++i)
                    {
                        if (objs[i].id == m_lastSelectedId) idx1 = i;
                        if (objs[i].id == obj.id) idx2 = i;
                    }
                    if (idx1 != -1 && idx2 != -1)
                    {
                        int start = std::min(idx1, idx2);
                        int end = std::max(idx1, idx2);
                        if (!ctrl) state.clearSelection();
                        for (int i = start; i <= end; ++i)
                        {
                            state.selectObject(objs[i].id, true);
                        }
                    }
                }
                else
                {
                    state.selectObject(obj.id, ctrl);
                    m_lastSelectedId = obj.id;
                }
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                m_renamingObjId = obj.id;
                strncpy(m_renameBuf, obj.name.c_str(), sizeof(m_renameBuf) - 1);
            }

            // Drag and Drop Source (Drag this object)
            if (ImGui::BeginDragDropSource())
            {
                int draggedId = obj.id;
                ImGui::SetDragDropPayload("RONIN_GAMEOBJECT_REPARENT", &draggedId, sizeof(int));
                ImGui::Text("Move %s", obj.name.c_str());
                ImGui::EndDragDropSource();
            }

            // Drag and Drop Target (Drop another object onto this as parent, or drop Prefab to instantiate as child)
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_GAMEOBJECT_REPARENT"))
                {
                    int draggedId = *(const int *)payload->Data;
                    if (draggedId != obj.id)
                    {
                        EditorObject *draggedObj = state.findObject(draggedId);
                        if (draggedObj)
                        {
                            draggedObj->parentId = obj.id;
                            state.isDirty = true;
                        }
                    }
                }
                else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_PREFAB"))
                {
                    std::string prefabPath((const char *)payload->Data);
                    state.instantiatePrefab(prefabPath, obj.position, obj.id);
                }
                else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_ASSET_PATH"))
                {
                    std::string assetPath((const char *)payload->Data);
                    if (std::filesystem::path(assetPath).extension() == ".prefab")
                    {
                        state.instantiatePrefab(assetPath, obj.position, obj.id);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // Right-click context menu on object
            if (ImGui::BeginPopupContextItem())
            {
                state.selectedObjectId = obj.id;
                renderContextMenu(state, &obj);
                ImGui::EndPopup();
            }
        }

        // Children recursion
        if (nodeOpen && childCount > 0)
        {
            renderObjectTree(state, obj.id);
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    void HierarchyPanel::renderObjectTree(EditorState &state, int parentId)
    {
        auto &objects = state.activeWorld().objects;
        for (size_t i = 0; i < objects.size(); ++i)
        {
            if (objects[i].parentId == parentId)
            {
                renderObjectNode(state, objects[i]);
            }
        }
    }

    void HierarchyPanel::renderContextMenu(EditorState &state, EditorObject *targetObj)
    {
        if (targetObj)
        {
            if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
            {
                state.duplicateObject(targetObj->id);
            }
            if (ImGui::MenuItem("Rename", "F2"))
            {
                m_renamingObjId = targetObj->id;
                strncpy(m_renameBuf, targetObj->name.c_str(), sizeof(m_renameBuf) - 1);
            }
            if (ImGui::MenuItem("Delete", "Del"))
            {
                if (!state.isObjectSelected(targetObj->id))
                {
                    state.selectObject(targetObj->id, false);
                }
                for (int id : state.selectedObjectIds)
                {
                    m_pendingDeleteIds.push_back(id);
                }
                if (m_pendingDeleteIds.empty())
                {
                    m_pendingDeleteIds.push_back(targetObj->id);
                }
                return;
            }
            if (targetObj->parentId != -1)
            {
                if (ImGui::MenuItem("Unparent (Move to Root)"))
                {
                    targetObj->parentId = -1;
                    state.isDirty = true;
                }
            }
            ImGui::Separator();
        }

        int parentId = targetObj ? targetObj->id : -1;

        if (ImGui::MenuItem("Create Empty"))
        {
            state.createObject("Empty GameObject", parentId);
        }

        if (ImGui::BeginMenu("2D Object"))
        {
            if (ImGui::MenuItem("Sprite Rectangle"))
            {
                auto &obj = state.createObject("Sprite Rectangle", parentId);
                obj.sprite.enabled = true;
                obj.sprite.primitiveShape = 0;
                obj.sprite.size = {1.0f, 1.0f};
                obj.sprite.color = RoninEngine::Runtime::Color{220, 220, 220, 255};
            }
            if (ImGui::MenuItem("Sprite Circle"))
            {
                auto &obj = state.createObject("Sprite Circle", parentId);
                obj.sprite.enabled = true;
                obj.sprite.primitiveShape = 1;
                obj.sprite.size = {1.0f, 1.0f};
                obj.sprite.color = RoninEngine::Runtime::Color{80, 180, 240, 255};
            }
            if (ImGui::MenuItem("Sprite Triangle"))
            {
                auto &obj = state.createObject("Sprite Triangle", parentId);
                obj.sprite.enabled = true;
                obj.sprite.primitiveShape = 2;
                obj.sprite.size = {1.0f, 1.0f};
                obj.sprite.color = RoninEngine::Runtime::Color{240, 140, 60, 255};
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Terrain 2D"))
            {
                auto &obj = state.createObject("Terrain 2D", parentId);
                obj.terrain.enabled = true;
                obj.terrain.width = 16;
                obj.terrain.length = 8;
                obj.terrain.cellScale = 1.0f;
                obj.terrain.ensureGridInitialized(0);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Light"))
        {
            if (ImGui::MenuItem("Point Light (SimpleLight)"))
            {
                auto &obj = state.createObject("Point Light", parentId);
                obj.light.enabled = true;
                obj.light.lightType = 0;
                obj.light.range = 4.0f;
                obj.light.intensity = 1.0f;
                obj.light.color = RoninEngine::Runtime::Color{255, 240, 160, 220};
            }
            if (ImGui::MenuItem("Directional Light (DirectLight)"))
            {
                auto &obj = state.createObject("Directional Light", parentId);
                obj.light.enabled = true;
                obj.light.lightType = 1;
                obj.light.range = 10.0f;
                obj.light.intensity = 1.0f;
                obj.light.color = RoninEngine::Runtime::Color{255, 255, 255, 220};
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Audio"))
        {
            if (ImGui::MenuItem("Audio Source"))
            {
                auto &obj = state.createObject("Audio Source", parentId);
                obj.audio.enabled = true;
                obj.audio.volume = 0.85f;
                obj.audio.loop = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Camera"))
        {
            auto &obj = state.createObject("Camera", parentId);
            obj.camera.enabled = true;
            obj.camera.zoom = 1.0f;
            obj.camera.backcolor = RoninEngine::Runtime::Color{20, 24, 32, 255};
        }

        if (ImGui::BeginMenu("Physics"))
        {
            if (ImGui::MenuItem("Box Collider 2D"))
            {
                auto &obj = state.createObject("Box Collider", parentId);
                obj.physics.colliderEnabled = true;
                obj.physics.colliderType = 0;
                obj.physics.colliderSize = {1.0f, 1.0f};
            }
            if (ImGui::MenuItem("Circle Collider 2D"))
            {
                auto &obj = state.createObject("Circle Collider", parentId);
                obj.physics.colliderEnabled = true;
                obj.physics.colliderType = 1;
                obj.physics.colliderRadius = 0.5f;
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Particle System"))
        {
            auto &obj = state.createObject("Particle System", parentId);
            obj.particles.enabled = true;
            obj.particles.emit = true;
            obj.particles.startColor = RoninEngine::Runtime::Color{255, 180, 50, 240};
            obj.particles.endColor = RoninEngine::Runtime::Color{200, 40, 20, 0};
        }
    }

    void HierarchyPanel::Render(EditorState &state)
    {
        if (ImGui::Begin("Hierarchy"))
        {
            // Top Bar: Search and Create [+] button
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 32.0f);
            ImGui::InputTextWithHint("##SearchHierarchy", "Search...", m_searchFilter, sizeof(m_searchFilter));

            ImGui::SameLine();
            if (ImGui::Button("+", ImVec2(24.0f, 0.0f)))
            {
                ImGui::OpenPopup("HierarchyCreatePopup");
            }
            if (ImGui::BeginPopup("HierarchyCreatePopup"))
            {
                renderContextMenu(state, nullptr);
                ImGui::EndPopup();
            }

            ImGui::Separator();

            // Right-click in empty area
            if (ImGui::BeginPopupContextWindow("HierarchyEmptyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                renderContextMenu(state, nullptr);
                ImGui::EndPopup();
            }

            // Drag-and-drop to root (empty area): unparent GameObject or instantiate Prefab at root
            if (ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->ContentRegionRect, ImGui::GetID("HierarchyEmptyDropTarget")))
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_GAMEOBJECT_REPARENT"))
                {
                    int draggedId = *(const int *)payload->Data;
                    EditorObject *draggedObj = state.findObject(draggedId);
                    if (draggedObj)
                    {
                        draggedObj->parentId = -1;
                        state.isDirty = true;
                    }
                }
                else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_PREFAB"))
                {
                    std::string prefabPath((const char *)payload->Data);
                    state.instantiatePrefab(prefabPath, {0.0f, 0.0f}, -1);
                }
                else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_ASSET_PATH"))
                {
                    std::string assetPath((const char *)payload->Data);
                    if (std::filesystem::path(assetPath).extension() == ".prefab")
                    {
                        state.instantiatePrefab(assetPath, {0.0f, 0.0f}, -1);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // Render root objects
            renderObjectTree(state, -1);

            // Keyboard shortcuts: Delete, Ctrl+D (Duplicate), F2 (Rename)
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput)
            {
                if (ImGui::IsKeyPressed(ImGuiKey_Delete))
                {
                    for (int id : state.selectedObjectIds)
                    {
                        m_pendingDeleteIds.push_back(id);
                    }
                    if (m_pendingDeleteIds.empty() && state.selectedObjectId != -1)
                    {
                        m_pendingDeleteIds.push_back(state.selectedObjectId);
                    }
                }

                if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D))
                {
                    if (state.selectedObjectId != -1)
                    {
                        state.duplicateObject(state.selectedObjectId);
                    }
                }

                if (ImGui::IsKeyPressed(ImGuiKey_F2))
                {
                    if (state.selectedObjectId != -1)
                    {
                        EditorObject *selObj = state.findObject(state.selectedObjectId);
                        if (selObj)
                        {
                            m_renamingObjId = selObj->id;
                            strncpy(m_renameBuf, selObj->name.c_str(), sizeof(m_renameBuf) - 1);
                            m_renameBuf[sizeof(m_renameBuf) - 1] = '\0';
                        }
                    }
                }
            }

            // Safely execute pending object deletions after tree rendering
            if (!m_pendingDeleteIds.empty())
            {
                for (int id : m_pendingDeleteIds)
                {
                    state.deleteObject(id);
                }
                m_pendingDeleteIds.clear();
            }
        }
        ImGui::End();
    }
} // namespace RoninEditor
