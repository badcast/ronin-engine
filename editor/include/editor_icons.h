#pragma once

#include <imgui.h>
#include <string>

namespace RoninEditor
{
    namespace EditorPalette
    {
        // Standardized unified color palette
        constexpr ImU32 Primary       = IM_COL32(58, 120, 218, 255);  // #3A78DA
        constexpr ImU32 PrimaryHover  = IM_COL32(80, 145, 235, 255);
        constexpr ImU32 PrimaryActive = IM_COL32(40, 95, 180, 255);

        constexpr ImU32 Success       = IM_COL32(56, 175, 95, 255);   // Green
        constexpr ImU32 Warning       = IM_COL32(230, 155, 45, 255);  // Amber
        constexpr ImU32 Danger        = IM_COL32(220, 60, 60, 255);   // Red

        constexpr ImU32 TextMuted     = IM_COL32(130, 135, 148, 255);
        constexpr ImU32 TextNormal    = IM_COL32(210, 215, 225, 255);
        constexpr ImU32 TextWhite     = IM_COL32(255, 255, 255, 255);

        constexpr ImU32 IconFolder    = IM_COL32(225, 185, 75, 255);  // Amber folder
        constexpr ImU32 IconWorld     = IM_COL32(65, 175, 240, 255);  // Cyan world
        constexpr ImU32 IconSprite    = IM_COL32(165, 115, 235, 255); // Purple sprite
        constexpr ImU32 IconAudio     = IM_COL32(240, 105, 140, 255); // Rose audio
        constexpr ImU32 IconScript    = IM_COL32(80, 205, 140, 255);  // Emerald code
        constexpr ImU32 IconCamera    = IM_COL32(75, 185, 235, 255);  // Sky camera
        constexpr ImU32 IconLight     = IM_COL32(255, 220, 90, 255);  // Warm sun light
    }

    enum class IconType
    {
        Play,
        Pause,
        Stop,
        Step,
        Move,
        Rotate,
        Scale,
        Terrain,
        Reset,
        Folder,
        File,
        World,
        Sprite,
        Audio,
        Camera,
        Light,
        Search,
        Delete,
        Plus
    };

    class EditorIcons
    {
    public:
        // Render a pure vector icon onto an ImDrawList within the given bounds
        static void DrawIcon(ImDrawList *drawList, const ImVec2 &pMin, const ImVec2 &pMax, IconType icon, ImU32 color = EditorPalette::TextNormal);

        // Render an interactive icon button with hover and active states
        static bool IconButton(const char *id, IconType icon, const ImVec2 &size = ImVec2(24, 24), bool active = false, ImU32 iconColor = EditorPalette::TextNormal);

        // Render an icon followed by text on the same line
        static void IconLabel(IconType icon, const char *label, ImU32 iconColor = EditorPalette::TextNormal, float iconSize = 14.0f);
    };
} // namespace RoninEditor
