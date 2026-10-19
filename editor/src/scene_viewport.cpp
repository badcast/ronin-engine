#include "scene_viewport.h"
#include "project_manager.h"
#include <SDL2/SDL_image.h>
#include <SimpleJSON/json.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace RoninEditor
{
    static void drawFilledCircle(SDL_Renderer *renderer, int cx, int cy, int radius)
    {
        for (int w = 0; w < radius * 2; ++w)
        {
            for (int h = 0; h < radius * 2; ++h)
            {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx * dx + dy * dy) <= (radius * radius))
                {
                    SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
                }
            }
        }
    }

    static void drawWireCircle(SDL_Renderer *renderer, int cx, int cy, int radius)
    {
        int x = radius - 1;
        int y = 0;
        int dx = 1;
        int dy = 1;
        int err = dx - (radius << 1);

        while (x >= y)
        {
            SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            SDL_RenderDrawPoint(renderer, cx + y, cy + x);
            SDL_RenderDrawPoint(renderer, cx - y, cy + x);
            SDL_RenderDrawPoint(renderer, cx - x, cy + y);
            SDL_RenderDrawPoint(renderer, cx - x, cy - y);
            SDL_RenderDrawPoint(renderer, cx - y, cy - x);
            SDL_RenderDrawPoint(renderer, cx + y, cy - x);
            SDL_RenderDrawPoint(renderer, cx + x, cy - y);

            if (err <= 0)
            {
                y++;
                err += dy;
                dy += 2;
            }
            if (err > 0)
            {
                x--;
                dx += 2;
                err += dx - (radius << 1);
            }
        }
    }

    static void getRotatedBoxCorners(
        const RoninEngine::Runtime::Vec2 &center,
        const RoninEngine::Runtime::Vec2 &size,
        float angleDegrees,
        RoninEngine::Runtime::Vec2 outCorners[4])
    {
        float rad = angleDegrees * (3.1415926535f / 180.0f);
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        float hw = size.x * 0.5f;
        float hh = size.y * 0.5f;

        // Local corners (world coordinates where Y is up):
        // 0: top-left, 1: top-right, 2: bottom-right, 3: bottom-left
        RoninEngine::Runtime::Vec2 local[4] = {
            {-hw,  hh},
            { hw,  hh},
            { hw, -hh},
            {-hw, -hh}
        };

        for (int i = 0; i < 4; ++i)
        {
            float rx = local[i].x * cosA - local[i].y * sinA;
            float ry = local[i].x * sinA + local[i].y * cosA;
            outCorners[i] = {center.x + rx, center.y + ry};
        }
    }

    static void drawRotatedQuad(
        SDL_Renderer *renderer,
        const RoninEngine::Runtime::Vec2 screenCorners[4],
        SDL_Color col)
    {
        SDL_Vertex vertices[4];
        for (int i = 0; i < 4; ++i)
        {
            vertices[i].position.x = screenCorners[i].x;
            vertices[i].position.y = screenCorners[i].y;
            vertices[i].color = col;
            vertices[i].tex_coord = {0.0f, 0.0f};
        }
        int indices[6] = {0, 1, 2, 0, 2, 3};
        SDL_RenderGeometry(renderer, nullptr, vertices, 4, indices, 6);
    }

    static void drawRotatedWireQuad(
        SDL_Renderer *renderer,
        const RoninEngine::Runtime::Vec2 screenCorners[4],
        SDL_Color col)
    {
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        for (int i = 0; i < 4; ++i)
        {
            int next = (i + 1) % 4;
            SDL_RenderDrawLine(renderer,
                static_cast<int>(screenCorners[i].x), static_cast<int>(screenCorners[i].y),
                static_cast<int>(screenCorners[next].x), static_cast<int>(screenCorners[next].y));
        }
    }

    SceneViewport::SceneViewport() = default;

    SceneViewport::~SceneViewport()
    {
        Shutdown();
    }

    void SceneViewport::Init(SDL_Renderer *renderer)
    {
        m_renderer = renderer;
    }

    void SceneViewport::Shutdown()
    {
        if (m_sceneTexture)
        {
            SDL_DestroyTexture(m_sceneTexture);
            m_sceneTexture = nullptr;
        }
        if (m_gameTexture)
        {
            SDL_DestroyTexture(m_gameTexture);
            m_gameTexture = nullptr;
        }

        for (auto &pair : m_textureCache)
        {
            if (pair.second)
                SDL_DestroyTexture(pair.second);
        }
        m_textureCache.clear();
        m_sceneWidth = 0;
        m_sceneHeight = 0;
        m_gameWidth = 0;
        m_gameHeight = 0;
    }

    SDL_Texture *SceneViewport::getOrLoadTexture(const std::string &relOrAbsPath, SDL_Rect *outSrcRect, bool *hasSrcRect)
    {
        if (hasSrcRect) *hasSrcRect = false;
        if (relOrAbsPath.empty()) return nullptr;

        auto root = ProjectManager::GetInstance().GetRootPath();
        std::filesystem::path fullPath = relOrAbsPath;
        if (fullPath.is_relative())
            fullPath = root / fullPath;

        if (!std::filesystem::exists(fullPath))
            return nullptr;

        std::string actualImgPath = fullPath.string();
        if (fullPath.extension() == ".sprite")
        {
            std::ifstream ifs(fullPath);
            if (ifs.is_open())
            {
                std::stringstream ss;
                ss << ifs.rdbuf();
                try
                {
                    auto js = json::JSON::Load(ss.str());
                    if (js.hasKey("texturePath"))
                    {
                        std::string texRel = js.at("texturePath").ToString();
                        std::filesystem::path texFull = texRel;
                        if (texFull.is_relative())
                            texFull = root / texFull;
                        if (std::filesystem::exists(texFull))
                        {
                            actualImgPath = texFull.string();
                            if (js.hasKey("rect") && outSrcRect && hasSrcRect)
                            {
                                auto r = js.at("rect");
                                if (r.size() >= 4)
                                {
                                    outSrcRect->x = r[0].ToInt();
                                    outSrcRect->y = r[1].ToInt();
                                    outSrcRect->w = r[2].ToInt();
                                    outSrcRect->h = r[3].ToInt();
                                    *hasSrcRect = true;
                                }
                            }
                        }
                    }
                }
                catch (...) {}
            }
        }

        auto it = m_textureCache.find(actualImgPath);
        if (it != m_textureCache.end())
            return it->second;

        SDL_Surface *surf = IMG_Load(actualImgPath.c_str());
        if (!surf) return nullptr;

        SDL_Texture *tex = SDL_CreateTextureFromSurface(m_renderer, surf);
        SDL_FreeSurface(surf);

        if (tex)
        {
            m_textureCache[actualImgPath] = tex;
        }
        return tex;
    }

    void SceneViewport::updateSceneTexture(int width, int height)
    {
        width = std::max(64, width);
        height = std::max(64, height);

        if (m_sceneTexture != nullptr && m_sceneWidth == width && m_sceneHeight == height)
            return;

        if (m_sceneTexture)
        {
            SDL_DestroyTexture(m_sceneTexture);
            m_sceneTexture = nullptr;
        }

        m_sceneWidth = width;
        m_sceneHeight = height;

        m_sceneTexture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            m_sceneWidth,
            m_sceneHeight);

        if (!m_sceneTexture)
        {
            std::cerr << "Failed to create Scene viewport texture: " << SDL_GetError() << std::endl;
        }
    }

    void SceneViewport::updateGameTexture(int width, int height)
    {
        width = std::max(64, width);
        height = std::max(64, height);

        if (m_gameTexture != nullptr && m_gameWidth == width && m_gameHeight == height)
            return;

        if (m_gameTexture)
        {
            SDL_DestroyTexture(m_gameTexture);
            m_gameTexture = nullptr;
        }

        m_gameWidth = width;
        m_gameHeight = height;

        m_gameTexture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            m_gameWidth,
            m_gameHeight);

        if (!m_gameTexture)
        {
            std::cerr << "Failed to create Game viewport texture: " << SDL_GetError() << std::endl;
        }
    }

    RoninEngine::Runtime::Vec2 SceneViewport::WorldToScreen(
        const RoninEngine::Runtime::Vec2 &worldPos,
        const RoninEngine::Runtime::Vec2 &viewCenter,
        const RoninEngine::Runtime::Vec2 &viewSize) const
    {
        float scale = m_pixelsPerUnit * m_zoom;
        float screenX = (worldPos.x - viewCenter.x) * scale + viewSize.x * 0.5f;
        // Invert Y: Ronin world Y goes up, screen Y goes down
        float screenY = (viewCenter.y - worldPos.y) * scale + viewSize.y * 0.5f;
        return {screenX, screenY};
    }

    RoninEngine::Runtime::Vec2 SceneViewport::ScreenToWorld(
        const RoninEngine::Runtime::Vec2 &screenPos,
        const RoninEngine::Runtime::Vec2 &viewCenter,
        const RoninEngine::Runtime::Vec2 &viewSize) const
    {
        float scale = m_pixelsPerUnit * m_zoom;
        if (scale < 0.0001f) scale = 0.0001f;
        float worldX = (screenPos.x - viewSize.x * 0.5f) / scale + viewCenter.x;
        float worldY = viewCenter.y - (screenPos.y - viewSize.y * 0.5f) / scale;
        return {worldX, worldY};
    }

    void SceneViewport::handleSceneInput(
        EditorState &state,
        const RoninEngine::Runtime::Vec2 &viewportPos,
        const RoninEngine::Runtime::Vec2 &viewportSize)
    {
        ImGuiIO &io = ImGui::GetIO();
        RoninEngine::Runtime::Vec2 mouseScreen = {io.MousePos.x - viewportPos.x, io.MousePos.y - viewportPos.y};
        bool isHovered = (mouseScreen.x >= 0.0f && mouseScreen.x < viewportSize.x &&
                          mouseScreen.y >= 0.0f && mouseScreen.y < viewportSize.y);

        if (!isHovered && !m_isDraggingGizmo && !m_isPanning)
            return;

        // 1. Mouse Wheel Zoom (centered around mouse cursor)
        if (isHovered && io.MouseWheel != 0.0f)
        {
            RoninEngine::Runtime::Vec2 worldBefore = ScreenToWorld(mouseScreen, m_cameraPos, viewportSize);
            float zoomFactor = (io.MouseWheel > 0.0f) ? 1.15f : 0.8695f;
            m_zoom = std::clamp(m_zoom * zoomFactor, 0.001f, 1000.0f);
            RoninEngine::Runtime::Vec2 worldAfter = ScreenToWorld(mouseScreen, m_cameraPos, viewportSize);
            m_cameraPos.x -= (worldAfter.x - worldBefore.x);
            m_cameraPos.y -= (worldAfter.y - worldBefore.y);
        }

        // 2. Middle Mouse or Right Mouse Drag Pan
        bool panButtonDown = io.MouseDown[2] || (io.MouseDown[1] && !m_isDraggingGizmo) || (io.KeyAlt && io.MouseDown[0]);
        if (panButtonDown)
        {
            if (!m_isPanning && isHovered)
            {
                m_isPanning = true;
                m_panStartMouse = mouseScreen;
                m_panStartCamera = m_cameraPos;
            }
            else if (m_isPanning)
            {
                float scale = m_pixelsPerUnit * m_zoom;
                float dx = (mouseScreen.x - m_panStartMouse.x) / scale;
                float dy = (mouseScreen.y - m_panStartMouse.y) / scale;
                m_cameraPos.x = m_panStartCamera.x - dx;
                m_cameraPos.y = m_panStartCamera.y + dy;
            }
        }
        else
        {
            m_isPanning = false;
        }

        // 3. Gizmo Interaction & Object Picking (Left Click)
        EditorObject *selObj = state.findObject(state.selectedObjectId);
        RoninEngine::Runtime::Vec2 objScreen = {0.0f, 0.0f};
        if (selObj)
        {
            objScreen = WorldToScreen(selObj->position, m_cameraPos, viewportSize);
        }

        float handleLength = 50.0f;
        float handleHitRadius = 12.0f;

        if (state.gizmoTool == GizmoToolMode::TerrainPaint && isHovered && !m_isPanning)
        {
            EditorObject *terrainObj = (selObj && selObj->terrain.enabled) ? selObj : state.findActiveTerrainObject();
            if (terrainObj && terrainObj->terrain.enabled)
            {
                float totalW = terrainObj->terrain.width * terrainObj->terrain.cellScale;
                float totalH = terrainObj->terrain.length * terrainObj->terrain.cellScale;
                float minX = terrainObj->position.x - totalW * 0.5f;
                float minY = terrainObj->position.y - totalH * 0.5f;

                RoninEngine::Runtime::Vec2 worldMouse = ScreenToWorld(mouseScreen, m_cameraPos, viewportSize);
                if (worldMouse.x >= minX && worldMouse.x <= minX + totalW &&
                    worldMouse.y >= minY && worldMouse.y <= minY + totalH)
                {
                    RoninEngine::Runtime::Vec2 uv(
                        (worldMouse.x - minX) / totalW,
                        (worldMouse.y - minY) / totalH);

                    if (io.MouseDown[0])
                    {
                        bool isDrag = (io.MouseDownDuration[0] > 0.0f);
                        if (state.paintTerrainAtUV(terrainObj->terrain, uv, isDrag))
                        {
                            state.isDirty = true;
                        }
                    }
                }
            }
        }
        else if (io.MouseClicked[0] && isHovered && !m_isPanning && !io.KeyAlt)
        {
            // Check if clicking Gizmo handles of selected object
            int clickedAxis = 0;
            if (selObj)
            {
                // Center box
                if (std::hypot(mouseScreen.x - objScreen.x, mouseScreen.y - objScreen.y) <= 12.0f)
                {
                    clickedAxis = 3; // Center / Free move
                }
                // X handle (pointing right)
                else if (mouseScreen.x >= objScreen.x && mouseScreen.x <= objScreen.x + handleLength &&
                         std::abs(mouseScreen.y - objScreen.y) <= handleHitRadius)
                {
                    clickedAxis = 1; // X axis
                }
                // Y handle (pointing up in world = pointing up in screen = -Y)
                else if (mouseScreen.y <= objScreen.y && mouseScreen.y >= objScreen.y - handleLength &&
                         std::abs(mouseScreen.x - objScreen.x) <= handleHitRadius)
                {
                    clickedAxis = 2; // Y axis
                }
                // Rotation ring
                else if (state.gizmoTool == GizmoToolMode::Rotate)
                {
                    float dist = std::hypot(mouseScreen.x - objScreen.x, mouseScreen.y - objScreen.y);
                    if (std::abs(dist - 45.0f) <= 10.0f)
                    {
                        clickedAxis = 4; // Rotate
                    }
                }
            }

            if (clickedAxis > 0)
            {
                state.recordUndo("Transform " + (selObj ? selObj->name : "GameObject"));
                m_isDraggingGizmo = true;
                m_dragGizmoAxis = clickedAxis;
                m_dragStartMouse = mouseScreen;
                m_dragStartWorldPos = selObj->position;
                m_dragStartAngle = selObj->angle;
                m_dragStartScale = selObj->scale;
            }
            else
            {
                // Object picking in scene
                RoninEngine::Runtime::Vec2 worldMouse = ScreenToWorld(mouseScreen, m_cameraPos, viewportSize);
                int pickedId = state.pickObjectAt(worldMouse);
                state.selectObject(pickedId, io.KeyCtrl);
            }
        }

        // Hotkeys for Tool Switching, Focus & Deletion
        if ((isHovered || ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) && !io.WantTextInput)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_W))
            {
                state.gizmoTool = GizmoToolMode::SelectMove;
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_E))
            {
                state.gizmoTool = GizmoToolMode::Rotate;
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_R))
            {
                state.gizmoTool = GizmoToolMode::Scale;
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_T))
            {
                state.gizmoTool = GizmoToolMode::TerrainPaint;
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_F))
            {
                if (selObj)
                {
                    m_cameraPos = selObj->position;
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                if (!state.selectedObjectIds.empty())
                {
                    std::vector<int> toDel = state.selectedObjectIds;
                    for (int id : toDel)
                        state.deleteObject(id);
                }
                else if (state.selectedObjectId != -1)
                {
                    state.deleteObject(state.selectedObjectId);
                }
            }
        }

        // Dragging Gizmo update
        if (m_isDraggingGizmo)
        {
            if (io.MouseDown[0] && selObj)
            {
                float scale = m_pixelsPerUnit * m_zoom;
                float dx = (mouseScreen.x - m_dragStartMouse.x) / scale;
                float dy = -(mouseScreen.y - m_dragStartMouse.y) / scale;

                if (state.gizmoTool == GizmoToolMode::SelectMove)
                {
                    if (m_dragGizmoAxis == 1) // X only
                    {
                        selObj->position.x = m_dragStartWorldPos.x + dx;
                    }
                    else if (m_dragGizmoAxis == 2) // Y only
                    {
                        selObj->position.y = m_dragStartWorldPos.y + dy;
                    }
                    else if (m_dragGizmoAxis == 3) // Center free move
                    {
                        selObj->position.x = m_dragStartWorldPos.x + dx;
                        selObj->position.y = m_dragStartWorldPos.y + dy;
                    }

                    if (state.snapToGrid && state.gridStep > 0.0f)
                    {
                        selObj->position.x = std::round(selObj->position.x / state.gridStep) * state.gridStep;
                        selObj->position.y = std::round(selObj->position.y / state.gridStep) * state.gridStep;
                    }
                }
                else if (state.gizmoTool == GizmoToolMode::Rotate)
                {
                    float startAngle = std::atan2(-(m_dragStartMouse.y - objScreen.y), m_dragStartMouse.x - objScreen.x);
                    float curAngle = std::atan2(-(mouseScreen.y - objScreen.y), mouseScreen.x - objScreen.x);
                    float deltaDeg = (curAngle - startAngle) * 180.0f / 3.14159265f;
                    selObj->angle = m_dragStartAngle + deltaDeg;
                }
                else if (state.gizmoTool == GizmoToolMode::Scale)
                {
                    if (m_dragGizmoAxis == 1)
                        selObj->scale.x = std::max(0.1f, m_dragStartScale.x + dx);
                    else if (m_dragGizmoAxis == 2)
                        selObj->scale.y = std::max(0.1f, m_dragStartScale.y + dy);
                    else
                    {
                        float f = 1.0f + (dx + dy) * 0.5f;
                        selObj->scale.x = std::max(0.1f, m_dragStartScale.x * f);
                        selObj->scale.y = std::max(0.1f, m_dragStartScale.y * f);
                    }
                }
                state.isDirty = true;
            }
            else
            {
                m_isDraggingGizmo = false;
                m_dragGizmoAxis = 0;
            }
        }
    }

    void SceneViewport::renderSceneToTexture(EditorState &state)
    {
        if (!m_renderer || !m_sceneTexture)
            return;

        SDL_Texture *prevTarget = SDL_GetRenderTarget(m_renderer);
        SDL_SetRenderTarget(m_renderer, m_sceneTexture);

        // 1. Clear background
        SDL_SetRenderDrawColor(m_renderer, 28, 30, 36, 255);
        SDL_RenderClear(m_renderer);

        RoninEngine::Runtime::Vec2 viewSize = {static_cast<float>(m_sceneWidth), static_cast<float>(m_sceneHeight)};

        // 2. Draw Grid
        if (state.showGrid)
        {
            RoninEngine::Runtime::Vec2 worldMin = ScreenToWorld({0.0f, viewSize.y}, m_cameraPos, viewSize);
            RoninEngine::Runtime::Vec2 worldMax = ScreenToWorld({viewSize.x, 0.0f}, m_cameraPos, viewSize);

            // Minor grid lines
            SDL_SetRenderDrawColor(m_renderer, 38, 42, 50, 255);
            int minX = static_cast<int>(std::floor(worldMin.x)) - 1;
            int maxX = static_cast<int>(std::ceil(worldMax.x)) + 1;
            int minY = static_cast<int>(std::floor(worldMin.y)) - 1;
            int maxY = static_cast<int>(std::ceil(worldMax.y)) + 1;

            for (int x = minX; x <= maxX; ++x)
            {
                if (x % 5 == 0) continue;
                auto sp = WorldToScreen({static_cast<float>(x), 0.0f}, m_cameraPos, viewSize);
                SDL_RenderDrawLine(m_renderer, static_cast<int>(sp.x), 0, static_cast<int>(sp.x), m_sceneHeight);
            }
            for (int y = minY; y <= maxY; ++y)
            {
                if (y % 5 == 0) continue;
                auto sp = WorldToScreen({0.0f, static_cast<float>(y)}, m_cameraPos, viewSize);
                SDL_RenderDrawLine(m_renderer, 0, static_cast<int>(sp.y), m_sceneWidth, static_cast<int>(sp.y));
            }

            // Major grid lines (every 5 units)
            SDL_SetRenderDrawColor(m_renderer, 52, 56, 68, 255);
            for (int x = minX; x <= maxX; ++x)
            {
                if (x % 5 != 0) continue;
                auto sp = WorldToScreen({static_cast<float>(x), 0.0f}, m_cameraPos, viewSize);
                SDL_RenderDrawLine(m_renderer, static_cast<int>(sp.x), 0, static_cast<int>(sp.x), m_sceneHeight);
            }
            for (int y = minY; y <= maxY; ++y)
            {
                if (y % 5 != 0) continue;
                auto sp = WorldToScreen({0.0f, static_cast<float>(y)}, m_cameraPos, viewSize);
                SDL_RenderDrawLine(m_renderer, 0, static_cast<int>(sp.y), m_sceneWidth, static_cast<int>(sp.y));
            }

            // Coordinate Origin Axes
            auto origin = WorldToScreen({0.0f, 0.0f}, m_cameraPos, viewSize);
            // X-Axis (Red)
            SDL_SetRenderDrawColor(m_renderer, 220, 60, 60, 255);
            SDL_RenderDrawLine(m_renderer, 0, static_cast<int>(origin.y), m_sceneWidth, static_cast<int>(origin.y));
            // Y-Axis (Green)
            SDL_SetRenderDrawColor(m_renderer, 60, 200, 60, 255);
            SDL_RenderDrawLine(m_renderer, static_cast<int>(origin.x), 0, static_cast<int>(origin.x), m_sceneHeight);
        }

        // 3. Draw GameObjects
        const auto &objects = state.activeWorld().objects;
        for (const auto &obj : objects)
        {
            if (!obj.active)
                continue;

            auto screenCenter = WorldToScreen(obj.position, m_cameraPos, viewSize);

            // A. SpriteRenderer (Supports texture, slices, flips, and angle rotation)
            if (obj.sprite.enabled)
            {
                float w = obj.sprite.size.x * obj.scale.x;
                float h = obj.sprite.size.y * obj.scale.y;

                SDL_Rect srcRect{0, 0, 0, 0};
                bool hasSrcRect = false;
                SDL_Texture *tex = getOrLoadTexture(obj.sprite.spritePath, &srcRect, &hasSrcRect);

                if (tex != nullptr && obj.sprite.primitiveShape != 1)
                {
                    float dstW = w * m_pixelsPerUnit * m_zoom;
                    float dstH = h * m_pixelsPerUnit * m_zoom;
                    SDL_Rect dstRect{
                        static_cast<int>(screenCenter.x - dstW * 0.5f),
                        static_cast<int>(screenCenter.y - dstH * 0.5f),
                        static_cast<int>(dstW),
                        static_cast<int>(dstH)
                    };

                    SDL_SetTextureColorMod(tex, obj.sprite.color.r, obj.sprite.color.g, obj.sprite.color.b);
                    SDL_SetTextureAlphaMod(tex, obj.sprite.color.a);

                    SDL_RendererFlip flip = SDL_FLIP_NONE;
                    if (obj.sprite.flip & 1) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
                    if (obj.sprite.flip & 2) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);

                    SDL_RenderCopyEx(m_renderer, tex, hasSrcRect ? &srcRect : nullptr, &dstRect, obj.angle, nullptr, flip);
                }
                else if (obj.sprite.primitiveShape == 1) // Circle
                {
                    float rPx = std::min(w, h) * 0.5f * m_pixelsPerUnit * m_zoom;
                    SDL_SetRenderDrawColor(m_renderer, obj.sprite.color.r, obj.sprite.color.g, obj.sprite.color.b, obj.sprite.color.a);
                    drawFilledCircle(m_renderer, static_cast<int>(screenCenter.x), static_cast<int>(screenCenter.y), static_cast<int>(rPx));
                }
                else
                {
                    RoninEngine::Runtime::Vec2 worldCorners[4];
                    getRotatedBoxCorners(obj.position, {w, h}, obj.angle, worldCorners);
                    RoninEngine::Runtime::Vec2 screenCorners[4];
                    for (int i = 0; i < 4; ++i)
                        screenCorners[i] = WorldToScreen(worldCorners[i], m_cameraPos, viewSize);

                    SDL_Color col{obj.sprite.color.r, obj.sprite.color.g, obj.sprite.color.b, obj.sprite.color.a};
                    drawRotatedQuad(m_renderer, screenCorners, col);
                }
            }

            // B. Terrain2D (Rotates all cells, colliders, and surface strokes around obj.position)
            if (obj.terrain.enabled)
            {
                float cellW = obj.terrain.cellScale;
                float cellH = obj.terrain.cellScale;
                float totalW = obj.terrain.width * cellW;
                float totalH = obj.terrain.length * cellH;

                float rad = obj.angle * (3.1415926535f / 180.0f);
                float cosA = std::cos(rad);
                float sinA = std::sin(rad);

                for (int r = 0; r < obj.terrain.length; ++r)
                {
                    for (int c = 0; c < obj.terrain.width; ++c)
                    {
                        size_t idx = static_cast<size_t>(r * obj.terrain.width + c);
                        auto color = (idx < obj.terrain.cellColors.size()) ? obj.terrain.cellColors[idx] : obj.terrain.tileColor;
                        if (color.a == 0) continue;

                        float localCellX = (c + 0.5f - obj.terrain.width * 0.5f) * cellW;
                        float localCellY = (obj.terrain.length * 0.5f - (r + 0.5f)) * cellH;

                        RoninEngine::Runtime::Vec2 cellCenterWorld(
                            obj.position.x + localCellX * cosA - localCellY * sinA,
                            obj.position.y + localCellX * sinA + localCellY * cosA
                        );

                        RoninEngine::Runtime::Vec2 cellWorldCorners[4];
                        getRotatedBoxCorners(cellCenterWorld, {cellW, cellH}, obj.angle, cellWorldCorners);

                        RoninEngine::Runtime::Vec2 cellScreenCorners[4];
                        for (int i = 0; i < 4; ++i)
                            cellScreenCorners[i] = WorldToScreen(cellWorldCorners[i], m_cameraPos, viewSize);

                        SDL_Color scCol{color.r, color.g, color.b, color.a};
                        drawRotatedQuad(m_renderer, cellScreenCorners, scCol);

                        // Draw collider boundary if present
                        if (idx < obj.terrain.cellColliders.size() && obj.terrain.cellColliders[idx])
                        {
                            drawRotatedWireQuad(m_renderer, cellScreenCorners, {240, 160, 40, 180});
                        }
                    }
                }

                // Render freehand paint strokes on the terrain surface
                for (const auto &st : obj.terrain.paintStrokes)
                {
                    float localStrokeX = (st.uvPos.x - 0.5f) * totalW;
                    float localStrokeY = (0.5f - st.uvPos.y) * totalH;

                    RoninEngine::Runtime::Vec2 strokeWorld(
                        obj.position.x + localStrokeX * cosA - localStrokeY * sinA,
                        obj.position.y + localStrokeX * sinA + localStrokeY * cosA
                    );

                    auto strokeScreen = WorldToScreen(strokeWorld, m_cameraPos, viewSize);
                    float radiusPx = std::max(2.0f, st.radiusNormalized * totalW * m_pixelsPerUnit * m_zoom);
                    Uint8 alpha = static_cast<Uint8>(std::clamp(st.color.a * st.opacity, 10.0f, 255.0f));
                    SDL_SetRenderDrawColor(m_renderer, st.color.r, st.color.g, st.color.b, alpha);
                    drawFilledCircle(m_renderer, static_cast<int>(strokeScreen.x), static_cast<int>(strokeScreen.y), static_cast<int>(radiusPx));
                }
            }

            // C. Light2D Gizmo
            if (obj.light.enabled)
            {
                SDL_SetRenderDrawColor(m_renderer, obj.light.color.r, obj.light.color.g, obj.light.color.b, 220);
                drawFilledCircle(m_renderer, static_cast<int>(screenCenter.x), static_cast<int>(screenCenter.y), 6);
                float radiusPx = obj.light.range * m_pixelsPerUnit * m_zoom;
                drawWireCircle(m_renderer, static_cast<int>(screenCenter.x), static_cast<int>(screenCenter.y), static_cast<int>(radiusPx));
            }

            // D. Camera2D Gizmo
            if (obj.camera.enabled)
            {
                SDL_SetRenderDrawColor(m_renderer, 80, 180, 240, 255);
                drawWireCircle(m_renderer, static_cast<int>(screenCenter.x), static_cast<int>(screenCenter.y), 8);
                // Camera frustum wire rectangle
                float fw = 16.0f * (m_pixelsPerUnit / 64.0f) * m_zoom * 24.0f;
                float fh = 9.0f * (m_pixelsPerUnit / 64.0f) * m_zoom * 24.0f;
                RoninEngine::Runtime::Vec2 frCorners[4];
                getRotatedBoxCorners(obj.position, {fw / (m_pixelsPerUnit * m_zoom), fh / (m_pixelsPerUnit * m_zoom)}, obj.angle, frCorners);
                RoninEngine::Runtime::Vec2 frScreen[4];
                for (int i = 0; i < 4; ++i)
                    frScreen[i] = WorldToScreen(frCorners[i], m_cameraPos, viewSize);
                drawRotatedWireQuad(m_renderer, frScreen, {80, 180, 240, 255});
            }

            // E. Physics Collider Gizmo (Rotated with obj.angle)
            bool hasCollider = obj.boxCollider2D.enabled || obj.physics.colliderEnabled;
            if (hasCollider)
            {
                if (obj.physics.colliderEnabled && obj.physics.colliderType == 1) // Circle
                {
                    SDL_SetRenderDrawColor(m_renderer, 40, 240, 80, 220);
                    float rPx = obj.physics.colliderRadius * m_pixelsPerUnit * m_zoom;
                    drawWireCircle(m_renderer, static_cast<int>(screenCenter.x), static_cast<int>(screenCenter.y), static_cast<int>(rPx));
                }
                else // Box
                {
                    RoninEngine::Runtime::Vec2 colSize = obj.boxCollider2D.enabled ? obj.boxCollider2D.size : obj.physics.colliderSize;
                    RoninEngine::Runtime::Vec2 colOffset = obj.boxCollider2D.enabled ? obj.boxCollider2D.offset : RoninEngine::Runtime::Vec2{0.0f, 0.0f};

                    float rad = obj.angle * (3.1415926535f / 180.0f);
                    float cosA = std::cos(rad);
                    float sinA = std::sin(rad);

                    RoninEngine::Runtime::Vec2 worldCenter(
                        obj.position.x + colOffset.x * cosA - colOffset.y * sinA,
                        obj.position.y + colOffset.x * sinA + colOffset.y * cosA
                    );

                    RoninEngine::Runtime::Vec2 worldCorners[4];
                    getRotatedBoxCorners(worldCenter, colSize, obj.angle, worldCorners);

                    RoninEngine::Runtime::Vec2 screenCorners[4];
                    for (int i = 0; i < 4; ++i)
                        screenCorners[i] = WorldToScreen(worldCorners[i], m_cameraPos, viewSize);

                    drawRotatedWireQuad(m_renderer, screenCorners, {40, 240, 80, 220});
                }
            }
        }

        // 4. Draw Selected Object Highlight & Transform Gizmo
        EditorObject *sel = state.findObject(state.selectedObjectId);
        if (sel)
        {
            auto selCenter = WorldToScreen(sel->position, m_cameraPos, viewSize);

            // Rotated Selection box
            auto b = state.getObjectWorldBounds(*sel);
            RoninEngine::Runtime::Vec2 selWorldCorners[4];
            getRotatedBoxCorners(sel->position, {b.w, b.h}, sel->angle, selWorldCorners);

            RoninEngine::Runtime::Vec2 selScreenCorners[4];
            for (int i = 0; i < 4; ++i)
                selScreenCorners[i] = WorldToScreen(selWorldCorners[i], m_cameraPos, viewSize);

            drawRotatedWireQuad(m_renderer, selScreenCorners, {60, 220, 255, 255});

            // Gizmos
            float handleLen = 50.0f;
            int cx = static_cast<int>(selCenter.x);
            int cy = static_cast<int>(selCenter.y);

            if (state.gizmoTool == GizmoToolMode::SelectMove)
            {
                // Center square (Free move handle)
                SDL_Rect centerBox{cx - 5, cy - 5, 11, 11};
                SDL_SetRenderDrawColor(m_renderer, 240, 240, 60, 255);
                SDL_RenderFillRect(m_renderer, &centerBox);

                // +X Axis handle (Red)
                SDL_SetRenderDrawColor(m_renderer, 240, 60, 60, 255);
                SDL_RenderDrawLine(m_renderer, cx, cy, cx + static_cast<int>(handleLen), cy);
                SDL_RenderDrawLine(m_renderer, cx, cy - 1, cx + static_cast<int>(handleLen), cy - 1);
                SDL_RenderDrawLine(m_renderer, cx, cy + 1, cx + static_cast<int>(handleLen), cy + 1);
                // Arrow head
                SDL_RenderDrawLine(m_renderer, cx + static_cast<int>(handleLen), cy, cx + static_cast<int>(handleLen) - 8, cy - 5);
                SDL_RenderDrawLine(m_renderer, cx + static_cast<int>(handleLen), cy, cx + static_cast<int>(handleLen) - 8, cy + 5);

                // +Y Axis handle (Green, pointing up in screen = -Y)
                SDL_SetRenderDrawColor(m_renderer, 60, 220, 60, 255);
                SDL_RenderDrawLine(m_renderer, cx, cy, cx, cy - static_cast<int>(handleLen));
                SDL_RenderDrawLine(m_renderer, cx - 1, cy, cx - 1, cy - static_cast<int>(handleLen));
                SDL_RenderDrawLine(m_renderer, cx + 1, cy, cx + 1, cy - static_cast<int>(handleLen));
                // Arrow head
                SDL_RenderDrawLine(m_renderer, cx, cy - static_cast<int>(handleLen), cx - 5, cy - static_cast<int>(handleLen) + 8);
                SDL_RenderDrawLine(m_renderer, cx, cy - static_cast<int>(handleLen), cx + 5, cy - static_cast<int>(handleLen) + 8);
            }
            else if (state.gizmoTool == GizmoToolMode::Rotate)
            {
                SDL_SetRenderDrawColor(m_renderer, 60, 180, 240, 255);
                drawWireCircle(m_renderer, cx, cy, 45);
                drawWireCircle(m_renderer, cx, cy, 46);
                // Orientation marker
                float rad = sel->angle * 3.14159265f / 180.0f;
                int ax = cx + static_cast<int>(std::cos(rad) * 45.0f);
                int ay = cy - static_cast<int>(std::sin(rad) * 45.0f);
                SDL_SetRenderDrawColor(m_renderer, 240, 240, 60, 255);
                drawFilledCircle(m_renderer, ax, ay, 4);
            }
            else if (state.gizmoTool == GizmoToolMode::Scale)
            {
                // X scale handle (Red with box)
                SDL_SetRenderDrawColor(m_renderer, 240, 60, 60, 255);
                SDL_RenderDrawLine(m_renderer, cx, cy, cx + static_cast<int>(handleLen), cy);
                SDL_Rect rBox{cx + static_cast<int>(handleLen) - 4, cy - 4, 9, 9};
                SDL_RenderFillRect(m_renderer, &rBox);

                // Y scale handle (Green with box)
                SDL_SetRenderDrawColor(m_renderer, 60, 220, 60, 255);
                SDL_RenderDrawLine(m_renderer, cx, cy, cx, cy - static_cast<int>(handleLen));
                SDL_Rect gBox{cx - 4, cy - static_cast<int>(handleLen) - 4, 9, 9};
                SDL_RenderFillRect(m_renderer, &gBox);
            }
        }

        SDL_SetRenderTarget(m_renderer, prevTarget);
    }

    void SceneViewport::renderGameToTexture(EditorState &state)
    {
        if (!m_renderer || !m_gameTexture)
            return;

        SDL_Texture *prevTarget = SDL_GetRenderTarget(m_renderer);
        SDL_SetRenderTarget(m_renderer, m_gameTexture);

        // Find primary camera in active world
        const EditorObject *camObj = state.findMainCameraObject();
        RoninEngine::Runtime::Vec2 camPos = camObj ? camObj->position : RoninEngine::Runtime::Vec2{0.0f, 0.0f};
        float camZoom = camObj ? std::max(0.0001f, camObj->camera.zoom) : 1.0f;
        auto clearColor = camObj ? camObj->camera.backcolor : RoninEngine::Runtime::Color{18, 22, 30, 255};

        // 1. Clear background with camera background color
        SDL_SetRenderDrawColor(m_renderer, clearColor.r, clearColor.g, clearColor.b, 255);
        SDL_RenderClear(m_renderer);

        RoninEngine::Runtime::Vec2 viewSize = {static_cast<float>(m_gameWidth), static_cast<float>(m_gameHeight)};
        float scale = m_pixelsPerUnit * camZoom;

        auto worldToGame = [&](const RoninEngine::Runtime::Vec2 &wp) -> RoninEngine::Runtime::Vec2 {
            return {
                (wp.x - camPos.x) * scale + viewSize.x * 0.5f,
                (camPos.y - wp.y) * scale + viewSize.y * 0.5f
            };
        };

        // 2. Draw all game objects from game camera perspective
        const auto &objects = state.activeWorld().objects;
        for (const auto &obj : objects)
        {
            if (!obj.active)
                continue;

            auto screenCenter = worldToGame(obj.position);

            if (obj.sprite.enabled)
            {
                float w = obj.sprite.size.x * obj.scale.x;
                float h = obj.sprite.size.y * obj.scale.y;

                SDL_Rect srcRect{0, 0, 0, 0};
                bool hasSrcRect = false;
                SDL_Texture *tex = getOrLoadTexture(obj.sprite.spritePath, &srcRect, &hasSrcRect);

                if (tex != nullptr && obj.sprite.primitiveShape != 1)
                {
                    float dstW = w * scale;
                    float dstH = h * scale;
                    SDL_Rect dstRect{
                        static_cast<int>(screenCenter.x - dstW * 0.5f),
                        static_cast<int>(screenCenter.y - dstH * 0.5f),
                        static_cast<int>(dstW),
                        static_cast<int>(dstH)
                    };

                    SDL_SetTextureColorMod(tex, obj.sprite.color.r, obj.sprite.color.g, obj.sprite.color.b);
                    SDL_SetTextureAlphaMod(tex, obj.sprite.color.a);

                    SDL_RendererFlip flip = SDL_FLIP_NONE;
                    if (obj.sprite.flip & 1) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
                    if (obj.sprite.flip & 2) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);

                    SDL_RenderCopyEx(m_renderer, tex, hasSrcRect ? &srcRect : nullptr, &dstRect, obj.angle, nullptr, flip);
                }
                else if (obj.sprite.primitiveShape == 1)
                {
                    float rPx = std::min(w, h) * 0.5f * scale;
                    SDL_SetRenderDrawColor(m_renderer, obj.sprite.color.r, obj.sprite.color.g, obj.sprite.color.b, obj.sprite.color.a);
                    drawFilledCircle(m_renderer, static_cast<int>(screenCenter.x), static_cast<int>(screenCenter.y), static_cast<int>(rPx));
                }
                else
                {
                    RoninEngine::Runtime::Vec2 worldCorners[4];
                    getRotatedBoxCorners(obj.position, {w, h}, obj.angle, worldCorners);
                    RoninEngine::Runtime::Vec2 screenCorners[4];
                    for (int i = 0; i < 4; ++i)
                        screenCorners[i] = worldToGame(worldCorners[i]);

                    SDL_Color col{obj.sprite.color.r, obj.sprite.color.g, obj.sprite.color.b, obj.sprite.color.a};
                    drawRotatedQuad(m_renderer, screenCorners, col);
                }
            }

            if (obj.terrain.enabled)
            {
                float cellW = obj.terrain.cellScale;
                float cellH = obj.terrain.cellScale;
                float totalW = obj.terrain.width * cellW;
                float totalH = obj.terrain.length * cellH;

                float rad = obj.angle * (3.1415926535f / 180.0f);
                float cosA = std::cos(rad);
                float sinA = std::sin(rad);

                for (int r = 0; r < obj.terrain.length; ++r)
                {
                    for (int c = 0; c < obj.terrain.width; ++c)
                    {
                        size_t idx = static_cast<size_t>(r * obj.terrain.width + c);
                        auto color = (idx < obj.terrain.cellColors.size()) ? obj.terrain.cellColors[idx] : obj.terrain.tileColor;
                        if (color.a == 0) continue;

                        float localCellX = (c + 0.5f - obj.terrain.width * 0.5f) * cellW;
                        float localCellY = (obj.terrain.length * 0.5f - (r + 0.5f)) * cellH;

                        RoninEngine::Runtime::Vec2 cellCenterWorld(
                            obj.position.x + localCellX * cosA - localCellY * sinA,
                            obj.position.y + localCellX * sinA + localCellY * cosA
                        );

                        RoninEngine::Runtime::Vec2 cellWorldCorners[4];
                        getRotatedBoxCorners(cellCenterWorld, {cellW, cellH}, obj.angle, cellWorldCorners);

                        RoninEngine::Runtime::Vec2 cellScreenCorners[4];
                        for (int i = 0; i < 4; ++i)
                            cellScreenCorners[i] = worldToGame(cellWorldCorners[i]);

                        SDL_Color scCol{color.r, color.g, color.b, color.a};
                        drawRotatedQuad(m_renderer, cellScreenCorners, scCol);
                    }
                }

                // Render freehand paint strokes on the terrain surface
                for (const auto &st : obj.terrain.paintStrokes)
                {
                    float localStrokeX = (st.uvPos.x - 0.5f) * totalW;
                    float localStrokeY = (0.5f - st.uvPos.y) * totalH;

                    RoninEngine::Runtime::Vec2 strokeWorld(
                        obj.position.x + localStrokeX * cosA - localStrokeY * sinA,
                        obj.position.y + localStrokeX * sinA + localStrokeY * cosA
                    );

                    auto strokeScreen = worldToGame(strokeWorld);
                    float radiusPx = std::max(2.0f, st.radiusNormalized * totalW * scale);
                    Uint8 alpha = static_cast<Uint8>(std::clamp(st.color.a * st.opacity, 10.0f, 255.0f));
                    SDL_SetRenderDrawColor(m_renderer, st.color.r, st.color.g, st.color.b, alpha);
                    drawFilledCircle(m_renderer, static_cast<int>(strokeScreen.x), static_cast<int>(strokeScreen.y), static_cast<int>(radiusPx));
                }
            }
        }

        // 3. Draw Game GUI Elements (UI::GUI)
        for (const auto &gui : state.activeWorld().guiElements)
        {
            if (!gui.visible) continue;
            SDL_Rect gr{gui.rect.x, gui.rect.y, gui.rect.w, gui.rect.h};
            SDL_SetRenderDrawColor(m_renderer, 45, 52, 68, 220);
            SDL_RenderFillRect(m_renderer, &gr);
            SDL_SetRenderDrawColor(m_renderer, 80, 140, 220, 255);
            SDL_RenderDrawRect(m_renderer, &gr);
        }

        SDL_SetRenderTarget(m_renderer, prevTarget);
    }

    void SceneViewport::RenderScene(EditorState &state)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("World"))
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            int w = static_cast<int>(avail.x);
            int h = static_cast<int>(avail.y);

            if (w > 0 && h > 0)
            {
                updateSceneTexture(w, h);
                renderSceneToTexture(state);

                ImVec2 imgPos = ImGui::GetCursorScreenPos();
                ImGui::Image((ImTextureID)(intptr_t)m_sceneTexture, ImVec2(static_cast<float>(w), static_cast<float>(h)));

                // Drag & Drop Target for World Viewport:
                // Dragging a Prefab (or Sprite/Texture) onto the World instantiates it at the mouse coordinates!
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_PREFAB"))
                    {
                        std::string prefabPath((const char *)payload->Data);
                        ImVec2 mousePos = ImGui::GetMousePos();
                        RoninEngine::Runtime::Vec2 mouseScreen = {mousePos.x - imgPos.x, mousePos.y - imgPos.y};
                        RoninEngine::Runtime::Vec2 worldPos = ScreenToWorld(mouseScreen, m_cameraPos, {avail.x, avail.y});
                        state.instantiatePrefab(prefabPath, worldPos);
                    }
                    else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("RONIN_ASSET_PATH"))
                    {
                        std::string assetPath((const char *)payload->Data);
                        ImVec2 mousePos = ImGui::GetMousePos();
                        RoninEngine::Runtime::Vec2 mouseScreen = {mousePos.x - imgPos.x, mousePos.y - imgPos.y};
                        RoninEngine::Runtime::Vec2 worldPos = ScreenToWorld(mouseScreen, m_cameraPos, {avail.x, avail.y});

                        std::string ext = std::filesystem::path(assetPath).extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                        if (ext == ".prefab")
                        {
                            state.instantiatePrefab(assetPath, worldPos);
                        }
                        else if (ext == ".world")
                        {
                            state.loadWorldFromFile(assetPath);
                        }
                        else if (ext == ".sprite" || ext == ".png" || ext == ".jpg" || ext == ".bmp")
                        {
                            EditorObject &newObj = state.createObject(std::filesystem::path(assetPath).stem().string());
                            newObj.position = worldPos;
                            newObj.sprite.enabled = true;
                            newObj.sprite.spritePath = assetPath;
                            state.ensureComponentIds(newObj);
                            state.selectObject(newObj.id);
                            state.isDirty = true;
                            state.logInfo("Spawned Sprite object from: " + std::filesystem::path(assetPath).filename().string());
                        }
                    }
                    else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_SPRITE"))
                    {
                        std::string assetPath((const char *)payload->Data);
                        ImVec2 mousePos = ImGui::GetMousePos();
                        RoninEngine::Runtime::Vec2 mouseScreen = {mousePos.x - imgPos.x, mousePos.y - imgPos.y};
                        RoninEngine::Runtime::Vec2 worldPos = ScreenToWorld(mouseScreen, m_cameraPos, {avail.x, avail.y});
                        EditorObject &newObj = state.createObject(std::filesystem::path(assetPath).stem().string());
                        newObj.position = worldPos;
                        newObj.sprite.enabled = true;
                        newObj.sprite.spritePath = assetPath;
                        state.ensureComponentIds(newObj);
                        state.selectObject(newObj.id);
                        state.isDirty = true;
                    }
                    ImGui::EndDragDropTarget();
                }

                handleSceneInput(state, {imgPos.x, imgPos.y}, {avail.x, avail.y});

                // Top Toolbar Overlay in World Viewport
                ImGui::SetCursorScreenPos(ImVec2(imgPos.x + 10.0f, imgPos.y + 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.15f, 0.85f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 4.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 3.0f));

                if (ImGui::BeginChild("WorldControls", ImVec2(480.0f, 34.0f), true, ImGuiWindowFlags_NoScrollbar))
                {
                    const char *tools[] = {"Move", "Rotate", "Scale", "Terrain"};
                    for (int i = 0; i < 4; ++i)
                    {
                        bool active = (static_cast<int>(state.gizmoTool) == i);
                        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.55f, 0.85f, 1.0f));
                        if (ImGui::Button(tools[i]))
                        {
                            state.gizmoTool = static_cast<GizmoToolMode>(i);
                        }
                        if (active) ImGui::PopStyleColor();
                        ImGui::SameLine();
                    }

                    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                    ImGui::SameLine();

                    if (ImGui::Checkbox("Grid", &state.showGrid)) {}
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Snap", &state.snapToGrid)) {}
                    ImGui::SameLine();

                    if (ImGui::Button("Reset Cam"))
                    {
                        m_cameraPos = {0.0f, 0.0f};
                        m_zoom = 1.0f;
                    }
                    ImGui::EndChild();
                }

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void SceneViewport::RenderGame(EditorState &state)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Game"))
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x > 0.0f && avail.y > 0.0f)
            {
                float targetAspect = 0.0f;
                if (m_aspectRatioOption == 1) targetAspect = 16.0f / 9.0f;
                else if (m_aspectRatioOption == 2) targetAspect = 16.0f / 10.0f;
                else if (m_aspectRatioOption == 3) targetAspect = 4.0f / 3.0f;
                else if (m_aspectRatioOption == 4) targetAspect = 1.0f; // 1:1 Square

                float drawW = avail.x;
                float drawH = avail.y;
                float offsetX = 0.0f;
                float offsetY = 0.0f;

                if (targetAspect > 0.0f)
                {
                    float availAspect = avail.x / avail.y;
                    if (availAspect > targetAspect)
                    {
                        drawH = avail.y;
                        drawW = avail.y * targetAspect;
                        offsetX = (avail.x - drawW) * 0.5f;
                    }
                    else
                    {
                        drawW = avail.x;
                        drawH = avail.x / targetAspect;
                        offsetY = (avail.y - drawH) * 0.5f;
                    }
                }

                int texW = static_cast<int>(std::max(1.0f, drawW));
                int texH = static_cast<int>(std::max(1.0f, drawH));
                updateGameTexture(texW, texH);
                renderGameToTexture(state);

                ImVec2 imgPos = ImGui::GetCursorScreenPos();
                if (offsetX > 0.0f || offsetY > 0.0f)
                {
                    ImDrawList *drawList = ImGui::GetWindowDrawList();
                    drawList->AddRectFilled(imgPos, ImVec2(imgPos.x + avail.x, imgPos.y + avail.y), IM_COL32(10, 10, 12, 255));
                }

                ImGui::SetCursorScreenPos(ImVec2(imgPos.x + offsetX, imgPos.y + offsetY));
                ImGui::Image((ImTextureID)(intptr_t)m_gameTexture, ImVec2(drawW, drawH));

                // Top Toolbar Overlay in Game Viewport
                ImGui::SetCursorScreenPos(ImVec2(imgPos.x + 10.0f, imgPos.y + 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.15f, 0.85f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 4.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 3.0f));

                if (ImGui::BeginChild("GameControls", ImVec2(340.0f, 34.0f), true, ImGuiWindowFlags_NoScrollbar))
                {
                    const char *aspects[] = {"Free Aspect", "16:9", "16:10", "4:3", "1:1 Square"};
                    ImGui::SetNextItemWidth(120.0f);
                    ImGui::Combo("##AspectCombo", &m_aspectRatioOption, aspects, 5);
                    ImGui::SameLine();
                    ImGui::Checkbox("Stats", &m_showStats);
                    ImGui::EndChild();
                }

                // Stats HUD Overlay
                if (m_showStats)
                {
                    ImGui::SetCursorScreenPos(ImVec2(imgPos.x + 10.0f, imgPos.y + 50.0f));
                    if (ImGui::BeginChild("GameStats", ImVec2(220.0f, 70.0f), true, ImGuiWindowFlags_NoScrollbar))
                    {
                        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "FPS: %.1f", ImGui::GetIO().Framerate);
                        ImGui::Text("Objects: %zu", state.activeWorld().objects.size());
                        ImGui::Text("Resolution: %dx%d", texW, texH);
                        ImGui::EndChild();
                    }
                }

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
} // namespace RoninEditor
