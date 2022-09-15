#pragma once

#include "Types.h"
#include "dependency.h"

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

void InitalizeControls();
void Free_Controls();
void ResetControls();

}  // namespace UI
}  // namespace RoninEngine
