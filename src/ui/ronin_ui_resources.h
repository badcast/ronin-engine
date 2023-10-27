#ifndef _RONIN_UI_RESOURCES_H_
#define _RONIN_UI_RESOURCES_H_

#include "ronin.h"

namespace RoninEngine::UI
{
    using namespace RoninEngine::Runtime;

    struct SliderResource
    {
        float value;
        float min;
        float max;
    };

    typedef std::pair<int, std::list<std::string>> DropDownResource;

    struct ElementInteraction
    {
        Color normalState;
        Color hoverState;
        Color pressState;
        Color disabledState;
    };

    struct
    {
        ElementInteraction defaultInteraction = {Color::ghostwhite, Color::white, Color::gainsboro, Color::darkgray};

        Color dropdownText = Color::black;
        Color dropdownSelectedText = Color::lightslategrey;
        Color editText = Color::black;
        Color buttonText = Color::black;
        Color buttonDownSide = Color::darkgray;
    } colorSpace;

    // It's default resource for UI
    struct UIRes;

    enum GUIControlPresents
    {
        RGUI_TEXT,
        RGUI_BUTTON,
        RGUI_TEXT_EDIT,
        RGUI_HSLIDER,
        RGUI_VSLIDER,
        RGUI_PICTURE_BOX,
        RGUI_DROPDOWN
    };

    struct UIElement
    {
        uid id;
        uid parentId;
        std::uint8_t options;
        std::vector<uid> childs;
        Runtime::Rect rect;
        Runtime::Rect contextRect;
        GUIControlPresents prototype;
        std::string text;
        void *resources;
        void *event;
    };

    struct UILayout
    {
        bool aspect;
        GUIResources *handle;
        Runtime::Rectf region;
        std::vector<uid> elements;

        uid pushElement(uid id);
        void maketUpdate();
    };

    struct GUIResources
    {
        void *callbackData;
        bool interactable;
        bool _focusedUI;
        bool visible;
        bool mouse_hover;

        struct
        {
            uid focusedID;
            std::vector<UIElement> elements;
            std::list<uid> layers;
            std::vector<UILayout> layouts;
            std::set<uid> button_clicked;
        } ui_layer;

        Runtime::World *owner;
        UIEventUserData callback;
    };

} // namespace RoninEngine::UI
#endif