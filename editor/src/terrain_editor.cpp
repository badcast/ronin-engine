#include "terrain_editor.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <cmath>

namespace RoninEditor
{
    TerrainEditorPanel::TerrainEditorPanel() = default;

    void TerrainEditorPanel::Render(EditorState &state)
    {
        if (state.isTerrainEditorOpen)
        {
            m_isOpen = true;
        }

        if (!m_isOpen)
            return;

        ImGui::SetNextWindowSize(ImVec2(800.0f, 520.0f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Terrain Editor", &m_isOpen))
        {
            ImGui::End();
            return;
        }

        EditorObject *terrainObj = nullptr;
        renderTargetSelector(state, terrainObj);

        if (!terrainObj)
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.85f, 0.70f, 0.30f, 1.0f), "No active Terrain2D GameObject selected.");
            ImGui::TextDisabled("Select a GameObject with a Terrain2D component or create a new terrain below.");
            ImGui::Spacing();

            if (ImGui::Button("+ Create New Terrain 2D", ImVec2(220.0f, 32.0f)))
            {
                auto &newObj = state.createObject("Terrain 2D");
                newObj.terrain.enabled = true;
                newObj.terrain.width = 16;
                newObj.terrain.length = 8;
                newObj.terrain.cellScale = 1.0f;
                newObj.terrain.ensureGridInitialized(0);
                m_selectedObjId = newObj.id;
                state.selectedObjectId = newObj.id;
                state.logInfo("Created new Terrain2D GameObject: " + newObj.name);
            }
            ImGui::End();
            return;
        }

        // Two-column layout: Left column = Tools & Properties (320px), Right column = Interactive Canvas
        renderHeaderInfo(state, *terrainObj);
        ImGui::Separator();

        float leftColWidth = 320.0f;
        ImGui::BeginChild("TerrainToolsPanel", ImVec2(leftColWidth, 0.0f), true);
        renderToolbarAndTabs(state, *terrainObj);

        switch (m_activeTab)
        {
            case 0:
                renderBrushPanel(state, *terrainObj);
                break;
            case 1:
                renderFillPanel(state, *terrainObj);
                break;
            case 2:
                renderCollidersPanel(state, *terrainObj);
                break;
            case 3:
                renderProceduralPanel(state, *terrainObj);
                break;
            case 4:
                renderSettingsPanel(state, *terrainObj);
                break;
            default:
                break;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("TerrainCanvasPanel", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        renderInteractiveCanvas(state, *terrainObj);
        ImGui::EndChild();

        ImGui::End();
    }

    void TerrainEditorPanel::renderTargetSelector(EditorState &state, EditorObject *&outObj)
    {
        // 1. Gather all terrain objects in active world
        std::vector<EditorObject *> terrainObjects;
        for (auto &obj : state.activeWorld().objects)
        {
            if (obj.terrain.enabled)
            {
                terrainObjects.push_back(&obj);
            }
        }

        // 2. Synchronize selected terrain object
        EditorObject *curSel = state.findObject(state.selectedObjectId);
        if (curSel && curSel->terrain.enabled)
        {
            m_selectedObjId = curSel->id;
        }
        else if (m_selectedObjId <= 0 || !state.findObject(m_selectedObjId) || !state.findObject(m_selectedObjId)->terrain.enabled)
        {
            if (!terrainObjects.empty())
            {
                auto it = std::find_if(terrainObjects.begin(), terrainObjects.end(), [](const EditorObject *o) {
                    return o->name.find("Ground") != std::string::npos || o->name.find("Terrain") != std::string::npos;
                });
                m_selectedObjId = (it != terrainObjects.end()) ? (*it)->id : terrainObjects[0]->id;
            }
            else
            {
                m_selectedObjId = -1;
            }
        }

        outObj = state.findObject(m_selectedObjId);

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Target Terrain:");
        ImGui::SameLine();

        std::string currentName = outObj ? (outObj->name + " (ID: " + std::to_string(outObj->id) + ")") : "<None>";
        ImGui::SetNextItemWidth(260.0f);
        if (ImGui::BeginCombo("##TargetTerrainCombo", currentName.c_str()))
        {
            for (auto *to : terrainObjects)
            {
                bool isSelected = (outObj && to->id == outObj->id);
                std::string label = to->name + " (ID: " + std::to_string(to->id) + ")";
                if (ImGui::Selectable(label.c_str(), isSelected))
                {
                    m_selectedObjId = to->id;
                    state.selectedObjectId = to->id;
                    outObj = to;
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button("+ New Terrain"))
        {
            auto &newObj = state.createObject("Terrain 2D");
            newObj.terrain.enabled = true;
            newObj.terrain.width = 16;
            newObj.terrain.length = 8;
            newObj.terrain.cellScale = 1.0f;
            newObj.terrain.ensureGridInitialized(0);
            m_selectedObjId = newObj.id;
            state.selectedObjectId = newObj.id;
            outObj = &newObj;
            state.logInfo("Created new Terrain2D GameObject: " + newObj.name);
        }
    }

    void TerrainEditorPanel::renderHeaderInfo(EditorState &state, EditorObject &obj)
    {
        (void)state;
        obj.terrain.ensureGridInitialized(0);

        int collidersCount = 0;
        for (bool c : obj.terrain.cellColliders)
        {
            if (c)
                collidersCount++;
        }

        ImGui::TextDisabled("Grid: %dx%d (%d cells) | Scale: %.2f | World: %.1fx%.1f | Colliders: %d | Strokes: %zu",
                            obj.terrain.width,
                            obj.terrain.length,
                            obj.terrain.width * obj.terrain.length,
                            obj.terrain.cellScale,
                            obj.terrain.width * obj.terrain.cellScale,
                            obj.terrain.length * obj.terrain.cellScale,
                            collidersCount,
                            obj.terrain.paintStrokes.size());
    }

    void TerrainEditorPanel::renderToolbarAndTabs(EditorState &state, EditorObject &obj)
    {
        (void)obj;
        const char *tabs[] = {"Brush", "Fill", "Colliders", "Procedural", "Settings"};
        for (int i = 0; i < 5; ++i)
        {
            bool active = (m_activeTab == i);
            if (active)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.55f, 0.85f, 1.0f));

            if (ImGui::Button(tabs[i], ImVec2(58.0f, 26.0f)))
            {
                m_activeTab = i;
                if (i == 0)
                    state.terrainBrushTool = TerrainBrushTool::FreehandSurfaceBrush;
                else if (i == 1)
                    state.terrainBrushTool = TerrainBrushTool::BucketFill;
                else if (i == 2)
                    state.terrainBrushTool = TerrainBrushTool::PaintCollider;
            }

            if (active)
                ImGui::PopStyleColor();

            if (i < 4)
                ImGui::SameLine();
        }
        ImGui::Spacing();
    }

    void TerrainEditorPanel::renderBrushPanel(EditorState &state, EditorObject &obj)
    {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Surface Paint & Materials");
        ImGui::Separator();

        // Brush mode selection
        const char *brushModes[] = {"Freehand Surface", "Grid Cell Stamp", "Eraser"};
        int currentMode = (state.terrainBrushTool == TerrainBrushTool::Eraser)
                              ? 2
                              : ((state.terrainBrushTool == TerrainBrushTool::PaintTileMaterial) ? 1 : 0);

        ImGui::Text("Brush Mode:");
        for (int i = 0; i < 3; ++i)
        {
            if (ImGui::RadioButton(brushModes[i], currentMode == i))
            {
                if (i == 0)
                    state.terrainBrushTool = TerrainBrushTool::FreehandSurfaceBrush;
                else if (i == 1)
                    state.terrainBrushTool = TerrainBrushTool::PaintTileMaterial;
                else
                    state.terrainBrushTool = TerrainBrushTool::Eraser;
            }
            if (i < 2)
                ImGui::SameLine();
        }

        ImGui::Spacing();
        ImGui::Text("Material Palette:");

        // 10 Materials palette with colored swatch buttons
        struct MatInfo
        {
            int id;
            const char *name;
            ImVec4 color;
        };

        static const MatInfo materials[10] = {
            {0, "Grass", ImVec4(0.27f, 0.58f, 0.32f, 1.0f)},
            {1, "Dirt", ImVec4(0.55f, 0.35f, 0.17f, 1.0f)},
            {2, "Stone", ImVec4(0.48f, 0.51f, 0.53f, 1.0f)},
            {3, "Sand", ImVec4(0.83f, 0.71f, 0.42f, 1.0f)},
            {4, "Water", ImVec4(0.17f, 0.45f, 0.72f, 1.0f)},
            {5, "Cobble", ImVec4(0.36f, 0.39f, 0.43f, 1.0f)},
            {6, "Snow", ImVec4(0.91f, 0.94f, 0.97f, 1.0f)},
            {7, "Lava", ImVec4(0.88f, 0.25f, 0.13f, 1.0f)},
            {8, "Tatami", ImVec4(0.63f, 0.45f, 0.28f, 1.0f)},
            {9, "Custom", ImVec4(0.80f, 0.40f, 0.70f, 1.0f)}};

        for (int i = 0; i < 10; ++i)
        {
            bool isSelected = (state.terrainActiveMaterial == materials[i].id);
            ImGui::PushID(i);

            if (isSelected)
            {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            }

            ImGui::PushStyleColor(ImGuiCol_Button, materials[i].color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(materials[i].color.x * 1.15f, materials[i].color.y * 1.15f, materials[i].color.z * 1.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(materials[i].color.x * 0.85f, materials[i].color.y * 0.85f, materials[i].color.z * 0.85f, 1.0f));

            if (ImGui::Button(materials[i].name, ImVec2(56.0f, 26.0f)))
            {
                state.terrainActiveMaterial = materials[i].id;
                if (state.terrainBrushTool == TerrainBrushTool::Eraser)
                {
                    state.terrainBrushTool = TerrainBrushTool::FreehandSurfaceBrush;
                }
            }

            ImGui::PopStyleColor(3);

            if (isSelected)
            {
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }

            ImGui::PopID();

            if ((i + 1) % 5 != 0)
                ImGui::SameLine();
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Brush parameters
        ImGui::SliderFloat("Brush Radius", &state.terrainBrushRadius, 0.35f, 8.0f, "%.2f cells");
        ImGui::SliderFloat("Opacity", &state.terrainBrushOpacity, 0.05f, 1.0f, "%.2f");
        ImGui::SliderFloat("Hardness", &state.terrainBrushHardness, 0.0f, 1.0f, "%.2f");

        float brushCol[4] = {
            state.terrainBrushColor.r / 255.0f,
            state.terrainBrushColor.g / 255.0f,
            state.terrainBrushColor.b / 255.0f,
            state.terrainBrushColor.a / 255.0f};

        if (ImGui::ColorEdit4("Tint / Custom Color", brushCol))
        {
            state.terrainBrushColor.r = static_cast<uint8_t>(brushCol[0] * 255.0f);
            state.terrainBrushColor.g = static_cast<uint8_t>(brushCol[1] * 255.0f);
            state.terrainBrushColor.b = static_cast<uint8_t>(brushCol[2] * 255.0f);
            state.terrainBrushColor.a = static_cast<uint8_t>(brushCol[3] * 255.0f);
        }

        if (state.terrainActiveMaterial == 9)
        {
            ImGui::InputText("Texture", m_customTextureBuf, sizeof(m_customTextureBuf));
        }

        ImGui::Spacing();
        if (ImGui::Button("Clear Freehand Paint Strokes", ImVec2(-1.0f, 26.0f)))
        {
            obj.terrain.paintStrokes.clear();
            state.isDirty = true;
            state.logInfo("Cleared terrain paint strokes.");
        }
    }

    void TerrainEditorPanel::renderFillPanel(EditorState &state, EditorObject &obj)
    {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Bucket & Region Fill");
        ImGui::Separator();

        ImGui::Text("Fill Material: %s", EditorState::getTerrainMaterialName(state.terrainActiveMaterial));
        ImGui::Spacing();

        if (ImGui::Button("Fill All Grid With Active Material", ImVec2(-1.0f, 28.0f)))
        {
            auto col = (state.terrainActiveMaterial == 9)
                           ? state.terrainBrushColor
                           : EditorState::getTerrainMaterialBaseColor(state.terrainActiveMaterial, state.terrainBrushColor);
            state.fillTerrainAll(obj.terrain, state.terrainActiveMaterial, col);
            state.logInfo("Filled entire terrain with " + std::string(EditorState::getTerrainMaterialName(state.terrainActiveMaterial)));
        }

        ImGui::Spacing();
        if (ImGui::Button("Fill With Base Tile Color", ImVec2(-1.0f, 28.0f)))
        {
            state.fillTerrainAll(obj.terrain, 0, obj.terrain.tileColor);
            state.logInfo("Filled terrain with base tile color.");
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Tip: You can also use the interactive canvas on the right to click any cell and flood-fill contiguous regions.");
    }

    void TerrainEditorPanel::renderCollidersPanel(EditorState &state, EditorObject &obj)
    {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Colliders & Obstacles");
        ImGui::Separator();

        bool isPaint = (state.terrainBrushTool == TerrainBrushTool::PaintCollider);
        bool isErase = (state.terrainBrushTool == TerrainBrushTool::EraseCollider);

        if (isPaint)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.35f, 0.25f, 1.0f));
        if (ImGui::Button("Paint Collider (Block)", ImVec2(148.0f, 28.0f)))
        {
            state.terrainBrushTool = TerrainBrushTool::PaintCollider;
        }
        if (isPaint)
            ImGui::PopStyleColor();

        ImGui::SameLine();

        if (isErase)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.75f, 0.35f, 1.0f));
        if (ImGui::Button("Erase Collider", ImVec2(148.0f, 28.0f)))
        {
            state.terrainBrushTool = TerrainBrushTool::EraseCollider;
        }
        if (isErase)
            ImGui::PopStyleColor();

        ImGui::Spacing();
        if (ImGui::Checkbox("Generate Border Colliders", &obj.terrain.generateBorderColliders))
        {
            state.isDirty = true;
        }

        ImGui::Spacing();
        if (ImGui::Button("Apply Perimeter Wall Colliders", ImVec2(-1.0f, 26.0f)))
        {
            state.applyTerrainBorderColliders(obj.terrain, true);
            state.logInfo("Applied perimeter border colliders.");
        }

        if (ImGui::Button("Clear All Colliders", ImVec2(-1.0f, 26.0f)))
        {
            std::fill(obj.terrain.cellColliders.begin(), obj.terrain.cellColliders.end(), false);
            state.isDirty = true;
            state.logInfo("Cleared all terrain colliders.");
        }
    }

    void TerrainEditorPanel::renderProceduralPanel(EditorState &state, EditorObject &obj)
    {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Procedural Biome Generator");
        ImGui::Separator();

        const char *presets[] = {
            "Plains & Meadows (Lush Grass + Water Streams)",
            "Rocky Mountain Slopes (Stone, Dirt & Snow)",
            "Desert Dunes (Sand, Stone & Dust)",
            "Ocean Archipelago (Islands & Deep Water)",
            "Dungeon Ruins (Cobblestone, Stone & Colliders)",
            "Volcanic Caldera (Lava, Magma & Basalt)"};

        for (int i = 0; i < 6; ++i)
        {
            if (ImGui::Button(presets[i], ImVec2(-1.0f, 28.0f)))
            {
                state.generateProceduralTerrain(obj.terrain, i);
                state.logInfo("Generated procedural terrain preset: " + std::string(presets[i]));
            }
            ImGui::Spacing();
        }
    }

    void TerrainEditorPanel::renderSettingsPanel(EditorState &state, EditorObject &obj)
    {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Grid & Dimension Settings");
        ImGui::Separator();

        bool resized = false;
        if (ImGui::DragInt("Width (Cells)", &obj.terrain.width, 1, 2, 128))
            resized = true;
        if (ImGui::DragInt("Length (Cells)", &obj.terrain.length, 1, 2, 128))
            resized = true;
        if (ImGui::DragFloat("Cell Scale", &obj.terrain.cellScale, 0.05f, 0.1f, 10.0f, "%.2f units"))
            state.isDirty = true;

        if (resized)
        {
            obj.terrain.ensureGridInitialized(0);
            state.isDirty = true;
        }

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
    }

    void TerrainEditorPanel::renderInteractiveCanvas(EditorState &state, EditorObject &obj)
    {
        obj.terrain.ensureGridInitialized(0);
        int w = obj.terrain.width;
        int l = obj.terrain.length;

        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasAvail = ImGui::GetContentRegionAvail();
        if (canvasAvail.x < 50.0f || canvasAvail.y < 50.0f)
            return;

        ImDrawList *drawList = ImGui::GetWindowDrawList();

        // 1. Draw Canvas background slate
        drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasAvail.x, canvasPos.y + canvasAvail.y), IM_COL32(20, 22, 28, 255));
        drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasAvail.x, canvasPos.y + canvasAvail.y), IM_COL32(45, 48, 58, 255));

        // 2. Compute terrain grid dimensions on screen
        float maxGridWidth = canvasAvail.x - 40.0f;
        float maxGridHeight = canvasAvail.y - 40.0f;
        float baseCellSize = std::min(maxGridWidth / static_cast<float>(w), maxGridHeight / static_cast<float>(l));
        baseCellSize = std::max(4.0f, baseCellSize);

        float cellSize = baseCellSize * m_canvasZoom;
        float gridPixelW = cellSize * w;
        float gridPixelH = cellSize * l;

        ImVec2 gridOrigin(
            canvasPos.x + (canvasAvail.x - gridPixelW) * 0.5f + m_canvasPan.x,
            canvasPos.y + (canvasAvail.y - gridPixelH) * 0.5f + m_canvasPan.y);

        ImGuiIO &io = ImGui::GetIO();
        bool isHovered = ImGui::IsWindowHovered();

        // 3. Pan & Zoom Canvas
        if (isHovered)
        {
            if (io.MouseWheel != 0.0f)
            {
                m_canvasZoom = std::clamp(m_canvasZoom + io.MouseWheel * 0.15f, 0.4f, 5.0f);
            }

            if (io.MouseDown[1] || io.MouseDown[2])
            {
                if (!m_isPanning)
                {
                    m_isPanning = true;
                    m_panStartMouse = {io.MousePos.x, io.MousePos.y};
                    m_panStartOffset = m_canvasPan;
                }
                else
                {
                    m_canvasPan.x = m_panStartOffset.x + (io.MousePos.x - m_panStartMouse.x);
                    m_canvasPan.y = m_panStartOffset.y + (io.MousePos.y - m_panStartMouse.y);
                }
            }
            else
            {
                m_isPanning = false;
            }
        }

        // 4. Render Grid Cells
        for (int r = 0; r < l; ++r)
        {
            for (int c = 0; c < w; ++c)
            {
                size_t idx = static_cast<size_t>(r * w + c);
                auto col = (idx < obj.terrain.cellColors.size()) ? obj.terrain.cellColors[idx] : obj.terrain.tileColor;

                ImVec2 p0(gridOrigin.x + c * cellSize, gridOrigin.y + r * cellSize);
                ImVec2 p1(p0.x + cellSize, p0.y + cellSize);

                drawList->AddRectFilled(p0, p1, IM_COL32(col.r, col.g, col.b, 255));
                drawList->AddRect(p0, p1, IM_COL32(35, 38, 48, 120));

                // Draw collider overlay
                if (idx < obj.terrain.cellColliders.size() && obj.terrain.cellColliders[idx])
                {
                    drawList->AddRectFilled(p0, p1, IM_COL32(230, 60, 40, 110));
                    drawList->AddLine(p0, p1, IM_COL32(240, 100, 80, 200));
                    drawList->AddLine(ImVec2(p0.x, p1.y), ImVec2(p1.x, p0.y), IM_COL32(240, 100, 80, 200));
                }
            }
        }

        // 5. Render Freehand Paint Strokes
        for (const auto &st : obj.terrain.paintStrokes)
        {
            ImVec2 strokePos(gridOrigin.x + st.uvPos.x * gridPixelW, gridOrigin.y + st.uvPos.y * gridPixelH);
            float radPx = std::max(2.0f, st.radiusNormalized * gridPixelW);
            uint8_t a = static_cast<uint8_t>(std::clamp(st.color.a * st.opacity, 10.0f, 255.0f));
            drawList->AddCircleFilled(strokePos, radPx, IM_COL32(st.color.r, st.color.g, st.color.b, a));
        }

        // 6. Grid Outer Border
        drawList->AddRect(gridOrigin, ImVec2(gridOrigin.x + gridPixelW, gridOrigin.y + gridPixelH), IM_COL32(80, 140, 220, 255), 0.0f, 0, 2.0f);

        // 7. Interactive Mouse Painting on Canvas
        ImVec2 mousePos = io.MousePos;
        bool mouseInsideGrid = (mousePos.x >= gridOrigin.x && mousePos.x <= gridOrigin.x + gridPixelW &&
                                mousePos.y >= gridOrigin.y && mousePos.y <= gridOrigin.y + gridPixelH);

        if (mouseInsideGrid && isHovered && !m_isPanning)
        {
            RoninEngine::Runtime::Vec2 uv(
                (mousePos.x - gridOrigin.x) / gridPixelW,
                (mousePos.y - gridOrigin.y) / gridPixelH);

            // Draw brush preview ring
            float brushRingRadius = std::max(4.0f, (state.terrainBrushRadius / static_cast<float>(std::min(w, l))) * gridPixelW);
            drawList->AddCircle(mousePos, brushRingRadius, IM_COL32(255, 230, 80, 220), 24, 2.0f);

            // Left mouse paint
            if (io.MouseDown[0])
            {
                bool isDrag = (io.MouseDownDuration[0] > 0.0f);
                if (state.paintTerrainAtUV(obj.terrain, uv, isDrag))
                {
                    state.isDirty = true;
                }
            }
        }

        // 8. Overlay Canvas Controls (Reset View, Zoom)
        ImGui::SetCursorScreenPos(ImVec2(canvasPos.x + 10.0f, canvasPos.y + 10.0f));
        if (ImGui::Button("Reset View##TerrainCanvas"))
        {
            m_canvasZoom = 1.0f;
            m_canvasPan = {0.0f, 0.0f};
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Zoom: %.1fx | Pan: Right-Click Drag | Paint: Left-Click", m_canvasZoom);
    }
} // namespace RoninEditor
