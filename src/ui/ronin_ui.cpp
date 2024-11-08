#include "ronin.h"
#include "ronin_ui_resources.h"
#include "ronin_debug.h"

enum
{
    ElementEnableMask = 1,
    ElementVisibleMask = 2,
    ElementGroupMask = 4
};

namespace RoninEngine::UI
{
    using namespace RoninEngine::Runtime;
    using namespace RoninEngine::Exception;

    extern bool general_ui_render(UIElement &element, const UIState &uiState, bool &ui_focus);
    extern void general_ui_event_action(UIElement *element);

    uid push_new_element(GUI *gui, uid parent = NOPARENT)
    {
        if(!gui->ElementContains(parent))
            throw ronin_ui_group_parent_error();

        UIElement &elem = gui->handle->elements.emplace_back();
        elem.id = gui->handle->elements.size() - 1;
        elem.parentId = parent;
        elem.options = ElementVisibleMask | ElementEnableMask;

        // Clean memory
        memset(&elem.resource, 0, sizeof(elem.resource));

        // add the child
        if(parent)
            gui->handle->elements.at(parent).childs.emplace_back(elem.id);
        else
            gui->handle->layers.push_back(elem.id);

        return elem.id;
    }

    inline UIElement &call_get_element(GUIResources *resources, uid id)
    {
        return resources->elements.at(id);
    }

    inline UILayout *call_get_last_layout(GUIResources *resources)
    {
        if(resources->layouts.empty())
            return nullptr;
        return &resources->layouts.back();
    }

    void ui_resource_new(UIElement &element)
    {
        switch(element.prototype)
        {
            case GUIControlPresent::RGUI_DROPDOWN:
            {
                RoninMemory::alloc_self(element.resource.dropdown);
                break;
            }
        }
    }

    void ui_resource_del(UIElement &element)
    {
        switch(element.prototype)
        {
            case GUIControlPresent::RGUI_DROPDOWN:
                RoninMemory::free(element.resource.dropdown);
                break;
            case RGUI_TEXT:
            case RGUI_BUTTON:
            case RGUI_TEXT_EDIT:
            case RGUI_HSLIDER:
            case RGUI_VSLIDER:
            case RGUI_PICTURE_BOX:
            case RGUI_CHECKBOX:
            default:
                break;
        }
    }

    GUI::GUI(World *world)
    {
        RoninMemory::alloc_self<GUIResources>(handle);
        handle->owner = world;
        handle->interactable = true;
        handle->visible = true;
        // push main component
        handle->elements.emplace_back();
    }

    GUI::~GUI()
    {
        PopAllElements();
        RoninMemory::free(handle);
    }

    // private--------------------------------------

    std::list<uid> get_groups(GUI *gui)
    {
        std::list<uid> groups;

        for(auto &iter : gui->handle->elements)
        {
            if(gui->IsGroup(iter.id))
                groups.push_back(iter.id);
        }

        return groups;
    }

    // public---------------------------------------

    bool GUI::ElementContains(uid id)
    {
        return handle->elements.size() >= id;
    }

    uid GUI::PushGroup(const Rect &rect)
    {
        uid id = push_new_element(this);
        UIElement &data = call_get_element(this->handle, id);
        data.rect = rect;
        data.options |= ElementGroupMask;

        return id;
    }

    uid GUI::PushGroup()
    {
        return PushGroup(Rect::zero);
    }

    uid GUI::PushCustomUI(const UIOverlay *custom, const Rect &rect, uid parent)
    {
        if(custom == nullptr)
        {
            ronin_log("overlay can not be null");
            return -1;
        }

        uid id = push_new_element(this, parent);
        UIElement &data = call_get_element(this->handle, id);
        data.id = id;
        data.rect = rect;
        data.prototype = RGUI_CUSTOM_OVERLAY;
        data.resource.overlay.inspector = const_cast<UIOverlay *>(custom);
        return id;
    }

    void GUI::LayoutNew(UILayoutDirection direction, Rectf region, bool aspectRatio)
    {
        UILayout &layout = handle->layouts.emplace_back();
        layout.direction = direction;
        layout.handle = handle;
        layout.aspectRatio = aspectRatio;
        if(aspectRatio)
        {
            layout.region.x = Math::Clamp01(region.x);
            layout.region.y = Math::Clamp01(region.y);
            layout.region.w = Math::Clamp01(region.w);
            layout.region.h = Math::Clamp01(region.h);
        }
        else
        {
            layout.region = region;
        }
    }

    uid GUI::LayoutLabel(const std::string &text)
    {
        UILayout *layout = call_get_last_layout(handle);
        if(layout == nullptr)
            throw RoninEngine::Exception::ronin_ui_layout_error();

        return layout->addElement(PushLabel(text, Vec2Int::zero));
    }

    uid GUI::LayoutButton(const std::string &text)
    {
        UILayout *layout = call_get_last_layout(handle);
        if(layout == nullptr)
            throw RoninEngine::Exception::ronin_ui_layout_error();

        return layout->addElement(PushButton(text, Vec2Int::zero));
    }

    uid GUI::PushLabel(const std::string &text, const Rect &rect, int fontWidth, uid parent)
    {
        // TODO: fontWidth - for set Font size
        uid id = push_new_element(this, parent);
        UIElement &data = call_get_element(this->handle, id);
        data.text = text;
        data.rect = rect;
        data.prototype = RGUI_TEXT;
        data.id = id;
        return id;
    }

    uid GUI::PushLabel(const std::string &text, const Vec2Int &point, int fontWidth, uid parent)
    {
        return PushLabel(text, {point.x, point.y, 0, 0}, fontWidth, parent);
    }

    uid GUI::PushButton(const std::string &text, const Vec2Int &point, UIEventVoid event_callback, uid parent)
    {
        return PushButton(text, {point.x, point.y, defaultMakets.buttonSize.x, defaultMakets.buttonSize.y}, event_callback, parent);
    }

    uid GUI::PushButton(const std::string &text, const Rect &rect, RoninEngine::UI::UIEventVoid event_callback, uid parent)
    {
        uid id = push_new_element(this, parent);
        UIElement &data = call_get_element(this->handle, id);
        data.rect = rect;
        data.text = text;
        data.prototype = RGUI_BUTTON;
        data.event = reinterpret_cast<void *>(event_callback);

        return id;
    }

    uid GUI::PushTextEdit(const std::string &text, const Vec2Int &point, uid parent)
    {
        return PushTextEdit(text, {point.x, point.y, defaultMakets.editSize.x, defaultMakets.editSize.y}, parent);
    }
    uid GUI::PushTextEdit(const std::string &text, const Rect &rect, uid parent)
    {
        uid id = push_new_element(this, parent);
        UIElement &element = call_get_element(this->handle, id);

        element.text = text;
        element.rect = rect;
        element.prototype = RGUI_TEXT_EDIT;
        return id;
    }

    uid GUI::PushPictureBox(SpriteRef sprite, const Rect &rect, uid parent)
    {
        uid id = push_new_element(this, parent);

        UIElement &data = call_get_element(this->handle, id);
        data.prototype = RGUI_PICTURE_BOX;
        data.rect = rect;
        data.resource.picturebox = sprite.ptr_;
        return id;
    }
    uid GUI::PushPictureBox(SpriteRef sprite, const Vec2Int &point, uid parent)
    {
        return PushPictureBox(sprite, {point.x, point.y, sprite->width(), sprite->height()}, parent);
    }

    template <typename Container>
    uid internal_push_dropdown(GUI *guiInstance, const Container &container, int index, const Rect &rect, UIEventInteger changed, uid parent)
    {
        using T = typename std::iterator_traits<decltype(container.cbegin())>::value_type;

        uid id = push_new_element(guiInstance, parent);
        UIElement &element = call_get_element(guiInstance->handle, id);
        element.prototype = RGUI_DROPDOWN;
        element.rect = rect;
        element.event = (void *) changed;

        // Init UI Element
        ui_resource_new(element);

        element.resource.dropdown->first = index;

        for(auto iter = std::cbegin(container); iter != std::cend(container); ++iter)
        {
            if constexpr(std::is_same<T, std::string>())
            {
                element.resource.dropdown->second.emplace_back(*iter);
            }
            else
            {
                element.resource.dropdown->second.emplace_back(std::to_string(*iter));
            }
        }

        if(!element.resource.dropdown->second.empty())
        {
            auto iter = element.resource.dropdown->second.begin();
            std::advance(iter, Math::Min(static_cast<int>(element.resource.dropdown->second.size() - 1), index));
            element.text = *iter;
        }

        return id;
    }

    uid GUI::PushDropDown(const std::list<std::string> &elements, int index, const Rect &rect, UIEventInteger changed, uid parent)
    {
        return internal_push_dropdown(this, elements, index, rect, changed, parent);
    }

    uid GUI::PushSlider(float value, float min, float max, const Rect &rect, UIEventFloat changed, uid parent)
    {
        uid id = push_new_element(this, parent);
        UIElement &element = call_get_element(this->handle, id);
        element.prototype = RGUI_HSLIDER;
        element.rect = rect;
        element.event = (void *) changed;
        element.resource.slider.value = value;
        element.resource.slider.min = min;
        element.resource.slider.max = max;
        element.resource.slider.stepPercentage = 0.1f;
        return id;
    }

    uid GUI::PushCheckBox(bool checked, const std::string &text, const Runtime::Rect &rect, UIEventBool changed, uid parent)
    {
        uid id = push_new_element(this, parent);

        UIElement &element = call_get_element(this->handle, id);
        element.prototype = RGUI_CHECKBOX;
        element.rect = rect;
        element.event = (void *) changed;
        element.text = text;
        element.resource.checkbox = checked;

        return id;
    }

    uid GUI::PushSpriteButton(const std::vector<SpriteRef > &states, const Runtime::Rect &rect, UIEventVoid click, uid parent)
    {
        uid id = push_new_element(this, parent);

        UIElement &element = call_get_element(this->handle, id);
        element.prototype = RGUI_SPRITE_BUTTON;
        element.rect = rect;
        element.event = (void *) click;
        memset(&element.resource.spriteButton, 0, sizeof(element.resource.spriteButton));

        int n = states.size() - 1;
        SpriteRef src;
        if(n > -1)
        {

            if(n >= 2)
                src = states[2];
            else
                src = states[0];

            element.resource.spriteButton.disable = src.ptr_;

            if(n >= 1)
                src = states[1];
            else
                src = states[0];

            element.resource.spriteButton.hover = src.ptr_;

            element.resource.spriteButton.normal = states[0].ptr_;
        }
        return id;
    }

    void GUI::SpriteButtonSetIcon(uid id, SpriteRef icon)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_SPRITE_BUTTON)
        {
            throw ronin_uid_nocast_error();
        }

        elem.resource.spriteButton.icon = icon.ptr_;
    }

    // property --------------------------------------------------------------------------------------------------------

    void GUI::ElementSetRect(uid id, const Rect &rect)
    {
        call_get_element(this->handle, id).rect = rect;
    }

    Rect GUI::ElementGetRect(uid id)
    {
        return call_get_element(this->handle, id).rect;
    }

    void GUI::ElementSetText(uid id, const std::string &text)
    {
        call_get_element(this->handle, id).text = text;
    }

    std::string GUI::ElementGetText(uid id)
    {
        return call_get_element(this->handle, id).text;
    }

    void GUI::ElementSetVisible(uid id, bool state)
    {
        call_get_element(this->handle, id).options = (call_get_element(this->handle, id).options & ~ElementVisibleMask) | (ElementVisibleMask * (state == true));
    }

    bool GUI::ElementGetVisible(uid id)
    {
        return (call_get_element(this->handle, id).options & ElementVisibleMask) != 0;
    }

    void GUI::ElementSetEnable(uid id, bool state)
    {
        call_get_element(this->handle, id).options = ((call_get_element(this->handle, id).options & ~ElementEnableMask)) | (ElementEnableMask * (state == true));
    }

    bool GUI::ElementGetEnable(uid id)
    {
        return (call_get_element(this->handle, id).options & ElementEnableMask) != 0;
    }

    bool GUI::CheckBoxGetValue(uid id)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_CHECKBOX)
        {
            throw ronin_uid_nocast_error();
        }
        return elem.resource.checkbox;
    }

    void GUI::CheckBoxSetValue(uid id, bool value)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_CHECKBOX)
        {
            throw ronin_uid_nocast_error();
        }
        elem.resource.checkbox = value;
    }

    void GUI::SliderSetPercentage(uid id, float percentage)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER)
        {
            throw ronin_uid_nocast_error();
        }

        elem.resource.slider.stepPercentage = percentage;
    }

    float GUI::SliderGetPercentage(uid id)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER)
        {
            throw ronin_uid_nocast_error();
        }

        return elem.resource.slider.stepPercentage;
    }

    bool GUI::ButtonClicked(uid id)
    {
        return handle->button_clicked.count(id) > 0;
    }

    void GUI::PictureBoxSetSprite(uid id, Runtime::SpriteRef sprite)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_PICTURE_BOX)
        {
            throw ronin_uid_nocast_error();
        }

        elem.resource.picturebox = sprite.ptr_;
    }

    Runtime::SpriteRef GUI::PictureBoxGetSprite(uid id)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_PICTURE_BOX)
        {
            throw ronin_uid_nocast_error();
        }

        return elem.resource.picturebox->GetRef<Sprite>();
    }

    void GUI::SliderSetValue(uid id, float value)
    {
        if(!ElementContains(id))
            throw RoninEngine::Exception::ronin_out_of_range_error();

        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER)
        {
            throw ronin_uid_nocast_error();
        }
        elem.resource.slider.value = Math::Clamp(value, elem.resource.slider.min, elem.resource.slider.max);
    }

    float GUI::SliderGetValue(uid id)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER)
        {
            throw ronin_uid_nocast_error();
        }

        return elem.resource.slider.value;
    }

    // grouping-----------------------------------------------------------------------------------------------------------

    bool GUI::IsGroup(uid id)
    {
        if(!ElementContains(id))
            throw RoninEngine::Exception::ronin_out_of_range_error();

        return (call_get_element(this->handle, id).options & ElementGroupMask) == ElementGroupMask;
    }

    bool GUI::GroupShowAsUnique(uid id) throw()
    {
        if(!ElementContains(id))
            throw RoninEngine::Exception::ronin_out_of_range_error();

        if(!IsGroup(id))
            return false;

        handle->layers.remove_if([this](auto v) { return IsGroup(v); });

        return GroupShow(id);
    }

    bool GUI::GroupShow(uid id) throw()
    {
        bool result = false;
        if(IsGroup(id))
        {
            auto iter = std::find_if(begin(handle->layers), end(handle->layers), std::bind(std::equal_to<uid>(), std::placeholders::_1, id));

            if(iter == std::end(handle->layers))
            {
                handle->layers.emplace_back(id);
                ElementSetVisible(id, true);
                result = true;
            }
        }
        return result;
    }

    bool GUI::GroupClose(uid id) throw()
    {
        if(!IsGroup(id))
            return false;
        handle->layers.remove(id);
        ElementSetVisible(id, false);
        return true;
    }

    bool GUI::IsMouseOver()
    {
        return handle->any_mouse_hover;
    }

    void GUI::SetInteractable(bool state)
    {
        handle->interactable = state;
    }
    bool GUI::GetInteractable()
    {
        return handle->interactable;
    }

    void GUI::SetGeneralCallback(UIEventUserData callback, void *userData)
    {
        handle->callback = callback;
        handle->callbackData = userData;
    }

    bool GUI::PopElement(uid id)
    {
        // TODO: Make Pop (Remove) element
        return false;
    }

    void GUI::PopAllElements()
    {
        for(UIElement &elem : handle->elements)
        {
            ui_resource_del(elem);
        }
        handle->layers.clear();
        handle->elements.clear();
    }

    bool GUI::FocusedGUI()
    {
        return handle->_focusedUI;
    }

    uid UILayout::addElement(uid id)
    {
        auto iter = std::find(std::begin(elements), std::end(elements), id);
        if(iter != std::end(elements))
            return -1;

        UIElement &element = call_get_element(handle, id);
        this->elements.push_back(id);
        maketUpdate();
        return id;
    }

    void UILayout::maketUpdate()
    {
        Rectf _reg = region;
        for(uid id : elements)
        {
            UIElement *elem = &call_get_element(handle, id);

            elem->rect.x = _reg.x;
            elem->rect.y = _reg.y;

            switch(direction)
            {
                case UILayoutDirection::Horizontal:
                    _reg.x += elem->rect.w;
                    break;
                case UILayoutDirection::Vertical:
                    _reg.y += elem->rect.h;
                    break;
                default:;
            }
        }
    }

    void GUI::AddEventListener_Click(uid id, UIEventVoid event)
    {
        if(!ElementContains(id))
            throw RoninEngine::Exception::ronin_out_of_range_error();

        UIElement &data = call_get_element(handle, id);
        data.event = (void *) event;
    }

    // It's main GUI drawer
    void native_draw_render(GUI *gui)
    {
        GUIResources *handle = gui->handle;

        if(handle->visible == false)
            return;

        uid id;
        std::deque<uid> ui_drains;

        UIElement *uiElement;
        UIState uiState;

        Vec2Int ms;
        bool ui_msclick, ui_hover, ui_contex;

        ms = Input::GetMousePoint();

        uiState.gui = gui;
        uiState.ms = Input::GetMousePoint();

        handle->_focusedUI = false;
        ui_msclick = Input::GetMouseUp(MouseButton::MouseLeft);

        for(auto iter = begin(handle->layers); iter != end(handle->layers); ++iter)
        {
            if(gui->ElementGetVisible(*iter))
                ui_drains.emplace_back(*iter);
        }

        gui->handle->any_mouse_hover = false;
        gui->handle->button_clicked.clear();
        // TODO: OPTIMIZE HERE
        while(!ui_drains.empty() && !handle->owner->irs->requestUnloading)
        {
            id = ui_drains.front();
            ui_drains.pop_front();

            uiElement = &call_get_element(gui->handle, id);
            for(auto iter = begin(uiElement->childs); iter != end(uiElement->childs); ++iter)
                ui_drains.push_back(*iter);

            // status mouse hovered
            uiState.ms_hover = Vec2Int::HasIntersection(ms, uiElement->rect);
            ui_contex = !uiElement->contextRect.empty();

            if(uiState.ms_hover)
                gui->handle->any_mouse_hover = uiState.ms_hover;

            if(!(uiElement->options & ElementGroupMask) && (uiElement->options & ElementVisibleMask))
            {
                ui_hover = id == handle->focusedID;

                // unfocus on click an not hovered
                if(ui_hover && (!uiState.ms_hover || !ui_contex) && uiState.ms_click)
                {
                    ui_hover = false;
                    handle->focusedID = 0;
                }

                // Refresh UIState
                uiState.ms_click = ui_msclick && uiState.ms_hover;

                // Draw Internal GUI
                if((general_ui_render(*uiElement, uiState, ui_hover)) && handle->interactable)
                {
                    // Избавляемся от перекликов в UI
                    handle->_focusedUI = true;

                    if(ui_hover)
                    {
                        handle->focusedID = id;
                    }

                    if(uiElement->options & ElementEnableMask)
                    {
                        if(handle->callback)
                        {
                            // Send callback of the General UI Event
                            handle->callback(id, handle->callbackData);
                        }
                        // run event
                        general_ui_event_action(uiElement);
                    }
                }
                else
                { // disabled state
                    // TODO: disabled state for UI element's
                    if(id == handle->focusedID && !ui_hover)
                        handle->focusedID = 0;
                }
                if(!handle->_focusedUI)
                    handle->_focusedUI = ui_hover;
            }
        }
    }

} // namespace RoninEngine::UI
