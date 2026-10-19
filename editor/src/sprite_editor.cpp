#include "sprite_editor.h"
#include "project_manager.h"
#include "meta_manager.h"
#include "asset_database.h"
#include <imgui.h>
#include <SDL2/SDL_image.h>
#include <SimpleJSON/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace RoninEditor
{
    SpriteEditor::SpriteEditor() = default;

    SpriteEditor::~SpriteEditor()
    {
        freeTexture();
    }

    void SpriteEditor::Init(SDL_Renderer *renderer)
    {
        m_renderer = renderer;
    }

    void SpriteEditor::Shutdown()
    {
        freeTexture();
        m_renderer = nullptr;
    }

    void SpriteEditor::freeTexture()
    {
        if (m_texture)
        {
            SDL_DestroyTexture(m_texture);
            m_texture = nullptr;
        }
        if (m_surface)
        {
            SDL_FreeSurface(m_surface);
            m_surface = nullptr;
        }
        m_texWidth = 0;
        m_texHeight = 0;
    }

    void SpriteEditor::loadTexture(const std::filesystem::path &imagePath)
    {
        freeTexture();

        std::filesystem::path fullPath = imagePath;
        if (!std::filesystem::exists(fullPath))
        {
            auto root = ProjectManager::GetInstance().GetRootPath();
            fullPath = root / imagePath;
        }

        if (!std::filesystem::exists(fullPath))
            return;

        m_surface = IMG_Load(fullPath.string().c_str());
        if (!m_surface)
            return;

        m_texWidth = m_surface->w;
        m_texHeight = m_surface->h;

        if (m_renderer)
        {
            m_texture = SDL_CreateTextureFromSurface(m_renderer, m_surface);
        }

        // Store relative path if inside project
        auto root = ProjectManager::GetInstance().GetRootPath();
        std::error_code ec;
        auto rel = std::filesystem::relative(fullPath, root, ec);
        m_meta.texturePath = (ec || rel.empty()) ? fullPath.string() : rel.string();

        if (m_meta.rect.w <= 0 || m_meta.rect.h <= 0 || m_meta.rect.w > m_texWidth || m_meta.rect.h > m_texHeight)
        {
            m_meta.rect = {0, 0, m_texWidth, m_texHeight};
        }
    }

    bool SpriteEditor::loadSpriteJson(const std::filesystem::path &jsonPath)
    {
        std::ifstream ifs(jsonPath);
        if (!ifs.is_open()) return false;

        std::stringstream ss;
        ss << ifs.rdbuf();
        json::JSON j = json::JSON::Load(ss.str());
        if (j.IsNull()) return false;

        if (j.hasKey("name")) m_meta.name = j["name"].ToString();
        else m_meta.name = jsonPath.stem().string();

        bool loadedTex = false;
        if (j.hasKey("textureGuid"))
        {
            std::filesystem::path resolved = AssetDatabase::GetInstance().GetPathByGuid(j["textureGuid"].ToString());
            if (!resolved.empty() && std::filesystem::exists(resolved))
            {
                m_meta.texturePath = resolved.string();
                loadTexture(m_meta.texturePath);
                loadedTex = true;
            }
        }
        if (!loadedTex && j.hasKey("texturePath"))
        {
            m_meta.texturePath = j["texturePath"].ToString();
            loadTexture(m_meta.texturePath);
        }

        if (j.hasKey("rect") && j["rect"].JSONType() == json::JSON::Class::Object)
        {
            auto &jr = j["rect"];
            m_meta.rect.x = jr.hasKey("x") ? jr["x"].ToInt() : 0;
            m_meta.rect.y = jr.hasKey("y") ? jr["y"].ToInt() : 0;
            m_meta.rect.w = jr.hasKey("w") ? jr["w"].ToInt() : m_texWidth;
            m_meta.rect.h = jr.hasKey("h") ? jr["h"].ToInt() : m_texHeight;
        }

        if (j.hasKey("pivot") && j["pivot"].JSONType() == json::JSON::Class::Object)
        {
            auto &jp = j["pivot"];
            m_meta.pivot.x = jp.hasKey("x") ? static_cast<float>(jp["x"].ToFloat()) : 0.5f;
            m_meta.pivot.y = jp.hasKey("y") ? static_cast<float>(jp["y"].ToFloat()) : 0.5f;
        }

        if (j.hasKey("pixelsPerUnit"))
            m_meta.pixelsPerUnit = static_cast<float>(j["pixelsPerUnit"].ToFloat());

        if (j.hasKey("border") && j["border"].JSONType() == json::JSON::Class::Object)
        {
            auto &jb = j["border"];
            m_meta.borderLeft = jb.hasKey("l") ? jb["l"].ToInt() : 0;
            m_meta.borderBottom = jb.hasKey("b") ? jb["b"].ToInt() : 0;
            m_meta.borderRight = jb.hasKey("r") ? jb["r"].ToInt() : 0;
            m_meta.borderTop = jb.hasKey("t") ? jb["t"].ToInt() : 0;
        }

        m_currentFile = jsonPath;
        m_isDirty = false;
        return true;
    }

    bool SpriteEditor::saveSpriteJson(const std::filesystem::path &jsonPath)
    {
        json::JSON j = json::JSON::Make(json::JSON::Class::Object);
        j["formatVersion"] = 1;
        j["name"] = m_meta.name;
        j["texturePath"] = m_meta.texturePath;

        std::string texGuid = AssetDatabase::GetInstance().GetGuidByPath(m_meta.texturePath);
        if (!texGuid.empty())
        {
            j["textureGuid"] = texGuid;
        }

        json::JSON jr = json::JSON::Make(json::JSON::Class::Object);
        jr["x"] = m_meta.rect.x;
        jr["y"] = m_meta.rect.y;
        jr["w"] = m_meta.rect.w;
        jr["h"] = m_meta.rect.h;
        j["rect"] = jr;

        json::JSON jp = json::JSON::Make(json::JSON::Class::Object);
        jp["x"] = static_cast<double>(m_meta.pivot.x);
        jp["y"] = static_cast<double>(m_meta.pivot.y);
        j["pivot"] = jp;

        j["pixelsPerUnit"] = static_cast<double>(m_meta.pixelsPerUnit);

        json::JSON jb = json::JSON::Make(json::JSON::Class::Object);
        jb["l"] = m_meta.borderLeft;
        jb["b"] = m_meta.borderBottom;
        jb["r"] = m_meta.borderRight;
        jb["t"] = m_meta.borderTop;
        j["border"] = jb;

        std::ofstream ofs(jsonPath);
        if (!ofs.is_open()) return false;
        ofs << j.dump(1, "  ");
        ofs.close();

        // Ensure companion .meta file is generated
        MetaFile mf;
        MetaManager::EnsureMetaFile(jsonPath, mf);
        AssetDatabase::GetInstance().Refresh();

        m_currentFile = jsonPath;
        m_isDirty = false;
        return true;
    }

    void SpriteEditor::OpenAsset(const std::filesystem::path &filePath)
    {
        m_isOpen = true;
        std::string ext = filePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".sprite")
        {
            loadSpriteJson(filePath);
        }
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
        {
            m_currentFile = filePath;
            m_meta.name = filePath.stem().string();
            loadTexture(filePath);
            m_meta.rect = {0, 0, m_texWidth, m_texHeight};
            m_meta.pivot = {0.5f, 0.5f};
            m_meta.pixelsPerUnit = 100.0f;
            m_isDirty = true;
        }
    }

    void SpriteEditor::autoTrim()
    {
        if (!m_surface) return;

        SDL_LockSurface(m_surface);
        int bpp = m_surface->format->BytesPerPixel;
        int minX = m_surface->w, minY = m_surface->h, maxX = -1, maxY = -1;

        for (int y = 0; y < m_surface->h; ++y)
        {
            Uint8 *row = static_cast<Uint8 *>(m_surface->pixels) + y * m_surface->pitch;
            for (int x = 0; x < m_surface->w; ++x)
            {
                Uint8 a = 255;
                if (bpp == 4)
                {
                    Uint32 pixel = *reinterpret_cast<Uint32 *>(row + x * 4);
                    if (m_surface->format->Amask != 0)
                        a = static_cast<Uint8>((pixel & m_surface->format->Amask) >> m_surface->format->Ashift);
                }
                else if (bpp == 1 && m_surface->format->palette)
                {
                    Uint32 pixel = *(row + x);
                    a = m_surface->format->palette->colors[pixel].a;
                }

                if (a > 10)
                {
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }
        SDL_UnlockSurface(m_surface);

        if (maxX >= minX && maxY >= minY)
        {
            m_meta.rect = {minX, minY, maxX - minX + 1, maxY - minY + 1};
            m_isDirty = true;
        }
    }

    void SpriteEditor::autoSliceGrid(int cellW, int cellH, EditorState &state)
    {
        if (!m_surface || cellW <= 0 || cellH <= 0) return;

        auto root = ProjectManager::GetInstance().GetRootPath();
        std::filesystem::path outDir = m_currentFile.has_parent_path() ? m_currentFile.parent_path() : (root / "Assets" / "Textures");

        int cols = m_surface->w / cellW;
        int rows = m_surface->h / cellH;
        int createdCount = 0;

        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                int index = r * cols + c;
                std::string spriteName = m_meta.name + "_" + std::to_string(index);
                std::filesystem::path outFile = outDir / (spriteName + ".sprite");

                SpriteMetadata sm;
                sm.name = spriteName;
                sm.texturePath = m_meta.texturePath;
                sm.rect = {c * cellW, r * cellH, cellW, cellH};
                sm.pivot = m_meta.pivot;
                sm.pixelsPerUnit = m_meta.pixelsPerUnit;

                json::JSON j = json::JSON::Make(json::JSON::Class::Object);
                j["formatVersion"] = 1;
                j["name"] = sm.name;
                j["texturePath"] = sm.texturePath;

                json::JSON jr = json::JSON::Make(json::JSON::Class::Object);
                jr["x"] = sm.rect.x;
                jr["y"] = sm.rect.y;
                jr["w"] = sm.rect.w;
                jr["h"] = sm.rect.h;
                j["rect"] = jr;

                json::JSON jp = json::JSON::Make(json::JSON::Class::Object);
                jp["x"] = static_cast<double>(sm.pivot.x);
                jp["y"] = static_cast<double>(sm.pivot.y);
                j["pivot"] = jp;

                std::string texGuid = AssetDatabase::GetInstance().GetGuidByPath(sm.texturePath);
                if (!texGuid.empty())
                {
                    j["textureGuid"] = texGuid;
                }

                std::ofstream ofs(outFile);
                if (ofs.is_open())
                {
                    ofs << j.dump(1, "  ");
                    ofs.close();
                    MetaFile mf;
                    MetaManager::EnsureMetaFile(outFile, mf);
                    createdCount++;
                }
            }
        }

        AssetDatabase::GetInstance().Refresh();
        state.logInfo("Grid Sliced " + std::to_string(createdCount) + " sprite assets into " + outDir.string());
    }

    void SpriteEditor::renderCanvas(EditorState &state)
    {
        (void)state;
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        if (canvasSize.x < 100.0f || canvasSize.y < 100.0f)
            return;

        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImDrawList *drawList = ImGui::GetWindowDrawList();

        // Canvas background
        drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(24, 26, 32, 255));

        // Invisible button to capture input
        ImGui::InvisibleButton("SpriteCanvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
        bool isHovered = ImGui::IsItemHovered();
        ImGuiIO &io = ImGui::GetIO();

        // Pan with Middle Mouse or Right Mouse Drag
        if (isHovered && (io.MouseDown[2] || io.MouseDown[1]))
        {
            m_pan.x += io.MouseDelta.x;
            m_pan.y += io.MouseDelta.y;
        }

        // Zoom with Mouse Wheel
        if (isHovered && io.MouseWheel != 0.0f)
        {
            float zoomDelta = (io.MouseWheel > 0.0f) ? 1.2f : 0.8333f;
            float newZoom = std::clamp(m_zoom * zoomDelta, 0.1f, 32.0f);

            // Zoom centered on mouse
            ImVec2 mouse = io.MousePos;
            m_pan.x = mouse.x - (mouse.x - (canvasPos.x + canvasSize.x * 0.5f + m_pan.x)) * (newZoom / m_zoom) - (canvasPos.x + canvasSize.x * 0.5f);
            m_pan.y = mouse.y - (mouse.y - (canvasPos.y + canvasSize.y * 0.5f + m_pan.y)) * (newZoom / m_zoom) - (canvasPos.y + canvasSize.y * 0.5f);
            m_zoom = newZoom;
        }

        // Texture center position in screen coordinates
        ImVec2 center(canvasPos.x + canvasSize.x * 0.5f + m_pan.x, canvasPos.y + canvasSize.y * 0.5f + m_pan.y);

        if (m_texture && m_texWidth > 0 && m_texHeight > 0)
        {
            float drawW = m_texWidth * m_zoom;
            float drawH = m_texHeight * m_zoom;
            ImVec2 imgMin(center.x - drawW * 0.5f, center.y - drawH * 0.5f);
            ImVec2 imgMax(imgMin.x + drawW, imgMin.y + drawH);

            // Draw checkerboard behind texture
            const float checkSz = 16.0f * std::clamp(m_zoom, 0.5f, 2.0f);
            for (float cy = imgMin.y; cy < imgMax.y; cy += checkSz)
            {
                for (float cx = imgMin.x; cx < imgMax.x; cx += checkSz)
                {
                    int parity = static_cast<int>((cx - imgMin.x) / checkSz) + static_cast<int>((cy - imgMin.y) / checkSz);
                    ImU32 col = (parity % 2 == 0) ? IM_COL32(40, 44, 52, 255) : IM_COL32(32, 35, 42, 255);
                    ImVec2 cMax(std::min(cx + checkSz, imgMax.x), std::min(cy + checkSz, imgMax.y));
                    drawList->AddRectFilled(ImVec2(cx, cy), cMax, col);
                }
            }

            // Draw texture
            drawList->AddImage(reinterpret_cast<ImTextureID>(m_texture), imgMin, imgMax);
            drawList->AddRect(imgMin, imgMax, IM_COL32(80, 85, 100, 255));

            // Sprite Bounding Box in screen coordinates
            ImVec2 boxMin(imgMin.x + m_meta.rect.x * m_zoom, imgMin.y + m_meta.rect.y * m_zoom);
            ImVec2 boxMax(boxMin.x + m_meta.rect.w * m_zoom, boxMin.y + m_meta.rect.h * m_zoom);

            // Semi-transparent overlay outside bounding box
            drawList->AddRectFilled(imgMin, ImVec2(imgMax.x, boxMin.y), IM_COL32(0, 0, 0, 110));
            drawList->AddRectFilled(ImVec2(imgMin.x, boxMax.y), imgMax, IM_COL32(0, 0, 0, 110));
            drawList->AddRectFilled(ImVec2(imgMin.x, boxMin.y), ImVec2(boxMin.x, boxMax.y), IM_COL32(0, 0, 0, 110));
            drawList->AddRectFilled(ImVec2(boxMax.x, boxMin.y), ImVec2(imgMax.x, boxMax.y), IM_COL32(0, 0, 0, 110));

            // Green outline for sprite rect
            drawList->AddRect(boxMin, boxMax, IM_COL32(50, 220, 120, 255), 0.0f, 0, 2.0f);

            // Corner handles
            const float handleR = 5.0f;
            drawList->AddCircleFilled(boxMin, handleR, IM_COL32(255, 255, 255, 255));
            drawList->AddCircleFilled(ImVec2(boxMax.x, boxMin.y), handleR, IM_COL32(255, 255, 255, 255));
            drawList->AddCircleFilled(boxMax, handleR, IM_COL32(255, 255, 255, 255));
            drawList->AddCircleFilled(ImVec2(boxMin.x, boxMax.y), handleR, IM_COL32(255, 255, 255, 255));

            // Pivot Handle (Blue crosshair & circle)
            ImVec2 pivotPos(boxMin.x + m_meta.rect.w * m_meta.pivot.x * m_zoom,
                            boxMin.y + m_meta.rect.h * (1.0f - m_meta.pivot.y) * m_zoom);
            drawList->AddCircleFilled(pivotPos, 6.0f, IM_COL32(40, 160, 255, 220));
            drawList->AddCircle(pivotPos, 6.0f, IM_COL32(255, 255, 255, 255), 0, 1.5f);
            drawList->AddLine(ImVec2(pivotPos.x - 10.0f, pivotPos.y), ImVec2(pivotPos.x + 10.0f, pivotPos.y), IM_COL32(40, 160, 255, 255), 2.0f);
            drawList->AddLine(ImVec2(pivotPos.x, pivotPos.y - 10.0f), ImVec2(pivotPos.x, pivotPos.y + 10.0f), IM_COL32(40, 160, 255, 255), 2.0f);

            // Dragging interactions
            ImVec2 m = io.MousePos;
            if (isHovered && io.MouseClicked[0])
            {
                // Check pivot handle first
                if (std::hypot(m.x - pivotPos.x, m.y - pivotPos.y) <= 8.0f)
                {
                    m_dragMode = 6;
                }
                else if (std::hypot(m.x - boxMin.x, m.y - boxMin.y) <= 8.0f)
                {
                    m_dragMode = 2; // Top-Left
                }
                else if (std::hypot(m.x - boxMax.x, m.y - boxMin.y) <= 8.0f)
                {
                    m_dragMode = 3; // Top-Right
                }
                else if (std::hypot(m.x - boxMax.x, m.y - boxMax.y) <= 8.0f)
                {
                    m_dragMode = 4; // Bottom-Right
                }
                else if (std::hypot(m.x - boxMin.x, m.y - boxMax.y) <= 8.0f)
                {
                    m_dragMode = 5; // Bottom-Left
                }
                else if (m.x >= boxMin.x && m.x <= boxMax.x && m.y >= boxMin.y && m.y <= boxMax.y)
                {
                    m_dragMode = 1; // Move Rect Body
                }
                else
                {
                    m_dragMode = 0;
                }
            }

            if (io.MouseReleased[0])
            {
                m_dragMode = 0;
            }

            if (m_dragMode > 0 && io.MouseDown[0])
            {
                m_isDirty = true;
                float dPixelX = io.MouseDelta.x / m_zoom;
                float dPixelY = io.MouseDelta.y / m_zoom;

                if (m_dragMode == 1) // Move Rect
                {
                    m_meta.rect.x = std::clamp(static_cast<int>(std::round(m_meta.rect.x + dPixelX)), 0, std::max(0, m_texWidth - m_meta.rect.w));
                    m_meta.rect.y = std::clamp(static_cast<int>(std::round(m_meta.rect.y + dPixelY)), 0, std::max(0, m_texHeight - m_meta.rect.h));
                }
                else if (m_dragMode == 2) // Resize Top-Left
                {
                    int newX = std::clamp(static_cast<int>(std::round(m_meta.rect.x + dPixelX)), 0, std::max(0, m_meta.rect.x + m_meta.rect.w - 1));
                    int newY = std::clamp(static_cast<int>(std::round(m_meta.rect.y + dPixelY)), 0, std::max(0, m_meta.rect.y + m_meta.rect.h - 1));
                    m_meta.rect.w -= (newX - m_meta.rect.x);
                    m_meta.rect.h -= (newY - m_meta.rect.y);
                    m_meta.rect.x = newX;
                    m_meta.rect.y = newY;
                }
                else if (m_dragMode == 4) // Resize Bottom-Right
                {
                    m_meta.rect.w = std::clamp(static_cast<int>(std::round(m_meta.rect.w + dPixelX)), 1, std::max(1, m_texWidth - m_meta.rect.x));
                    m_meta.rect.h = std::clamp(static_cast<int>(std::round(m_meta.rect.h + dPixelY)), 1, std::max(1, m_texHeight - m_meta.rect.y));
                }
                else if (m_dragMode == 6) // Drag Pivot
                {
                    float localPx = (m.x - boxMin.x) / m_zoom;
                    float localPy = (m.y - boxMin.y) / m_zoom;
                    m_meta.pivot.x = std::clamp(localPx / m_meta.rect.w, 0.0f, 1.0f);
                    m_meta.pivot.y = std::clamp(1.0f - (localPy / m_meta.rect.h), 0.0f, 1.0f);
                    m_pivotPreset = 9; // Custom
                }
            }
        }
        else
        {
            ImVec2 textPos(center.x - 120.0f, center.y - 10.0f);
            drawList->AddText(textPos, IM_COL32(160, 160, 160, 255), "No Texture Loaded. Click [Select Texture...]");
        }

        // Overlay toolbar in canvas
        ImGui::SetCursorScreenPos(ImVec2(canvasPos.x + 10.0f, canvasPos.y + 10.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.14f, 0.18f, 0.85f));
        ImGui::BeginChild("CanvasOverlayControls", ImVec2(320.0f, 36.0f), true, ImGuiWindowFlags_NoScrollbar);
        if (ImGui::Button("Reset View"))
        {
            m_pan = {0.0f, 0.0f};
            m_zoom = 1.0f;
        }
        ImGui::SameLine();
        ImGui::Text("Zoom: %.0f%%", m_zoom * 100.0f);
        ImGui::SameLine();
        if (m_texWidth > 0)
        {
            ImGui::TextDisabled("| %dx%d", m_texWidth, m_texHeight);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void SpriteEditor::renderSidebar(EditorState &state)
    {
        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.85f, 1.0f), "[Sprite Editor Settings]");
        ImGui::Separator();

        // 1. File & Name
        char nameBuf[128];
        std::snprintf(nameBuf, sizeof(nameBuf), "%s", m_meta.name.c_str());
        if (ImGui::InputText("Sprite Name", nameBuf, sizeof(nameBuf)))
        {
            m_meta.name = nameBuf;
            m_isDirty = true;
        }

        ImGui::Text("Texture Source:");
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%s", m_meta.texturePath.empty() ? "(None)" : m_meta.texturePath.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("Select...##PickTexture"))
        {
            m_showTexturePicker = true;
            m_openTexturePickerTrigger = true;
            m_pendingTexturePath = m_meta.texturePath;
            m_pickerFilter[0] = '\0';
        }

        ImGui::Spacing();
        ImGui::Separator();

        // 2. Rect Dimensions
        ImGui::Text("Sprite Rect (Pixels):");
        int rectPos[2] = {m_meta.rect.x, m_meta.rect.y};
        if (ImGui::DragInt2("Position (X, Y)", rectPos, 1.0f, 0, std::max(1, m_texWidth)))
        {
            m_meta.rect.x = std::max(0, rectPos[0]);
            m_meta.rect.y = std::max(0, rectPos[1]);
            m_isDirty = true;
        }

        int rectSz[2] = {m_meta.rect.w, m_meta.rect.h};
        if (ImGui::DragInt2("Size (W, H)", rectSz, 1.0f, 1, std::max(1, m_texWidth)))
        {
            m_meta.rect.w = std::max(1, rectSz[0]);
            m_meta.rect.h = std::max(1, rectSz[1]);
            m_isDirty = true;
        }

        // 3. Pivot Setting
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Pivot Point:");
        const char *pivotNames[] = {
            "Center (0.5, 0.5)",
            "Top-Left (0.0, 1.0)",
            "Top (0.5, 1.0)",
            "Top-Right (1.0, 1.0)",
            "Left (0.0, 0.5)",
            "Right (1.0, 0.5)",
            "Bottom-Left (0.0, 0.0)",
            "Bottom (0.5, 0.0)",
            "Bottom-Right (1.0, 0.0)",
            "Custom"
        };
        if (ImGui::Combo("Pivot Preset", &m_pivotPreset, pivotNames, 10))
        {
            m_isDirty = true;
            switch (m_pivotPreset)
            {
                case 0: m_meta.pivot = {0.5f, 0.5f}; break;
                case 1: m_meta.pivot = {0.0f, 1.0f}; break;
                case 2: m_meta.pivot = {0.5f, 1.0f}; break;
                case 3: m_meta.pivot = {1.0f, 1.0f}; break;
                case 4: m_meta.pivot = {0.0f, 0.5f}; break;
                case 5: m_meta.pivot = {1.0f, 0.5f}; break;
                case 6: m_meta.pivot = {0.0f, 0.0f}; break;
                case 7: m_meta.pivot = {0.5f, 0.0f}; break;
                case 8: m_meta.pivot = {1.0f, 0.0f}; break;
                default: break;
            }
        }

        float pivotFloats[2] = {m_meta.pivot.x, m_meta.pivot.y};
        if (ImGui::SliderFloat2("Custom Pivot", pivotFloats, 0.0f, 1.0f, "%.2f"))
        {
            m_meta.pivot.x = pivotFloats[0];
            m_meta.pivot.y = pivotFloats[1];
            m_pivotPreset = 9;
            m_isDirty = true;
        }

        // 4. Pixels Per Unit
        ImGui::DragFloat("Pixels Per Unit", &m_meta.pixelsPerUnit, 1.0f, 1.0f, 1000.0f, "%.1f");

        // 5. 9-Slice Borders
        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Border (9-Slice)", ImGuiTreeNodeFlags_None))
        {
            ImGui::DragInt("Left Border", &m_meta.borderLeft, 1, 0, m_meta.rect.w / 2);
            ImGui::DragInt("Right Border", &m_meta.borderRight, 1, 0, m_meta.rect.w / 2);
            ImGui::DragInt("Top Border", &m_meta.borderTop, 1, 0, m_meta.rect.h / 2);
            ImGui::DragInt("Bottom Border", &m_meta.borderBottom, 1, 0, m_meta.rect.h / 2);
        }

        // 6. Slicing Tools
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Automatic Slicing Tools:");

        if (ImGui::Button("Auto-Trim to Alpha Bounds"))
        {
            autoTrim();
        }

        ImGui::Spacing();
        ImGui::Text("Grid Slice:");
        ImGui::DragInt("Cell Width", &m_gridSliceW, 1, 4, 512);
        ImGui::DragInt("Cell Height", &m_gridSliceH, 1, 4, 512);

        if (ImGui::Button("Slice Spritesheet"))
        {
            autoSliceGrid(m_gridSliceW, m_gridSliceH, state);
        }

        // 7. Actions: Save, Revert, New
        ImGui::Spacing();
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.35f, 1.0f));
        if (ImGui::Button("Save Sprite", ImVec2(130.0f, 30.0f)))
        {
            std::filesystem::path savePath = m_currentFile;
            if (savePath.extension() != ".sprite")
            {
                auto root = ProjectManager::GetInstance().GetRootPath();
                std::string spriteName = m_meta.name.empty() ? "NewSprite" : m_meta.name;
                savePath = (root / "Assets" / "Textures" / (spriteName + ".sprite"));
            }
            if (saveSpriteJson(savePath))
            {
                state.logInfo("Saved sprite: " + savePath.filename().string());
            }
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Revert", ImVec2(80.0f, 30.0f)))
        {
            if (m_currentFile.extension() == ".sprite")
            {
                loadSpriteJson(m_currentFile);
            }
            else if (!m_meta.texturePath.empty())
            {
                loadTexture(m_meta.texturePath);
            }
        }
    }

    void SpriteEditor::renderTexturePickerModal(EditorState &state)
    {
        (void)state;
        if (m_openTexturePickerTrigger)
        {
            ImGui::OpenPopup("Select Texture");
            m_openTexturePickerTrigger = false;
        }

        if (!m_showTexturePicker)
            return;

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500.0f, 440.0f));

        if (ImGui::BeginPopupModal("Select Texture", &m_showTexturePicker, ImGuiWindowFlags_None))
        {
            if (ImGui::IsWindowAppearing())
            {
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::InputTextWithHint("##TextureSearch", "Filter textures...", m_pickerFilter, sizeof(m_pickerFilter));
            ImGui::Separator();

            ImGui::BeginChild("TextureListScroll", ImVec2(0, 270.0f), true);

            bool applySelection = false;

            auto root = ProjectManager::GetInstance().GetRootPath();
            std::filesystem::path assetsDir = root / "Assets";

            std::vector<std::filesystem::path> texPaths;
            if (std::filesystem::exists(assetsDir))
            {
                std::error_code ec;
                for (const auto &it : std::filesystem::recursive_directory_iterator(assetsDir, ec))
                {
                    if (!it.is_directory())
                    {
                        std::string ext = it.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                        {
                            texPaths.push_back(it.path());
                        }
                    }
                }
            }

            for (const auto &p : texPaths)
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

                bool isSelected = (m_pendingTexturePath == p);
                if (ImGui::Selectable(fname.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    m_pendingTexturePath = p;
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        applySelection = true;
                    }
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s\n(Double-click to select immediately)", p.string().c_str());
                }
            }

            ImGui::EndChild();

            ImGui::Separator();
            if (m_pendingTexturePath.empty())
            {
                ImGui::TextDisabled("Selected: (None)");
            }
            else
            {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Selected: %s", m_pendingTexturePath.filename().string().c_str());
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
                m_showTexturePicker = false;
                ImGui::CloseCurrentPopup();
            }

            if (applySelection && !m_pendingTexturePath.empty())
            {
                loadTexture(m_pendingTexturePath);
                m_showTexturePicker = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void SpriteEditor::Render(EditorState &state)
    {
        if (!m_isOpen)
            return;

        if (ImGui::Begin("Sprite Editor", &m_isOpen))
        {
            float availWidth = ImGui::GetContentRegionAvail().x;
            float sidebarWidth = 280.0f;
            float canvasWidth = std::max(100.0f, availWidth - sidebarWidth - 10.0f);

            // Left: Canvas
            ImGui::BeginChild("SpriteCanvasArea", ImVec2(canvasWidth, 0), true);
            renderCanvas(state);
            ImGui::EndChild();

            ImGui::SameLine();

            // Right: Sidebar controls
            ImGui::BeginChild("SpriteSidebarArea", ImVec2(sidebarWidth, 0), true);
            renderSidebar(state);
            ImGui::EndChild();

            renderTexturePickerModal(state);
        }
        ImGui::End();
    }
} // namespace RoninEditor
