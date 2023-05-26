#pragma once

#include "Types.h"

namespace RoninEngine
{
    namespace UI
    {
        class GUI;

        enum GUIControlPresents : std::uint8_t { _UC, RGUI_TEXT, RGUI_BUTTON, RGUI_EDIT, RGUI_HSLIDER, RGUI_VSLIDER, RGUI_IMAGEANIMATOR, RGUI_TEXTRAND, RGUI_IMAGE, RGUI_DROPDOWN };

        struct UIElement {
            Runtime::Rect rect;
            Runtime::Rect contextRect;
            std::uint8_t options;
            uid id;
            uid parentId;
            std::string text;
            GUIControlPresents prototype;
            std::list<uint8_t> childs;
            void* resources;
            void* event;
        };

        // main callback for elements
        typedef void (*ui_callback)(uid id, void* userdata);

        // events
        typedef void (*event_index_changed)(uid id, int selectedIndex);

        typedef void (*event_value_changed)(uid id, float value);

        extern void draw_font_at(SDL_Renderer* renderer, const std::string& text, int fontSize, Runtime::Vec2Int screenPoint, const Runtime::Color color, bool alignCenter = false);

    } // namespace UI
} // namespace RoninEngine
