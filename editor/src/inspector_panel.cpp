#include "inspector_panel.h"
#include "project_manager.h"
#include "asset_browser.h"
#include "meta_manager.h"
#include "asset_database.h"
#include "editor_icons.h"
#include <ronin/TextStream.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace RoninEditor
{
    InspectorPanel::InspectorPanel() = default;

    InspectorPanel::~InspectorPanel()
    {
        stopAudioPreview();
    }

    void InspectorPanel::playAudioPreview(const std::string &clipPath)
    {
        stopAudioPreview();
        if (clipPath.empty()) return;

        std::filesystem::path fullPath = clipPath;
        if (!std::filesystem::exists(fullPath))
        {
            auto root = ProjectManager::GetInstance().GetRootPath();
            fullPath = root / clipPath;
        }

        if (std::filesystem::exists(fullPath))
        {
            m_previewChunk = Mix_LoadWAV(fullPath.string().c_str());
            if (m_previewChunk)
            {
                m_previewPlayingPath = clipPath;
                Mix_PlayChannel(-1, m_previewChunk, 0);
            }
        }
    }

    void InspectorPanel::stopAudioPreview()
    {
        if (m_previewChunk)
        {
            Mix_HaltChannel(-1);
            Mix_FreeChunk(m_previewChunk);
            m_previewChunk = nullptr;
            m_previewPlayingPath.clear();
        }
    }

    static void drawVec2Control(const char *label, RoninEngine::Runtime::Vec2 &values, float resetValue = 0.0f, float columnWidth = 80.0f)
    {
        ImGui::PushID(label);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label);
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

        float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        // X Button & Drag
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.75f, 0.20f, 0.20f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.90f, 0.30f, 0.30f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.75f, 0.20f, 0.20f, 1.0f});
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Y Button & Drag
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.20f, 0.65f, 0.20f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.30f, 0.80f, 0.30f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.20f, 0.65f, 0.20f, 1.0f});
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();
    }

    ComponentHeaderResult InspectorPanel::renderComponentHeader(
        const char *title,
        int componentId,
        ComponentType type,
        bool allowRemove,
        bool defaultOpen)
    {
        ComponentHeaderResult res;
        ImGui::PushID(title);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowOverlap;
        if (defaultOpen)
            flags |= ImGuiTreeNodeFlags_DefaultOpen;

        ImGui::SetNextItemAllowOverlap();

        char headerLabel[128];
        if (componentId > 0)
            std::snprintf(headerLabel, sizeof(headerLabel), "%s  [ID: %d]", title, componentId);
        else
            std::snprintf(headerLabel, sizeof(headerLabel), "%s", title);

        res.isOpen = ImGui::CollapsingHeader(headerLabel, flags);

        // Calculate positions for buttons on the right side of the header
        float resetW = 44.0f;
        float menuW = 26.0f;
        float removeW = allowRemove ? 24.0f : 0.0f;
        float spacing = 4.0f;
        float totalW = resetW + spacing + menuW + (allowRemove ? (spacing + removeW) : 0.0f) + 8.0f;

        float rightPos = ImGui::GetWindowContentRegionMax().x - totalW;
        if (rightPos > ImGui::GetCursorPosX())
        {
            ImGui::SameLine(rightPos);
        }
        else
        {
            ImGui::SameLine();
        }

        // 1. Reset button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.27f, 0.32f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.38f, 0.42f, 0.50f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.20f, 0.24f, 1.0f));
        if (ImGui::SmallButton("Reset"))
        {
            res.reset = true;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reset %s to default values", title);
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0, spacing);

        // 2. Menu options button "..."
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.27f, 0.32f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.38f, 0.42f, 0.50f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.20f, 0.24f, 1.0f));
        if (ImGui::SmallButton("..."))
        {
            ImGui::OpenPopup("ComponentPopup");
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Component actions (Copy, Paste, Duplicate)");
        ImGui::PopStyleColor(3);

        auto handleMenu = [&]() {
            if (ImGui::MenuItem("Reset Component"))
            {
                res.reset = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Copy Component"))
            {
                res.copy = true;
            }
            bool canPaste = m_clipboard.hasData && (m_clipboard.type == type);
            if (ImGui::MenuItem("Paste Component Values", nullptr, false, canPaste))
            {
                res.paste = true;
            }
            if (type != ComponentType::Transform)
            {
                if (ImGui::MenuItem("Duplicate Component"))
                {
                    res.duplicate = true;
                }
            }
            if (allowRemove)
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Remove Component"))
                {
                    res.remove = true;
                }
            }
        };

        if (ImGui::BeginPopup("ComponentPopup"))
        {
            handleMenu();
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupContextItem("HeaderCtx"))
        {
            handleMenu();
            ImGui::EndPopup();
        }

        // 3. Remove button "X"
        if (allowRemove)
        {
            ImGui::SameLine(0, spacing);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.42f, 0.18f, 0.18f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.78f, 0.22f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.55f, 0.14f, 0.14f, 1.0f));
            if (ImGui::SmallButton("X"))
            {
                res.remove = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove %s component", title);
            ImGui::PopStyleColor(3);
        }

        ImGui::PopID();
        return res;
    }

    void InspectorPanel::duplicateComponentToNewObject(EditorState &state, EditorObject &obj, ComponentType type, const char *title)
    {
        EditorWorld &w = state.activeWorld();
        EditorObject clone;
        clone.id = w.allocateObjectId();
        clone.name = obj.name + " (" + title + ")";
        clone.parentId = obj.parentId;
        clone.position = obj.position + RoninEngine::Runtime::Vec2{0.5f, -0.5f};
        clone.angle = obj.angle;
        clone.scale = obj.scale;
        clone.zOrder = obj.zOrder;
        clone.active = true;

        switch (type)
        {
        case ComponentType::SpriteRenderer:
            clone.sprite = obj.sprite;
            clone.sprite.componentId = w.allocateComponentId();
            break;
        case ComponentType::Camera2D:
            clone.camera = obj.camera;
            clone.camera.componentId = w.allocateComponentId();
            break;
        case ComponentType::Light2D:
            clone.light = obj.light;
            clone.light.componentId = w.allocateComponentId();
            break;
        case ComponentType::Terrain2D:
            clone.terrain = obj.terrain;
            clone.terrain.componentId = w.allocateComponentId();
            break;
        case ComponentType::ParticleSystem:
            clone.particles = obj.particles;
            clone.particles.componentId = w.allocateComponentId();
            break;
        case ComponentType::PhysicsController:
            clone.physics = obj.physics;
            clone.physics.componentId = w.allocateComponentId();
            break;
        case ComponentType::Rigidbody2D:
            clone.rigidbody2D = obj.rigidbody2D;
            clone.rigidbody2D.componentId = w.allocateComponentId();
            break;
        case ComponentType::BoxCollider2D:
            clone.boxCollider2D = obj.boxCollider2D;
            clone.boxCollider2D.componentId = w.allocateComponentId();
            break;
        case ComponentType::AudioSource:
            clone.audio = obj.audio;
            clone.audio.componentId = w.allocateComponentId();
            break;
        case ComponentType::MotionScript:
            clone.script = obj.script;
            clone.script.componentId = w.allocateComponentId();
            break;
        default:
            break;
        }

        w.objects.push_back(clone);
        state.selectedObjectId = clone.id;
        state.isDirty = true;
        state.logInfo("Duplicated component into new GameObject: " + clone.name + " [ID: " + std::to_string(clone.id) + "]");
    }

    void InspectorPanel::renderTransformComponent(EditorState &state, EditorObject &obj)
    {
        ComponentHeaderResult hr = renderComponentHeader("Transform", obj.id, ComponentType::Transform, false);
        if (hr.reset)
        {
            obj.position = {0.0f, 0.0f};
            obj.angle = 0.0f;
            obj.scale = {1.0f, 1.0f};
            state.isDirty = true;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Transform;
            m_clipboard.position = obj.position;
            m_clipboard.angle = obj.angle;
            m_clipboard.scale = obj.scale;
            state.logInfo("Copied Transform values");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::Transform)
        {
            obj.position = m_clipboard.position;
            obj.angle = m_clipboard.angle;
            obj.scale = m_clipboard.scale;
            state.isDirty = true;
            state.logInfo("Pasted Transform values");
        }

        if (hr.isOpen)
        {
            drawVec2Control("Position", obj.position, 0.0f);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 80.0f);
            ImGui::Text("Rotation");
            ImGui::NextColumn();
            if (ImGui::DragFloat("##Rotation", &obj.angle, 1.0f, -360.0f, 360.0f, "%.1f°"))
            {
                state.isDirty = true;
            }
            ImGui::Columns(1);

            drawVec2Control("Scale", obj.scale, 1.0f);
        }
    }

    void InspectorPanel::renderSpriteRendererComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.sprite.enabled) return;
        if (obj.sprite.componentId == 0) obj.sprite.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Sprite Renderer", obj.sprite.componentId, ComponentType::SpriteRenderer, true);
        if (hr.reset)
        {
            int oldId = obj.sprite.componentId;
            obj.sprite = SpriteRendererData{};
            obj.sprite.enabled = true;
            obj.sprite.componentId = oldId;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.sprite.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::SpriteRenderer;
            m_clipboard.sprite = obj.sprite;
            state.logInfo("Copied Sprite Renderer [ID: " + std::to_string(obj.sprite.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::SpriteRenderer)
        {
            int oldId = obj.sprite.componentId;
            obj.sprite = m_clipboard.sprite;
            obj.sprite.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.sprite.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Sprite Renderer values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::SpriteRenderer;
            m_clipboard.sprite = obj.sprite;
            duplicateComponentToNewObject(state, obj, ComponentType::SpriteRenderer, "Sprite Renderer");
            return;
        }

        if (hr.isOpen)
        {
            // Sprite Asset Slot with Asset ID
            ImGui::Text("Sprite Asset:");
            std::string dispSprite;
            std::string spriteTooltip;
            if (obj.sprite.spritePath.empty())
            {
                dispSprite = "(None / Click to Select or Drop Asset)";
                spriteTooltip = "Click to open Asset Picker popup or drag & drop a Sprite/Texture";
            }
            else
            {
                dispSprite = std::filesystem::path(obj.sprite.spritePath).stem().string();
                char idBuf[32];
                std::snprintf(idBuf, sizeof(idBuf), "0x%08X", obj.sprite.assetId);
                std::string guid = AssetDatabase::GetInstance().GetGuidByPath(obj.sprite.spritePath);
                spriteTooltip = "Name: " + std::filesystem::path(obj.sprite.spritePath).filename().string() +
                                "\nPath: " + obj.sprite.spritePath +
                                "\nAsset ID: " + idBuf +
                                (guid.empty() ? "" : ("\nGUID: " + guid));
            }

            float rightButtonsWidth = obj.sprite.spritePath.empty() ? 105.0f : 132.0f;
            float mainBtnWidth = std::max(80.0f, ImGui::GetContentRegionAvail().x - rightButtonsWidth);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.28f, 1.0f));
            if (ImGui::Button(dispSprite.c_str(), ImVec2(mainBtnWidth, 0.0f)))
            {
                m_showSpritePicker = true;
                m_openSpritePickerTrigger = true;
                m_pendingSpritePath = obj.sprite.spritePath;
                m_pendingSpriteAssetId = obj.sprite.assetId;
                m_pickerFilter[0] = '\0';
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", spriteTooltip.c_str());

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_ASSET_PATH"))
                {
                    obj.sprite.spritePath = static_cast<const char *>(payload->Data);
                    obj.sprite.assetId = AssetBrowser::CalculateAssetId(obj.sprite.spritePath);
                    obj.sprite.primitiveShape = 3;
                    state.isDirty = true;
                }
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_SPRITE"))
                {
                    obj.sprite.spritePath = static_cast<const char *>(payload->Data);
                    obj.sprite.assetId = AssetBrowser::CalculateAssetId(obj.sprite.spritePath);
                    obj.sprite.primitiveShape = 3;
                    state.isDirty = true;
                }
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
                {
                    obj.sprite.spritePath = static_cast<const char *>(payload->Data);
                    obj.sprite.assetId = AssetBrowser::CalculateAssetId(obj.sprite.spritePath);
                    obj.sprite.primitiveShape = 3;
                    state.isDirty = true;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            if (ImGui::Button("->##PingSprite", ImVec2(26.0f, 0.0f)))
            {
                if (!obj.sprite.spritePath.empty())
                {
                    std::filesystem::path full = obj.sprite.spritePath;
                    if (!std::filesystem::exists(full))
                        full = ProjectManager::GetInstance().GetRootPath() / obj.sprite.spritePath;
                    if (onPingAsset) onPingAsset(full);
                    state.selectAsset(full);
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Ping and inspect sprite asset in Asset Browser");

            ImGui::SameLine();
            if (ImGui::Button("Select...##PickSprite"))
            {
                m_showSpritePicker = true;
                m_openSpritePickerTrigger = true;
                m_pendingSpritePath = obj.sprite.spritePath;
                m_pendingSpriteAssetId = obj.sprite.assetId;
                m_pickerFilter[0] = '\0';
            }

            if (!obj.sprite.spritePath.empty())
            {
                ImGui::SameLine();
                if (ImGui::Button("X##ClearSprite", ImVec2(22.0f, 0.0f)))
                {
                    obj.sprite.spritePath.clear();
                    obj.sprite.assetId = 0;
                    state.isDirty = true;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clear sprite asset");

                if (ImGui::Button("Edit in Sprite Editor"))
                {
                    if (onOpenSpriteEditor)
                    {
                        onOpenSpriteEditor(obj.sprite.spritePath);
                    }
                }
            }

            const char *shapes[] = {"Rectangle", "Circle", "Triangle", "Custom Image"};
            ImGui::Combo("Shape", &obj.sprite.primitiveShape, shapes, 4);

            drawVec2Control("Size", obj.sprite.size, 1.0f);

            float col[4] = {
                obj.sprite.color.r / 255.0f,
                obj.sprite.color.g / 255.0f,
                obj.sprite.color.b / 255.0f,
                obj.sprite.color.a / 255.0f};
            if (ImGui::ColorEdit4("Color", col))
            {
                obj.sprite.color.r = static_cast<uint8_t>(col[0] * 255.0f);
                obj.sprite.color.g = static_cast<uint8_t>(col[1] * 255.0f);
                obj.sprite.color.b = static_cast<uint8_t>(col[2] * 255.0f);
                obj.sprite.color.a = static_cast<uint8_t>(col[3] * 255.0f);
                state.isDirty = true;
            }

            bool flipH = (obj.sprite.flip & 1) != 0;
            bool flipV = (obj.sprite.flip & 2) != 0;
            if (ImGui::Checkbox("Flip X", &flipH))
            {
                obj.sprite.flip = (flipH ? 1 : 0) | (flipV ? 2 : 0);
                state.isDirty = true;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("Flip Y", &flipV))
            {
                obj.sprite.flip = (flipH ? 1 : 0) | (flipV ? 2 : 0);
                state.isDirty = true;
            }

            const char *types[] = {"Simple", "Tile"};
            ImGui::Combo("Draw Mode", &obj.sprite.renderType, types, 2);
        }
    }

    void InspectorPanel::renderCamera2DComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.camera.enabled) return;
        if (obj.camera.componentId == 0) obj.camera.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Camera 2D", obj.camera.componentId, ComponentType::Camera2D, true);
        if (hr.reset)
        {
            int oldId = obj.camera.componentId;
            obj.camera = Camera2DData{};
            obj.camera.enabled = true;
            obj.camera.componentId = oldId;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.camera.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Camera2D;
            m_clipboard.camera = obj.camera;
            state.logInfo("Copied Camera 2D [ID: " + std::to_string(obj.camera.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::Camera2D)
        {
            int oldId = obj.camera.componentId;
            obj.camera = m_clipboard.camera;
            obj.camera.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.camera.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Camera 2D values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Camera2D;
            m_clipboard.camera = obj.camera;
            duplicateComponentToNewObject(state, obj, ComponentType::Camera2D, "Camera 2D");
            return;
        }

        if (hr.isOpen)
        {
            ImGui::Checkbox("Main Camera", &obj.camera.mainCamera);
            if (ImGui::DragFloat("Zoom (Size)", &obj.camera.zoom, 0.05f, 0.0001f, 0.0f, "%.2f"))
            {
                obj.camera.zoom = std::max(0.0001f, obj.camera.zoom);
                obj.camera.scale = RoninEngine::Runtime::Vec2(obj.camera.zoom, obj.camera.zoom);
                state.isDirty = true;
            }
            if (ImGui::DragFloat2("Scale", &obj.camera.scale.x, 0.05f, 0.0001f, 0.0f, "%.2f"))
            {
                obj.camera.scale.x = std::max(0.0001f, obj.camera.scale.x);
                obj.camera.scale.y = std::max(0.0001f, obj.camera.scale.y);
                obj.camera.zoom = obj.camera.scale.x;
                state.isDirty = true;
            }

            ImGui::Checkbox("Clear Flags (Background)", &obj.camera.backclear);
            if (obj.camera.backclear)
            {
                float col[4] = {
                    obj.camera.backcolor.r / 255.0f,
                    obj.camera.backcolor.g / 255.0f,
                    obj.camera.backcolor.b / 255.0f,
                    obj.camera.backcolor.a / 255.0f};
                if (ImGui::ColorEdit4("Background Color", col))
                {
                    obj.camera.backcolor.r = static_cast<uint8_t>(col[0] * 255.0f);
                    obj.camera.backcolor.g = static_cast<uint8_t>(col[1] * 255.0f);
                    obj.camera.backcolor.b = static_cast<uint8_t>(col[2] * 255.0f);
                    obj.camera.backcolor.a = static_cast<uint8_t>(col[3] * 255.0f);
                    state.isDirty = true;
                }
            }

            ImGui::Checkbox("Visible Grids", &obj.camera.visibleGrids);
            ImGui::Checkbox("Visible Borders", &obj.camera.visibleBorders);
            ImGui::Checkbox("Visible Objects", &obj.camera.visibleObjects);
        }
    }

    void InspectorPanel::renderLight2DComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.light.enabled) return;
        if (obj.light.componentId == 0) obj.light.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Light 2D", obj.light.componentId, ComponentType::Light2D, true);
        if (hr.reset)
        {
            int oldId = obj.light.componentId;
            obj.light = Light2DData{};
            obj.light.enabled = true;
            obj.light.componentId = oldId;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.light.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Light2D;
            m_clipboard.light = obj.light;
            state.logInfo("Copied Light 2D [ID: " + std::to_string(obj.light.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::Light2D)
        {
            int oldId = obj.light.componentId;
            obj.light = m_clipboard.light;
            obj.light.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.light.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Light 2D values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Light2D;
            m_clipboard.light = obj.light;
            duplicateComponentToNewObject(state, obj, ComponentType::Light2D, "Light 2D");
            return;
        }

        if (hr.isOpen)
        {
            const char *lightTypes[] = {"Simple Point Light", "Directional Light", "Spotlight (Cone)"};
            ImGui::Combo("Light Type", &obj.light.lightType, lightTypes, 3);

            ImGui::SliderFloat("Intensity", &obj.light.intensity, 0.0f, 5.0f, "%.2f");
            ImGui::DragFloat("Range", &obj.light.range, 0.1f, 0.1f, 100.0f, "%.1f m");

            if (obj.light.lightType == 2)
            {
                ImGui::SliderFloat("Cone Angle", &obj.light.angleDegrees, 5.0f, 180.0f, "%.1f°");
            }

            float col[4] = {
                obj.light.color.r / 255.0f,
                obj.light.color.g / 255.0f,
                obj.light.color.b / 255.0f,
                obj.light.color.a / 255.0f};
            if (ImGui::ColorEdit4("Light Color", col))
            {
                obj.light.color.r = static_cast<uint8_t>(col[0] * 255.0f);
                obj.light.color.g = static_cast<uint8_t>(col[1] * 255.0f);
                obj.light.color.b = static_cast<uint8_t>(col[2] * 255.0f);
                obj.light.color.a = static_cast<uint8_t>(col[3] * 255.0f);
                state.isDirty = true;
            }
        }
    }

    void InspectorPanel::renderTerrain2DComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.terrain.enabled) return;
        if (obj.terrain.componentId == 0) obj.terrain.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Terrain 2D", obj.terrain.componentId, ComponentType::Terrain2D, true);
        if (hr.reset)
        {
            int oldId = obj.terrain.componentId;
            obj.terrain = Terrain2DData{};
            obj.terrain.enabled = true;
            obj.terrain.componentId = oldId;
            obj.terrain.ensureGridInitialized(0);
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.terrain.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Terrain2D;
            m_clipboard.terrain = obj.terrain;
            state.logInfo("Copied Terrain 2D [ID: " + std::to_string(obj.terrain.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::Terrain2D)
        {
            int oldId = obj.terrain.componentId;
            obj.terrain = m_clipboard.terrain;
            obj.terrain.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.terrain.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Terrain 2D values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Terrain2D;
            m_clipboard.terrain = obj.terrain;
            duplicateComponentToNewObject(state, obj, ComponentType::Terrain2D, "Terrain 2D");
            return;
        }

        if (hr.isOpen)
        {
            ImGui::Text("Grid Size: %d x %d cells", obj.terrain.width, obj.terrain.length);
            ImGui::DragFloat("Cell Scale", &obj.terrain.cellScale, 0.05f, 0.1f, 10.0f, "%.2f units");

            float col[4] = {
                obj.terrain.tileColor.r / 255.0f,
                obj.terrain.tileColor.g / 255.0f,
                obj.terrain.tileColor.b / 255.0f,
                obj.terrain.tileColor.a / 255.0f};
            if (ImGui::ColorEdit4("Base Color", col))
            {
                obj.terrain.tileColor.r = static_cast<uint8_t>(col[0] * 255.0f);
                obj.terrain.tileColor.g = static_cast<uint8_t>(col[1] * 255.0f);
                obj.terrain.tileColor.b = static_cast<uint8_t>(col[2] * 255.0f);
                obj.terrain.tileColor.a = static_cast<uint8_t>(col[3] * 255.0f);
                state.isDirty = true;
            }

            ImGui::Checkbox("Generate Border Colliders", &obj.terrain.generateBorderColliders);

            if (ImGui::Button("Open Dedicated Terrain Editor", ImVec2(ImGui::GetContentRegionAvail().x, 26.0f)))
            {
                state.isTerrainEditorOpen = true;
            }
        }
    }

    void InspectorPanel::renderParticleSystemComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.particles.enabled) return;
        if (obj.particles.componentId == 0) obj.particles.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Particle System", obj.particles.componentId, ComponentType::ParticleSystem, true);
        if (hr.reset)
        {
            int oldId = obj.particles.componentId;
            obj.particles = ParticleSystemData{};
            obj.particles.enabled = true;
            obj.particles.componentId = oldId;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.particles.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::ParticleSystem;
            m_clipboard.particles = obj.particles;
            state.logInfo("Copied Particle System [ID: " + std::to_string(obj.particles.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::ParticleSystem)
        {
            int oldId = obj.particles.componentId;
            obj.particles = m_clipboard.particles;
            obj.particles.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.particles.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Particle System values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::ParticleSystem;
            m_clipboard.particles = obj.particles;
            duplicateComponentToNewObject(state, obj, ComponentType::ParticleSystem, "Particle System");
            return;
        }

        if (hr.isOpen)
        {
            ImGui::Checkbox("Emit", &obj.particles.emit);
            ImGui::SameLine();
            ImGui::Checkbox("Loop", &obj.particles.loop);
            ImGui::SameLine();
            ImGui::Checkbox("Rotate", &obj.particles.rotate);

            ImGui::DragFloat("Speed", &obj.particles.speed, 0.1f, 0.1f, 50.0f, "%.1f");
            ImGui::DragFloat("Duration", &obj.particles.duration, 0.1f, 0.1f, 30.0f, "%.1f s");
            ImGui::DragFloat("Interval", &obj.particles.interval, 0.01f, 0.01f, 2.0f, "%.3f s");
            ImGui::DragInt("Max Particles", &obj.particles.maxLimit, 1, 1, 1024);

            float sc[4] = {obj.particles.startColor.r / 255.0f, obj.particles.startColor.g / 255.0f, obj.particles.startColor.b / 255.0f, obj.particles.startColor.a / 255.0f};
            if (ImGui::ColorEdit4("Start Color", sc))
            {
                obj.particles.startColor.r = static_cast<uint8_t>(sc[0] * 255.0f);
                obj.particles.startColor.g = static_cast<uint8_t>(sc[1] * 255.0f);
                obj.particles.startColor.b = static_cast<uint8_t>(sc[2] * 255.0f);
                obj.particles.startColor.a = static_cast<uint8_t>(sc[3] * 255.0f);
                state.isDirty = true;
            }

            float ec[4] = {obj.particles.endColor.r / 255.0f, obj.particles.endColor.g / 255.0f, obj.particles.endColor.b / 255.0f, obj.particles.endColor.a / 255.0f};
            if (ImGui::ColorEdit4("End Color", ec))
            {
                obj.particles.endColor.r = static_cast<uint8_t>(ec[0] * 255.0f);
                obj.particles.endColor.g = static_cast<uint8_t>(ec[1] * 255.0f);
                obj.particles.endColor.b = static_cast<uint8_t>(ec[2] * 255.0f);
                obj.particles.endColor.a = static_cast<uint8_t>(ec[3] * 255.0f);
                state.isDirty = true;
            }

            drawVec2Control("Start Size", obj.particles.startSize, 0.35f);
            drawVec2Control("End Size", obj.particles.endSize, 0.05f);
        }
    }

    void InspectorPanel::renderPhysicsComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.physics.colliderEnabled && !obj.physics.moveControllerEnabled) return;
        if (obj.physics.componentId == 0) obj.physics.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Physics Controller", obj.physics.componentId, ComponentType::PhysicsController, true);
        if (hr.reset)
        {
            int oldId = obj.physics.componentId;
            obj.physics = PhysicsControllerData{};
            obj.physics.componentId = oldId;
            obj.physics.colliderEnabled = true;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.physics.colliderEnabled = false;
            obj.physics.moveControllerEnabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::PhysicsController;
            m_clipboard.physics = obj.physics;
            state.logInfo("Copied Physics Controller [ID: " + std::to_string(obj.physics.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::PhysicsController)
        {
            int oldId = obj.physics.componentId;
            obj.physics = m_clipboard.physics;
            obj.physics.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            state.isDirty = true;
            state.logInfo("Pasted Physics Controller values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::PhysicsController;
            m_clipboard.physics = obj.physics;
            duplicateComponentToNewObject(state, obj, ComponentType::PhysicsController, "Physics Controller");
            return;
        }

        if (hr.isOpen)
        {
            ImGui::Checkbox("Collider Enabled", &obj.physics.colliderEnabled);
            if (obj.physics.colliderEnabled)
            {
                const char *types[] = {"Box Collider 2D", "Circle Collider 2D"};
                ImGui::Combo("Collider Type", &obj.physics.colliderType, types, 2);

                if (obj.physics.colliderType == 0)
                {
                    drawVec2Control("Box Size", obj.physics.colliderSize, 1.0f);
                }
                else
                {
                    ImGui::DragFloat("Radius", &obj.physics.colliderRadius, 0.05f, 0.01f, 100.0f, "%.2f");
                }
                ImGui::DragInt("Layer Mask", &obj.physics.collisionLayer, 1, 0, 31);
            }

            ImGui::Separator();
            ImGui::Checkbox("MoveController2D (Player Input)", &obj.physics.moveControllerEnabled);
            if (obj.physics.moveControllerEnabled)
            {
                ImGui::DragFloat("Move Speed", &obj.physics.moveSpeed, 0.1f, 0.1f, 50.0f, "%.1f units/s");
            }
        }
    }

    void InspectorPanel::renderRigidbody2DComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.rigidbody2D.enabled) return;
        if (obj.rigidbody2D.componentId == 0) obj.rigidbody2D.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Rigidbody 2D", obj.rigidbody2D.componentId, ComponentType::Rigidbody2D, true);
        if (hr.reset)
        {
            int oldId = obj.rigidbody2D.componentId;
            obj.rigidbody2D.reset();
            obj.rigidbody2D.enabled = true;
            obj.rigidbody2D.componentId = oldId;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.rigidbody2D.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Rigidbody2D;
            m_clipboard.rigidbody2D = obj.rigidbody2D;
            state.logInfo("Copied Rigidbody 2D [ID: " + std::to_string(obj.rigidbody2D.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::Rigidbody2D)
        {
            int oldId = obj.rigidbody2D.componentId;
            obj.rigidbody2D = m_clipboard.rigidbody2D;
            obj.rigidbody2D.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.rigidbody2D.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Rigidbody 2D values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::Rigidbody2D;
            m_clipboard.rigidbody2D = obj.rigidbody2D;
            duplicateComponentToNewObject(state, obj, ComponentType::Rigidbody2D, "Rigidbody 2D");
            return;
        }

        if (hr.isOpen)
        {
            const char *bodyTypes[] = {"Dynamic", "Kinematic", "Static"};
            if (ImGui::Combo("Body Type", &obj.rigidbody2D.bodyType, bodyTypes, 3))
            {
                state.isDirty = true;
            }

            ImGui::Checkbox("Simulated", &obj.rigidbody2D.simulated);

            if (obj.rigidbody2D.bodyType == 0) // Dynamic
            {
                if (ImGui::DragFloat("Mass", &obj.rigidbody2D.mass, 0.1f, 0.001f, 10000.0f, "%.2f kg"))
                {
                    obj.rigidbody2D.mass = std::max(0.001f, obj.rigidbody2D.mass);
                    state.isDirty = true;
                }
                if (ImGui::DragFloat("Linear Drag", &obj.rigidbody2D.linearDamping, 0.05f, 0.0f, 100.0f, "%.2f"))
                {
                    state.isDirty = true;
                }
                if (ImGui::DragFloat("Angular Drag", &obj.rigidbody2D.angularDamping, 0.05f, 0.0f, 100.0f, "%.2f"))
                {
                    state.isDirty = true;
                }
                if (ImGui::DragFloat("Gravity Scale", &obj.rigidbody2D.gravityScale, 0.1f, -20.0f, 20.0f, "%.2f"))
                {
                    state.isDirty = true;
                }

                ImGui::Separator();
                ImGui::Text("Constraints:");
                if (ImGui::Checkbox("Freeze Rotation (Z)", &obj.rigidbody2D.freezeRotation))
                {
                    state.isDirty = true;
                }
            }

            if (obj.rigidbody2D.bodyType != 2) // Dynamic or Kinematic
            {
                drawVec2Control("Velocity", obj.rigidbody2D.velocity, 0.0f);
                if (ImGui::DragFloat("Angular Velocity", &obj.rigidbody2D.angularVelocity, 1.0f, -720.0f, 720.0f, "%.1f °/s"))
                {
                    state.isDirty = true;
                }
            }
        }
    }

    void InspectorPanel::renderBoxCollider2DComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.boxCollider2D.enabled) return;
        if (obj.boxCollider2D.componentId == 0) obj.boxCollider2D.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Box Collider 2D", obj.boxCollider2D.componentId, ComponentType::BoxCollider2D, true);
        if (hr.reset)
        {
            int oldId = obj.boxCollider2D.componentId;
            obj.boxCollider2D.reset();
            obj.boxCollider2D.enabled = true;
            obj.boxCollider2D.componentId = oldId;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.boxCollider2D.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::BoxCollider2D;
            m_clipboard.boxCollider2D = obj.boxCollider2D;
            state.logInfo("Copied Box Collider 2D [ID: " + std::to_string(obj.boxCollider2D.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::BoxCollider2D)
        {
            int oldId = obj.boxCollider2D.componentId;
            obj.boxCollider2D = m_clipboard.boxCollider2D;
            obj.boxCollider2D.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.boxCollider2D.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Box Collider 2D values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::BoxCollider2D;
            m_clipboard.boxCollider2D = obj.boxCollider2D;
            duplicateComponentToNewObject(state, obj, ComponentType::BoxCollider2D, "Box Collider 2D");
            return;
        }

        if (hr.isOpen)
        {
            drawVec2Control("Size", obj.boxCollider2D.size, 1.0f);
            drawVec2Control("Offset", obj.boxCollider2D.offset, 0.0f);

            if (ImGui::Checkbox("Is Trigger", &obj.boxCollider2D.isTrigger))
            {
                state.isDirty = true;
            }

            if (ImGui::SliderFloat("Friction", &obj.boxCollider2D.friction, 0.0f, 1.0f, "%.2f"))
            {
                state.isDirty = true;
            }

            if (ImGui::SliderFloat("Bounciness (Restitution)", &obj.boxCollider2D.bounciness, 0.0f, 1.0f, "%.2f"))
            {
                state.isDirty = true;
            }

            if (ImGui::DragFloat("Density", &obj.boxCollider2D.density, 0.1f, 0.001f, 100.0f, "%.2f"))
            {
                state.isDirty = true;
            }
        }
    }

    void InspectorPanel::renderAudioSourceComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.audio.enabled) return;
        if (obj.audio.componentId == 0) obj.audio.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Audio Source", obj.audio.componentId, ComponentType::AudioSource, true);
        if (hr.reset)
        {
            int oldId = obj.audio.componentId;
            obj.audio = AudioSourceData{};
            obj.audio.enabled = true;
            obj.audio.componentId = oldId;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.audio.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::AudioSource;
            m_clipboard.audio = obj.audio;
            state.logInfo("Copied Audio Source [ID: " + std::to_string(obj.audio.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::AudioSource)
        {
            int oldId = obj.audio.componentId;
            obj.audio = m_clipboard.audio;
            obj.audio.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.audio.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Audio Source values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::AudioSource;
            m_clipboard.audio = obj.audio;
            duplicateComponentToNewObject(state, obj, ComponentType::AudioSource, "Audio Source");
            return;
        }

        if (hr.isOpen)
        {
            ImGui::Text("Audio Clip:");
            std::string dispAudio;
            std::string audioTooltip;
            if (obj.audio.clipPath.empty())
            {
                dispAudio = "(None / Click to Select or Drop Audio)";
                audioTooltip = "Click to open Audio Asset Picker popup or drag & drop an audio file";
            }
            else
            {
                dispAudio = std::filesystem::path(obj.audio.clipPath).stem().string();
                char idBuf[32];
                std::snprintf(idBuf, sizeof(idBuf), "0x%08X", obj.audio.clipAssetId);
                std::string guid = AssetDatabase::GetInstance().GetGuidByPath(obj.audio.clipPath);
                audioTooltip = "Name: " + std::filesystem::path(obj.audio.clipPath).filename().string() +
                               "\nPath: " + obj.audio.clipPath +
                               "\nAsset ID: " + idBuf +
                               (guid.empty() ? "" : ("\nGUID: " + guid));
            }

            float rightButtonsWidth = obj.audio.clipPath.empty() ? 105.0f : 132.0f;
            float mainBtnWidth = std::max(80.0f, ImGui::GetContentRegionAvail().x - rightButtonsWidth);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.20f, 0.16f, 1.0f));
            if (ImGui::Button(dispAudio.c_str(), ImVec2(mainBtnWidth, 0.0f)))
            {
                m_showAudioPicker = true;
                m_openAudioPickerTrigger = true;
                m_pendingAudioPath = obj.audio.clipPath;
                m_pendingAudioAssetId = obj.audio.clipAssetId;
                m_pickerFilter[0] = '\0';
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", audioTooltip.c_str());

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_ASSET_PATH"))
                {
                    obj.audio.clipPath = static_cast<const char *>(payload->Data);
                    obj.audio.clipAssetId = AssetBrowser::CalculateAssetId(obj.audio.clipPath);
                    state.isDirty = true;
                }
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_AUDIO"))
                {
                    obj.audio.clipPath = static_cast<const char *>(payload->Data);
                    obj.audio.clipAssetId = AssetBrowser::CalculateAssetId(obj.audio.clipPath);
                    state.isDirty = true;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            if (ImGui::Button("->##PingAudio", ImVec2(26.0f, 0.0f)))
            {
                if (!obj.audio.clipPath.empty())
                {
                    std::filesystem::path full = obj.audio.clipPath;
                    if (!std::filesystem::exists(full))
                        full = ProjectManager::GetInstance().GetRootPath() / obj.audio.clipPath;
                    if (onPingAsset) onPingAsset(full);
                    state.selectAsset(full);
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Ping and inspect audio asset in Asset Browser");

            ImGui::SameLine();
            if (ImGui::Button("Select...##PickAudio"))
            {
                m_showAudioPicker = true;
                m_openAudioPickerTrigger = true;
                m_pendingAudioPath = obj.audio.clipPath;
                m_pendingAudioAssetId = obj.audio.clipAssetId;
                m_pickerFilter[0] = '\0';
            }

            if (!obj.audio.clipPath.empty())
            {
                ImGui::SameLine();
                if (ImGui::Button("X##ClearAudio", ImVec2(22.0f, 0.0f)))
                {
                    stopAudioPreview();
                    obj.audio.clipPath.clear();
                    obj.audio.clipAssetId = 0;
                    state.isDirty = true;
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clear audio clip");

                if (ImGui::SmallButton("Play Preview"))
                {
                    playAudioPreview(obj.audio.clipPath);
                    state.logInfo("Playing audio preview: " + obj.audio.clipPath);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Stop Preview"))
                {
                    stopAudioPreview();
                    state.logInfo("Stopped audio preview");
                }
            }

            ImGui::SliderFloat("Volume", &obj.audio.volume, 0.0f, 1.0f, "%.2f");
            ImGui::Checkbox("Loop", &obj.audio.loop);
            ImGui::SameLine();
            ImGui::Checkbox("Play On Awake", &obj.audio.playOnAwake);
        }
    }

    void InspectorPanel::renderMotionScriptComponent(EditorState &state, EditorObject &obj)
    {
        if (!obj.script.enabled) return;
        if (obj.script.componentId == 0) obj.script.componentId = state.activeWorld().allocateComponentId();

        ComponentHeaderResult hr = renderComponentHeader("Motion Script", obj.script.componentId, ComponentType::MotionScript, true);
        if (hr.reset)
        {
            int oldId = obj.script.componentId;
            obj.script = MotionScriptData{};
            obj.script.enabled = true;
            obj.script.componentId = oldId;
            state.isDirty = true;
        }
        if (hr.remove)
        {
            obj.script.enabled = false;
            state.isDirty = true;
            return;
        }
        if (hr.copy)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::MotionScript;
            m_clipboard.script = obj.script;
            state.logInfo("Copied Motion Script [ID: " + std::to_string(obj.script.componentId) + "]");
        }
        if (hr.paste && m_clipboard.hasData && m_clipboard.type == ComponentType::MotionScript)
        {
            int oldId = obj.script.componentId;
            obj.script = m_clipboard.script;
            obj.script.componentId = oldId ? oldId : state.activeWorld().allocateComponentId();
            obj.script.enabled = true;
            state.isDirty = true;
            state.logInfo("Pasted Motion Script values");
        }
        if (hr.duplicate)
        {
            m_clipboard.hasData = true;
            m_clipboard.type = ComponentType::MotionScript;
            m_clipboard.script = obj.script;
            duplicateComponentToNewObject(state, obj, ComponentType::MotionScript, "Motion Script");
            return;
        }

        if (hr.isOpen)
        {
            drawVec2Control("Velocity", obj.script.linearVelocity, 0.0f);
            ImGui::DragFloat("Angular Speed", &obj.script.angularVelocity, 1.0f, -720.0f, 720.0f, "%.1f °/s");
            ImGui::DragFloat("Pulse Speed", &obj.script.pulseSpeed, 0.1f, 0.0f, 20.0f, "%.1f");
            ImGui::DragFloat("Pulse Amplitude", &obj.script.pulseAmplitude, 0.05f, 0.0f, 2.0f, "%.2f");
        }
    }

    void InspectorPanel::renderAddComponentMenu(EditorState &state, EditorObject &obj)
    {
        ImGui::Spacing();
        ImGui::Separator();
        float btnWidth = ImGui::GetContentRegionAvail().x;
        if (ImGui::Button("Add Component", ImVec2(btnWidth, 30.0f)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            EditorWorld &w = state.activeWorld();

            if (!obj.sprite.enabled && ImGui::MenuItem("Sprite Renderer"))
            {
                obj.sprite.enabled = true;
                if (obj.sprite.componentId == 0) obj.sprite.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.camera.enabled && ImGui::MenuItem("Camera 2D"))
            {
                obj.camera.enabled = true;
                obj.camera.zoom = 1.0f;
                if (obj.camera.componentId == 0) obj.camera.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.light.enabled && ImGui::MenuItem("Light 2D"))
            {
                obj.light.enabled = true;
                obj.light.range = 3.5f;
                if (obj.light.componentId == 0) obj.light.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.terrain.enabled && ImGui::MenuItem("Terrain 2D"))
            {
                obj.terrain.enabled = true;
                obj.terrain.ensureGridInitialized(0);
                if (obj.terrain.componentId == 0) obj.terrain.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.particles.enabled && ImGui::MenuItem("Particle System"))
            {
                obj.particles.enabled = true;
                obj.particles.emit = true;
                if (obj.particles.componentId == 0) obj.particles.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.physics.colliderEnabled && ImGui::MenuItem("Physics 2D Collider"))
            {
                obj.physics.colliderEnabled = true;
                obj.physics.colliderType = 0;
                obj.physics.colliderSize = {1.0f, 1.0f};
                if (obj.physics.componentId == 0) obj.physics.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.rigidbody2D.enabled && ImGui::MenuItem("Rigidbody 2D (Physics)"))
            {
                obj.rigidbody2D.reset();
                obj.rigidbody2D.enabled = true;
                if (obj.rigidbody2D.componentId == 0) obj.rigidbody2D.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.boxCollider2D.enabled && ImGui::MenuItem("Box Collider 2D"))
            {
                obj.boxCollider2D.reset();
                obj.boxCollider2D.enabled = true;
                if (obj.boxCollider2D.componentId == 0) obj.boxCollider2D.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.physics.moveControllerEnabled && ImGui::MenuItem("MoveController2D (Input)"))
            {
                obj.physics.moveControllerEnabled = true;
                if (obj.physics.componentId == 0) obj.physics.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.audio.enabled && ImGui::MenuItem("Audio Source"))
            {
                obj.audio.enabled = true;
                obj.audio.volume = 0.85f;
                if (obj.audio.componentId == 0) obj.audio.componentId = w.allocateComponentId();
                state.isDirty = true;
            }
            if (!obj.script.enabled && ImGui::MenuItem("Motion Script"))
            {
                obj.script.enabled = true;
                if (obj.script.componentId == 0) obj.script.componentId = w.allocateComponentId();
                state.isDirty = true;
            }

            if (m_clipboard.hasData && m_clipboard.type != ComponentType::Transform)
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Paste Component as New"))
                {
                    switch (m_clipboard.type)
                    {
                    case ComponentType::SpriteRenderer:
                        obj.sprite = m_clipboard.sprite;
                        obj.sprite.componentId = w.allocateComponentId();
                        obj.sprite.enabled = true;
                        break;
                    case ComponentType::Camera2D:
                        obj.camera = m_clipboard.camera;
                        obj.camera.componentId = w.allocateComponentId();
                        obj.camera.enabled = true;
                        break;
                    case ComponentType::Light2D:
                        obj.light = m_clipboard.light;
                        obj.light.componentId = w.allocateComponentId();
                        obj.light.enabled = true;
                        break;
                    case ComponentType::Terrain2D:
                        obj.terrain = m_clipboard.terrain;
                        obj.terrain.componentId = w.allocateComponentId();
                        obj.terrain.enabled = true;
                        break;
                    case ComponentType::ParticleSystem:
                        obj.particles = m_clipboard.particles;
                        obj.particles.componentId = w.allocateComponentId();
                        obj.particles.enabled = true;
                        break;
                    case ComponentType::PhysicsController:
                        obj.physics = m_clipboard.physics;
                        obj.physics.componentId = w.allocateComponentId();
                        break;
                    case ComponentType::Rigidbody2D:
                        obj.rigidbody2D = m_clipboard.rigidbody2D;
                        obj.rigidbody2D.componentId = w.allocateComponentId();
                        obj.rigidbody2D.enabled = true;
                        break;
                    case ComponentType::BoxCollider2D:
                        obj.boxCollider2D = m_clipboard.boxCollider2D;
                        obj.boxCollider2D.componentId = w.allocateComponentId();
                        obj.boxCollider2D.enabled = true;
                        break;
                    case ComponentType::AudioSource:
                        obj.audio = m_clipboard.audio;
                        obj.audio.componentId = w.allocateComponentId();
                        obj.audio.enabled = true;
                        break;
                    case ComponentType::MotionScript:
                        obj.script = m_clipboard.script;
                        obj.script.componentId = w.allocateComponentId();
                        obj.script.enabled = true;
                        break;
                    default:
                        break;
                    }
                    state.isDirty = true;
                    state.logInfo("Pasted component as new onto " + obj.name);
                }
            }

            ImGui::EndPopup();
        }
    }

    void InspectorPanel::renderSpritePickerModal(EditorObject &obj, EditorState &state)
    {
        if (m_openSpritePickerTrigger)
        {
            ImGui::OpenPopup("Select Sprite or Texture");
            m_openSpritePickerTrigger = false;
        }

        if (!m_showSpritePicker)
            return;

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(560.0f, 480.0f));

        if (ImGui::BeginPopupModal("Select Sprite or Texture", &m_showSpritePicker, ImGuiWindowFlags_None))
        {
            if (ImGui::IsWindowAppearing())
            {
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::InputTextWithHint("##SpriteSearch", "Search sprites / textures by name or path...", m_pickerFilter, sizeof(m_pickerFilter));
            ImGui::Separator();

            ImGui::BeginChild("SpriteListScroll", ImVec2(0, 310.0f), true);

            bool applySelection = false;

            // (None) option
            bool isNoneSelected = m_pendingSpritePath.empty();
            if (ImGui::Selectable("(None)##SpriteNone", isNoneSelected, ImGuiSelectableFlags_AllowDoubleClick))
            {
                m_pendingSpritePath.clear();
                m_pendingSpriteAssetId = 0;
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    applySelection = true;
                }
            }

            auto root = ProjectManager::GetInstance().GetRootPath();
            std::filesystem::path assetsDir = root / "Assets";

            std::vector<std::filesystem::path> paths;
            if (std::filesystem::exists(assetsDir))
            {
                std::error_code ec;
                for (const auto &it : std::filesystem::recursive_directory_iterator(assetsDir, ec))
                {
                    if (!it.is_directory())
                    {
                        std::string ext = it.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        if (ext == ".sprite" || ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                        {
                            paths.push_back(it.path());
                        }
                    }
                }
            }

            for (const auto &p : paths)
            {
                std::string fname = p.filename().string();
                if (m_pickerFilter[0] != '\0')
                {
                    std::string filterLower = m_pickerFilter;
                    std::string fnameLower = fname;
                    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
                    std::transform(fnameLower.begin(), fnameLower.end(), fnameLower.begin(), ::tolower);
                    if (fnameLower.find(filterLower) == std::string::npos &&
                        p.string().find(filterLower) == std::string::npos)
                        continue;
                }

                std::error_code ec;
                auto rel = std::filesystem::relative(p, root, ec);
                std::string relStr = (ec || rel.empty()) ? p.string() : rel.string();
                uint32_t assetId = AssetBrowser::CalculateAssetId(relStr);

                char idBuf[32];
                std::snprintf(idBuf, sizeof(idBuf), "0x%08X", assetId);

                char label[384];
                std::snprintf(label, sizeof(label), "%s  [ID: %s]##%s", fname.c_str(), idBuf, relStr.c_str());

                bool isSelected = (m_pendingSpritePath == relStr);
                if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    m_pendingSpritePath = relStr;
                    m_pendingSpriteAssetId = assetId;
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        applySelection = true;
                    }
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Relative Path: %s\nAsset ID: %s\n(Double-click to select immediately)", relStr.c_str(), idBuf);
                }
            }

            ImGui::EndChild();

            ImGui::Separator();
            if (m_pendingSpritePath.empty())
            {
                ImGui::TextDisabled("Selected: (None)");
            }
            else
            {
                char idBuf[32];
                std::snprintf(idBuf, sizeof(idBuf), "0x%08X", m_pendingSpriteAssetId);
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Selected: %s [ID: %s]", m_pendingSpritePath.c_str(), idBuf);
            }

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.35f, 1.0f));
            if (ImGui::Button("Select", ImVec2(100.0f, 28.0f)) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
            {
                applySelection = true;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100.0f, 28.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                m_showSpritePicker = false;
                ImGui::CloseCurrentPopup();
            }

            if (applySelection)
            {
                obj.sprite.spritePath = m_pendingSpritePath;
                obj.sprite.assetId = m_pendingSpriteAssetId;
                if (!m_pendingSpritePath.empty())
                {
                    obj.sprite.primitiveShape = 3; // Custom Image
                }
                state.isDirty = true;
                m_showSpritePicker = false;
                ImGui::CloseCurrentPopup();
                state.logInfo("Assigned Sprite to '" + obj.name + "': " + (m_pendingSpritePath.empty() ? "(None)" : m_pendingSpritePath));
            }

            ImGui::EndPopup();
        }
    }

    void InspectorPanel::renderAudioPickerModal(EditorObject &obj, EditorState &state)
    {
        if (m_openAudioPickerTrigger)
        {
            ImGui::OpenPopup("Select Audio Clip");
            m_openAudioPickerTrigger = false;
        }

        if (!m_showAudioPicker)
            return;

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(560.0f, 480.0f));

        if (ImGui::BeginPopupModal("Select Audio Clip", &m_showAudioPicker, ImGuiWindowFlags_None))
        {
            if (ImGui::IsWindowAppearing())
            {
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::InputTextWithHint("##AudioSearch", "Search audio clips by name or path...", m_pickerFilter, sizeof(m_pickerFilter));
            ImGui::Separator();

            ImGui::BeginChild("AudioListScroll", ImVec2(0, 310.0f), true);

            bool applySelection = false;

            // (None) option
            bool isNoneSelected = m_pendingAudioPath.empty();
            if (ImGui::Selectable("(None)##AudioNone", isNoneSelected, ImGuiSelectableFlags_AllowDoubleClick))
            {
                m_pendingAudioPath.clear();
                m_pendingAudioAssetId = 0;
                stopAudioPreview();
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    applySelection = true;
                }
            }

            auto root = ProjectManager::GetInstance().GetRootPath();
            std::filesystem::path assetsDir = root / "Assets";

            std::vector<std::filesystem::path> paths;
            if (std::filesystem::exists(assetsDir))
            {
                std::error_code ec;
                for (const auto &it : std::filesystem::recursive_directory_iterator(assetsDir, ec))
                {
                    if (!it.is_directory())
                    {
                        std::string ext = it.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        if (ext == ".wav" || ext == ".ogg" || ext == ".mp3")
                        {
                            paths.push_back(it.path());
                        }
                    }
                }
            }

            for (const auto &p : paths)
            {
                std::string fname = p.filename().string();
                if (m_pickerFilter[0] != '\0')
                {
                    std::string filterLower = m_pickerFilter;
                    std::string fnameLower = fname;
                    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
                    std::transform(fnameLower.begin(), fnameLower.end(), fnameLower.begin(), ::tolower);
                    if (fnameLower.find(filterLower) == std::string::npos &&
                        p.string().find(filterLower) == std::string::npos)
                        continue;
                }

                std::error_code ec;
                auto rel = std::filesystem::relative(p, root, ec);
                std::string relStr = (ec || rel.empty()) ? p.string() : rel.string();
                uint32_t assetId = AssetBrowser::CalculateAssetId(relStr);

                char idBuf[32];
                std::snprintf(idBuf, sizeof(idBuf), "0x%08X", assetId);

                char label[384];
                std::snprintf(label, sizeof(label), "%s  [ID: %s]##%s", fname.c_str(), idBuf, relStr.c_str());

                bool isSelected = (m_pendingAudioPath == relStr);
                if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    m_pendingAudioPath = relStr;
                    m_pendingAudioAssetId = assetId;
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        applySelection = true;
                    }
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Relative Path: %s\nAsset ID: %s\n(Double-click to select immediately)", relStr.c_str(), idBuf);
                }
            }

            ImGui::EndChild();

            ImGui::Separator();
            if (m_pendingAudioPath.empty())
            {
                ImGui::TextDisabled("Selected: (None)");
            }
            else
            {
                char idBuf[32];
                std::snprintf(idBuf, sizeof(idBuf), "0x%08X", m_pendingAudioAssetId);
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Selected: %s [ID: %s]", m_pendingAudioPath.c_str(), idBuf);
            }

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.35f, 1.0f));
            if (ImGui::Button("Select", ImVec2(100.0f, 28.0f)) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
            {
                applySelection = true;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100.0f, 28.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                stopAudioPreview();
                m_showAudioPicker = false;
                ImGui::CloseCurrentPopup();
            }

            if (!m_pendingAudioPath.empty())
            {
                ImGui::SameLine();
                if (ImGui::Button("Play Preview", ImVec2(100.0f, 28.0f)))
                {
                    playAudioPreview(m_pendingAudioPath);
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop", ImVec2(60.0f, 28.0f)))
                {
                    stopAudioPreview();
                }
            }

            if (applySelection)
            {
                stopAudioPreview();
                obj.audio.clipPath = m_pendingAudioPath;
                obj.audio.clipAssetId = m_pendingAudioAssetId;
                state.isDirty = true;
                m_showAudioPicker = false;
                ImGui::CloseCurrentPopup();
                state.logInfo("Assigned Audio Clip to '" + obj.name + "': " + (m_pendingAudioPath.empty() ? "(None)" : m_pendingAudioPath));
            }

            ImGui::EndPopup();
        }
    }

    void InspectorPanel::renderAssetInspector(EditorState &state)
    {
        std::filesystem::path p = state.selectedAssetPath;
        if (p.empty() || !std::filesystem::exists(p))
        {
            ImGui::TextDisabled("Selected asset does not exist.");
            if (ImGui::Button("Close Asset View"))
            {
                state.clearAssetSelection();
            }
            return;
        }

        // 1. Navigation / Back button if a GameObject is selected
        if (state.selectedObjectId != -1)
        {
            EditorObject *selectedObj = state.findObject(state.selectedObjectId);
            std::string backLabel = "<- Back to Object: " + (selectedObj ? selectedObj->name : "Selected");
            if (ImGui::Button(backLabel.c_str()))
            {
                state.inspectAssetMode = false;
            }
            ImGui::SameLine();
        }
        if (ImGui::SmallButton("Deselect Asset"))
        {
            state.clearAssetSelection();
            return;
        }
        ImGui::Separator();

        // 2. Fetch metadata from AssetDatabase or MetaManager
        const AssetMetadata *md = AssetDatabase::GetInstance().GetMetadataByPath(p);
        MetaFile meta;
        if (!md)
        {
            MetaManager::EnsureMetaFile(p, meta);
        }

        std::string guid = md ? md->guid : meta.guid;
        uint32_t assetId = md ? md->assetId : meta.assetId;
        AssetType assetType = md ? md->type : meta.type;
        std::string filename = p.filename().string();
        std::string stem = p.stem().string();
        std::error_code ec;
        std::uintmax_t fileSize = std::filesystem::file_size(p, ec);

        auto root = ProjectManager::GetInstance().GetRootPath();
        std::filesystem::path rel = std::filesystem::relative(p, root, ec);
        std::string relPathStr = (ec || rel.empty()) ? p.string() : rel.string();

        // Header: Badge + Filename
        const char *badge = MetaManager::GetAssetTypeBadge(assetType);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", badge);
        ImGui::SameLine();
        ImGui::TextUnformatted(filename.c_str());
        ImGui::Separator();

        // Details Table / Group
        if (ImGui::CollapsingHeader("Asset Metadata", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Columns(2, "AssetMetaCols", false);
            ImGui::SetColumnWidth(0, 90.0f);

            ImGui::Text("Asset ID");
            ImGui::NextColumn();
            char idBuf[64];
            std::snprintf(idBuf, sizeof(idBuf), "0x%08X (%u)", assetId, assetId);
            ImGui::TextUnformatted(idBuf);
            ImGui::NextColumn();

            ImGui::Text("GUID");
            ImGui::NextColumn();
            ImGui::TextUnformatted(guid.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Copy##GUID"))
            {
                ImGui::SetClipboardText(guid.c_str());
                state.logInfo("Copied GUID to clipboard: " + guid);
            }
            ImGui::NextColumn();

            ImGui::Text("File Size");
            ImGui::NextColumn();
            if (fileSize < 1024)
                ImGui::Text("%llu bytes", static_cast<unsigned long long>(fileSize));
            else if (fileSize < 1024 * 1024)
                ImGui::Text("%.2f KB", static_cast<float>(fileSize) / 1024.0f);
            else
                ImGui::Text("%.2f MB", static_cast<float>(fileSize) / (1024.0f * 1024.0f));
            ImGui::NextColumn();

            ImGui::Text("Relative Path");
            ImGui::NextColumn();
            ImGui::TextWrapped("%s", relPathStr.c_str());
            ImGui::NextColumn();

            ImGui::Columns(1);
        }

        ImGui::Spacing();
        ImGui::Separator();

        // 3. Asset-Specific Inspector Sections
        if (assetType == AssetType::Audio)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "Audio Clip Inspector");
            ImGui::Spacing();

            // Preview controls
            bool isThisPlaying = (m_previewChunk != nullptr && m_previewPlayingPath == p.string());
            if (!isThisPlaying)
            {
                if (ImGui::Button("Play Preview", ImVec2(120.0f, 30.0f)))
                {
                    playAudioPreview(p.string());
                }
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("Stop Preview", ImVec2(120.0f, 30.0f)))
                {
                    stopAudioPreview();
                }
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Playing...");
            }

            ImGui::Spacing();
            if (state.selectedObjectId != -1)
            {
                EditorObject *curObj = state.findObject(state.selectedObjectId);
                if (curObj && ImGui::Button("Assign to Selected Object's AudioSource"))
                {
                    curObj->audio.clipPath = relPathStr;
                    curObj->audio.clipAssetId = assetId;
                    curObj->audio.enabled = true;
                    state.isDirty = true;
                    state.logInfo("Assigned audio clip to " + curObj->name);
                }
            }
        }
        else if (assetType == AssetType::Sprite || assetType == AssetType::Texture)
        {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "%s Inspector", (assetType == AssetType::Sprite ? "Sprite" : "Texture2D"));
            ImGui::Spacing();

            if (ImGui::Button("Open in Sprite Editor", ImVec2(180.0f, 30.0f)))
            {
                if (onOpenSpriteEditor)
                {
                    onOpenSpriteEditor(p);
                }
            }

            if (state.selectedObjectId != -1)
            {
                EditorObject *curObj = state.findObject(state.selectedObjectId);
                if (curObj && ImGui::Button("Assign to Selected Object's SpriteRenderer"))
                {
                    curObj->sprite.spritePath = relPathStr;
                    curObj->sprite.assetId = assetId;
                    curObj->sprite.primitiveShape = 3;
                    curObj->sprite.enabled = true;
                    state.isDirty = true;
                    state.logInfo("Assigned sprite to " + curObj->name);
                }
            }
        }
        else if (assetType == AssetType::World)
        {
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "World Scene Inspector");
            ImGui::Spacing();
            if (ImGui::Button("Load World in Editor", ImVec2(160.0f, 30.0f)))
            {
                state.loadWorldFromFile(p.string());
                state.logInfo("Loaded world from asset: " + filename);
            }
        }
        else if (assetType == AssetType::Code)
        {
            ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "System Source Code Inspector [Code]");
            ImGui::Spacing();

            std::vector<std::string> lines = RoninEngine::Runtime::TextStream::ReadLines(p.string());
            ImGui::Text("Language: C/C++ Source (.h, .hpp, .c, .cpp, .cxx, .inc)");
            ImGui::Text("Total lines: %zu", lines.size());
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Source Code Preview", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::BeginChild("CodePreview", ImVec2(0, 200), true);
                size_t previewCount = std::min<size_t>(lines.size(), 40);
                for (size_t i = 0; i < previewCount; ++i)
                {
                    ImGui::TextDisabled("%3zu: ", i + 1);
                    ImGui::SameLine();
                    ImGui::TextUnformatted(lines[i].c_str());
                }
                if (lines.size() > 40)
                {
                    ImGui::TextDisabled("... (%zu more lines)", lines.size() - 40);
                }
                ImGui::EndChild();
            }

#if defined(__linux__)
            if (ImGui::Button("Open in IDE / System Editor", ImVec2(220.0f, 30.0f)))
            {
                std::string cmd = "xdg-open \"" + p.string() + "\" &";
                std::system(cmd.c_str());
            }
#endif
        }
        else if (assetType == AssetType::TextStream)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "TextStream / Data Stream Inspector");
            ImGui::Spacing();

            std::vector<std::string> lines = RoninEngine::Runtime::TextStream::ReadLines(p.string());
            ImGui::Text("Total lines: %zu", lines.size());
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Text Content Preview", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::BeginChild("TextStreamPreview", ImVec2(0, 180), true);
                size_t previewCount = std::min<size_t>(lines.size(), 30);
                for (size_t i = 0; i < previewCount; ++i)
                {
                    ImGui::TextDisabled("%3zu: ", i + 1);
                    ImGui::SameLine();
                    ImGui::TextUnformatted(lines[i].c_str());
                }
                if (lines.size() > 30)
                {
                    ImGui::TextDisabled("... (%zu more lines)", lines.size() - 30);
                }
                ImGui::EndChild();
            }

#if defined(__linux__)
            if (ImGui::Button("Open in System Editor"))
            {
                std::string cmd = "xdg-open \"" + p.string() + "\" &";
                std::system(cmd.c_str());
            }
#endif
        }
        else if (assetType == AssetType::Prefab)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.8f, 1.0f), "Prefab Inspector");
            ImGui::Spacing();
            ImGui::TextWrapped("Prefab template: %s", stem.c_str());
            if (ImGui::Button("Instantiate Prefab in Scene", ImVec2(180.0f, 30.0f)))
            {
                EditorObject newObj;
                newObj.id = state.activeWorld().allocateObjectId();
                newObj.name = stem;
                newObj.position = {0.0f, 0.0f};
                state.activeWorld().objects.push_back(newObj);
                state.selectObject(newObj.id);
                state.isDirty = true;
                state.logInfo("Instantiated prefab: " + stem);
            }
        }
        else if (assetType == AssetType::Terrain)
        {
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.4f, 1.0f), "Terrain Asset Inspector");
            ImGui::Spacing();
            ImGui::TextWrapped("Reusable terrain data: %s", stem.c_str());
        }
    }

    void InspectorPanel::Render(EditorState &state)
    {
        if (ImGui::Begin("Inspector"))
        {
            if (state.inspectAssetMode && !state.selectedAssetPath.empty())
            {
                renderAssetInspector(state);
                ImGui::End();
                return;
            }

            EditorObject *obj = state.findObject(state.selectedObjectId);
            if (!obj)
            {
                if (!state.selectedAssetPath.empty())
                {
                    renderAssetInspector(state);
                }
                else
                {
                    ImGui::TextDisabled("No GameObject selected.");
                    ImGui::Separator();
                    ImGui::TextWrapped("Select a GameObject in the Hierarchy panel or an asset in the Project panel to inspect its properties.");
                }
                ImGui::End();
                return;
            }

            // 1. Top Header: Active, Name, Reset, Tag, Layer, Z-Order
            ImGui::Checkbox("##ObjActive", &obj->active);
            ImGui::SameLine();

            char nameBuf[128];
            strncpy(nameBuf, obj->name.c_str(), sizeof(nameBuf) - 1);
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 65.0f);
            if (ImGui::InputText("##ObjName", nameBuf, sizeof(nameBuf)))
            {
                obj->name = nameBuf;
                state.isDirty = true;
            }

            ImGui::SameLine();
            if (ImGui::SmallButton("Reset##Obj"))
            {
                obj->position = {0.0f, 0.0f};
                obj->angle = 0.0f;
                obj->scale = {1.0f, 1.0f};
                state.isDirty = true;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reset GameObject Transform to origin");

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 70.0f);

            ImGui::Text("Tag");
            ImGui::NextColumn();
            char tagBuf[64];
            strncpy(tagBuf, obj->tag.c_str(), sizeof(tagBuf) - 1);
            tagBuf[sizeof(tagBuf) - 1] = '\0';
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputText("##ObjTag", tagBuf, sizeof(tagBuf)))
            {
                obj->tag = tagBuf;
                state.isDirty = true;
            }

            ImGui::NextColumn();
            ImGui::Text("Layer");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            const char *layers[] = {"Default", "TransparentFX", "Ignore Raycast", "Water", "UI"};
            ImGui::Combo("##ObjLayer", &obj->layer, layers, 5);

            ImGui::NextColumn();
            ImGui::Text("Z-Order");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::DragInt("##ObjZOrder", &obj->zOrder, 1, -1000, 1000))
            {
                state.isDirty = true;
            }

            ImGui::Columns(1);
            ImGui::Separator();

            // 2. Components
            int currentObjId = obj->id;
            auto getObj = [&]() -> EditorObject * {
                if (state.selectedObjectId != currentObjId)
                    return nullptr;
                return state.findObject(currentObjId);
            };

            renderTransformComponent(state, *obj);
            if (!getObj()) { ImGui::End(); return; }
            renderSpriteRendererComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderCamera2DComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderLight2DComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderTerrain2DComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderParticleSystemComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderPhysicsComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderRigidbody2DComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderBoxCollider2DComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderAudioSourceComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }
            renderMotionScriptComponent(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }

            // 3. Add Component Button
            renderAddComponentMenu(state, *getObj());
            if (!getObj()) { ImGui::End(); return; }

            // 4. Modals
            renderSpritePickerModal(*getObj(), state);
            renderAudioPickerModal(*getObj(), state);
        }
        ImGui::End();
    }
} // namespace RoninEditor
