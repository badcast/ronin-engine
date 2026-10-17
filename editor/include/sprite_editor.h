#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <ronin/framework.h>
#include "editor_state.h"

namespace RoninEditor
{
    struct SpriteMetadata
    {
        std::string name = "NewSprite";
        std::string texturePath; // Relative to project root / Assets
        RoninEngine::Runtime::Rect rect = {0, 0, 64, 64}; // Pixel rect inside texture
        RoninEngine::Runtime::Vec2 pivot = {0.5f, 0.5f};  // Normalized pivot [0..1]
        float pixelsPerUnit = 100.0f;
        int borderLeft = 0;
        int borderBottom = 0;
        int borderRight = 0;
        int borderTop = 0;
    };

    class SpriteEditor
    {
    public:
        SpriteEditor();
        ~SpriteEditor();

        void Init(SDL_Renderer *renderer);
        void Shutdown();

        void OpenAsset(const std::filesystem::path &filePath);
        void Render(EditorState &state);

        bool IsOpen() const { return m_isOpen; }
        void SetOpen(bool open) { m_isOpen = open; }

    private:
        void loadTexture(const std::filesystem::path &imagePath);
        void freeTexture();
        bool loadSpriteJson(const std::filesystem::path &jsonPath);
        bool saveSpriteJson(const std::filesystem::path &jsonPath);

        void autoTrim();
        void autoSliceGrid(int cellW, int cellH, EditorState &state);

        void renderCanvas(EditorState &state);
        void renderSidebar(EditorState &state);
        void renderTexturePickerModal(EditorState &state);

        SDL_Renderer *m_renderer = nullptr;
        SDL_Texture *m_texture = nullptr;
        SDL_Surface *m_surface = nullptr;
        int m_texWidth = 0;
        int m_texHeight = 0;

        SpriteMetadata m_meta;
        std::filesystem::path m_currentFile; // .sprite or texture path
        bool m_isDirty = false;
        bool m_isOpen = true;

        // Canvas navigation
        RoninEngine::Runtime::Vec2 m_pan = {0.0f, 0.0f};
        float m_zoom = 1.0f;
        int m_dragMode = 0; // 0=None, 1=MoveRect, 2=TopLeft, 3=TopRight, 4=BottomRight, 5=BottomLeft, 6=Pivot

        // Slicing tool UI state
        int m_gridSliceW = 32;
        int m_gridSliceH = 32;
        int m_pivotPreset = 0; // 0=Center, 1=TopLeft, 2=Top, 3=TopRight, 4=Left, 5=Right, 6=BottomLeft, 7=Bottom, 8=BottomRight, 9=Custom

        // Texture Picker modal
        bool m_showTexturePicker = false;
        bool m_openTexturePickerTrigger = false;
        std::filesystem::path m_pendingTexturePath;
        char m_pickerFilter[128] = "";
    };
} // namespace RoninEditor
