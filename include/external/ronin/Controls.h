#pragma once

#include "Types.h"
#include "begin.h"

namespace RoninEngine {
namespace UI {
class GUI;

// TODO: create CTEXT, CBUTTON, CEDIT, CHSLIDER, CVSLIDER, CIMAGEANIMATOR, CTEXTRAND, CIMAGE, CDROPDOWN

enum ControlType : std::uint8_t {
    _UC,
    CTEXT,
    CBUTTON,
    CEDIT,
    CHSLIDER,
    CVSLIDER,
    CIMAGEANIMATOR,
    CTEXTRAND,
    CIMAGE,
    CDROPDOWN
};

struct UIElement {
    Runtime::Rect rect;
    Runtime::Rect contextRect;
    std::uint8_t options;
    uid id;
    uid parentId;
    std::string text;
    ControlType prototype;
    std::list<uint8_t> childs;
    void* resources;
    void * event;
};

//main callback for elements
typedef void (*ui_callback)(uid id, void* userdata);

//events
typedef void (*event_index_changed)(uid id, int selectedIndex);

typedef void (*event_value_changed)(uid id, float value);

extern void InitalizeControls();
extern void Free_Controls();
extern void ResetControls();

extern void DrawFontAt(SDL_Renderer* renderer, const std::string &text, int fontSize, const Runtime::Vec2Int &screenPoint);

}  // namespace UI
}  // namespace RoninEngine
