#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include "editor_state.h"

namespace RoninEditor
{
    class SceneViewport
    {
    public:
        SceneViewport();
        ~SceneViewport();

        void Init(SDL_Renderer *renderer);
        void Shutdown();

        // Renders Scene Viewport window in Dear ImGui
        void RenderScene(EditorState &state);

        // Renders Game Viewport window in Dear ImGui
        void RenderGame(EditorState &state);

        // Transform helpers
        RoninEngine::Runtime::Vec2 WorldToScreen(
            const RoninEngine::Runtime::Vec2 &worldPos,
            const RoninEngine::Runtime::Vec2 &viewCenter,
            const RoninEngine::Runtime::Vec2 &viewSize) const;

        RoninEngine::Runtime::Vec2 ScreenToWorld(
            const RoninEngine::Runtime::Vec2 &screenPos,
            const RoninEngine::Runtime::Vec2 &viewCenter,
            const RoninEngine::Runtime::Vec2 &viewSize) const;

    private:
        void updateSceneTexture(int width, int height);
        void updateGameTexture(int width, int height);

        void renderSceneToTexture(EditorState &state);
        void renderGameToTexture(EditorState &state);

        void handleSceneInput(EditorState &state, const RoninEngine::Runtime::Vec2 &viewportPos, const RoninEngine::Runtime::Vec2 &viewportSize);

        SDL_Renderer *m_renderer = nullptr;
        SDL_Texture *m_sceneTexture = nullptr;
        SDL_Texture *m_gameTexture = nullptr;

        int m_sceneWidth = 0;
        int m_sceneHeight = 0;
        int m_gameWidth = 0;
        int m_gameHeight = 0;

        // Editor Scene Camera
        RoninEngine::Runtime::Vec2 m_cameraPos = {0.0f, 0.0f};
        float m_zoom = 1.0f;
        float m_pixelsPerUnit = 64.0f;

        // Gizmo Dragging State
        bool m_isDraggingGizmo = false;
        int m_dragGizmoAxis = 0; // 0=None, 1=X (Red), 2=Y (Green), 3=Center/Free, 4=Rotate, 5=Scale
        RoninEngine::Runtime::Vec2 m_dragStartMouse = {0.0f, 0.0f};
        RoninEngine::Runtime::Vec2 m_dragStartWorldPos = {0.0f, 0.0f};
        float m_dragStartAngle = 0.0f;
        RoninEngine::Runtime::Vec2 m_dragStartScale = {1.0f, 1.0f};

        // Panning State
        bool m_isPanning = false;
        RoninEngine::Runtime::Vec2 m_panStartMouse = {0.0f, 0.0f};
        RoninEngine::Runtime::Vec2 m_panStartCamera = {0.0f, 0.0f};

        // GameView options
        int m_aspectRatioOption = 0; // 0=Free Aspect, 1=16:9, 2=16:10, 3=4:3
        bool m_showStats = true;

        // Texture Cache for Sprite rendering
        SDL_Texture *getOrLoadTexture(const std::string &relOrAbsPath, SDL_Rect *outSrcRect, bool *hasSrcRect);
        std::unordered_map<std::string, SDL_Texture *> m_textureCache;
    };
} // namespace RoninEditor
