#pragma once

#include "editor_state.h"
#include <string>

namespace RoninEditor
{
    class TerrainEditorPanel
    {
    public:
        TerrainEditorPanel();
        ~TerrainEditorPanel() = default;

        void Render(EditorState &state);

        bool IsOpen() const { return m_isOpen; }
        void SetOpen(bool open) { m_isOpen = open; }

    private:
        bool m_isOpen = true;
        int m_selectedObjId = -1;
        int m_activeTab = 0; // 0=Brush, 1=Fill, 2=Colliders, 3=Procedural, 4=Settings

        // Canvas navigation
        float m_canvasZoom = 1.0f;
        RoninEngine::Runtime::Vec2 m_canvasPan = {0.0f, 0.0f};
        bool m_isPanning = false;
        RoninEngine::Runtime::Vec2 m_panStartMouse = {0.0f, 0.0f};
        RoninEngine::Runtime::Vec2 m_panStartOffset = {0.0f, 0.0f};

        // Custom texture browse
        char m_customTextureBuf[256] = "Assets/Textures/Ground.png";

        void renderTargetSelector(EditorState &state, EditorObject *&outObj);
        void renderHeaderInfo(EditorState &state, EditorObject &obj);
        void renderToolbarAndTabs(EditorState &state, EditorObject &obj);
        void renderBrushPanel(EditorState &state, EditorObject &obj);
        void renderFillPanel(EditorState &state, EditorObject &obj);
        void renderCollidersPanel(EditorState &state, EditorObject &obj);
        void renderNavMeshPanel(EditorState &state, EditorObject &obj);
        void renderProceduralPanel(EditorState &state, EditorObject &obj);
        void renderSettingsPanel(EditorState &state, EditorObject &obj);
        void renderInteractiveCanvas(EditorState &state, EditorObject &obj);
    };
} // namespace RoninEditor
