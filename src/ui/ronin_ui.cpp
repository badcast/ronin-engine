#include "ronin.h"
#include "ronin_ui_resources.h"

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

    extern UIRes *factory_resource(GUIControlPresents type);
    extern void factory_free(UIElement *element);
    extern bool general_render_ui_section(GUI *gui, UIElement &element, const bool ms_hover, const bool ms_click, bool &ui_focus);
    extern void event_action(UIElement *element);

    extern void ui_reset_controls();

    uid call_register_ui(GUI *gui, uid parent = NOPARENT)
    {
        if(!gui->ElementContains(parent))
            throw ronin_ui_group_parent_error();
        UIElement &data = gui->handle->ui_layer.elements.emplace_back();
        data.id = gui->handle->ui_layer.elements.size() - 1;
        data.parentId = parent;
        data.options = ElementVisibleMask | ElementEnableMask;
        // add the child
        if(parent)
            gui->handle->ui_layer.elements.at(parent).childs.emplace_back(data.id);
        else
            gui->handle->ui_layer.layers.push_back(data.id);

        return data.id;
    }

    inline UIElement &call_get_element(GUIResources *resources, uid id)
    {
        return resources->ui_layer.elements[id];
    }

    inline UILayout *call_get_last_layout(GUIResources *resources)
    {
        if(resources->ui_layer.layouts.empty())
            return nullptr;
        return &resources->ui_layer.layouts.back();
    }

    GUI::GUI(World *world)
    {
        RoninMemory::alloc_self<GUIResources>(handle);
        handle->owner = world;
        handle->interactable = true;
        handle->visible = true;
        // push main component
        handle->ui_layer.elements.emplace_back();
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

        for(auto &iter : gui->handle->ui_layer.elements)
        {
            if(gui->IsGroup(iter.id))
                groups.push_back(iter.id);
        }

        return groups;
    }

    // public---------------------------------------

    bool GUI::ElementContains(uid id)
    {
        return handle->ui_layer.elements.size() >= id;
    }

    uid GUI::PushGroup(const Rect &rect)
    {
        uid id = call_register_ui(this);
        UIElement &data = call_get_element(this->handle, id);
        data.rect = rect;
        data.options |= ElementGroupMask;

        return id;
    }
    uid GUI::PushGroup()
    {
        return PushGroup(Rect::zero);
    }

    uid UILayout::pushElement(uid id)
    {
        UIElement &element = call_get_element(handle, id);
        this->elements.push_back(id);
        return id;
    }

    void UILayout::maketUpdate()
    {
        Rectf rect;
        for(uid id : elements)
        {
            UIElement *elem = &call_get_element(handle, id);
            elem->rect;
        }
    }

    void GUI::PushLayout(Rectf region, bool aspect)
    {
        if(aspect)
        {
            region.x = Math::Clamp01(region.x);
            region.y = Math::Clamp01(region.y);
            region.w = Math::Clamp01(region.w);
            region.h = Math::Clamp01(region.h);
        }
        UILayout &layout = handle->ui_layer.layouts.emplace_back();
        layout.handle = handle;
        layout.aspect = aspect;
        layout.region = region;
    }

    uid GUI::LayoutLabel(const std::string &text, int fontWidth)
    {
        UILayout *layout = call_get_last_layout(handle);
        if(layout == nullptr)
            throw RoninEngine::Exception::ronin_ui_layout_error();

        return layout->pushElement(PushLabel(text, Vec2Int::zero, fontWidth));
    }

    uid GUI::LayoutButton(const std::string &text)
    {
        UILayout *layout = call_get_last_layout(handle);
        if(layout == nullptr)
            throw RoninEngine::Exception::ronin_ui_layout_error();

        return layout->pushElement(PushButton(text, Vec2Int::zero));
    }

    uid GUI::PushLabel(const std::string &text, const Rect &rect, int fontWidth, uid parent)
    {
        // todo: fontWidth
        uid id = call_register_ui(this, parent);
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
        uid id = call_register_ui(this, parent);
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
        uid id = call_register_ui(this, parent);
        UIElement &element = call_get_element(this->handle, id);

        element.text = text;
        element.rect = rect;
        element.prototype = RGUI_TEXT_EDIT;
        return id;
    }

    uid GUI::PushPictureBox(Sprite *sprite, const Rect &rect, uid parent)
    {
        uid id = call_register_ui(this, parent);

        UIElement &data = call_get_element(this->handle, id);
        data.prototype = RGUI_PICTURE_BOX;
        data.rect = rect;
        data.resources = sprite;
        return id;
    }
    uid GUI::PushPictureBox(Sprite *sprite, const Vec2Int &point, uid parent)
    {
        return PushPictureBox(sprite, {point.x, point.y, sprite->width(), sprite->height()}, parent);
    }

    template <typename Container>
    uid internal_push_dropdown(
        GUI *guiInstance, const Container &container, int index, const Rect &rect, UIEventInteger changed, uid parent)
    {
        using T = typename std::iterator_traits<decltype(container.cbegin())>::value_type;

        uid id = call_register_ui(guiInstance, parent);
        UIElement &element = call_get_element(guiInstance->handle, id);
        element.prototype = RGUI_DROPDOWN;
        element.rect = rect;
        element.resources = factory_resource(element.prototype);
        element.event = (void *) changed;

        if(!element.resources)
            RoninSimulator::Kill();

        auto link = static_cast<std::pair<int, std::list<std::string>> *>(element.resources);
        link->first = index;

        for(auto iter = std::cbegin(container); iter != std::cend(container); ++iter)
        {
            if constexpr(std::is_same<T, std::string>())
            {
                link->second.emplace_back(*iter);
            }
            else
            {
                link->second.emplace_back(std::to_string(*iter));
            }
        }

        if(!link->second.empty())
        {
            auto iter = link->second.begin();
            std::advance(iter, Math::Min(static_cast<int>(link->second.size() - 1), index));
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
        uid id = call_register_ui(this, parent);
        UIElement &element = call_get_element(this->handle, id);
        element.prototype = RGUI_HSLIDER;
        element.rect = rect;
        element.resources = factory_resource(element.prototype);

        (*(float *) element.resources) = value;
        (*(float *) (static_cast<char *>(element.resources) + sizeof(float))) = min;
        (*(float *) (static_cast<char *>(element.resources) + sizeof(float) * 2)) = max;

        element.event = (void *) changed;
        return id;
    }

    // property --------------------------------------------------------------------------------------------------------

    void GUI::SetElementRect(uid id, const Rect &rect)
    {
        call_get_element(this->handle, id).rect = rect;
    }

    Rect GUI::GetElementRect(uid id)
    {
        return call_get_element(this->handle, id).rect;
    }

    void GUI::SetElementText(uid id, const std::string &text)
    {
        call_get_element(this->handle, id).text = text;
    }

    std::string GUI::GetElementText(uid id)
    {
        return call_get_element(this->handle, id).text;
    }

    void GUI::SetElementVisible(uid id, bool state)
    {
        call_get_element(this->handle, id).options =
            (call_get_element(this->handle, id).options & ~ElementVisibleMask) | (ElementVisibleMask * (state == true));
    }

    bool GUI::GetElementVisible(uid id)
    {
        return (call_get_element(this->handle, id).options & ElementVisibleMask) != 0;
    }

    void GUI::SetElementEnable(uid id, bool state)
    {
        call_get_element(this->handle, id).options =
            ((call_get_element(this->handle, id).options & ~ElementEnableMask)) | (ElementEnableMask * (state == true));
    }

    bool GUI::GetElementEnable(uid id)
    {
        return (call_get_element(this->handle, id).options & ElementEnableMask) != 0;
    }

    float GUI::SliderGetValue(uid id)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER)
        {
            throw ronin_uid_nocast_error();
        }

        return *static_cast<float *>(elem.resources);
    }

    void GUI::SliderSetPercentage(uid id, float percentage)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER)
        {
            throw ronin_uid_nocast_error();
        }

        reinterpret_cast<SliderResource *>(elem.resources)->stepPercentage = percentage;
    }

    float GUI::SliderGetPercentage(uid id)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER)
        {
            throw ronin_uid_nocast_error();
        }

        return reinterpret_cast<SliderResource *>(elem.resources)->stepPercentage;
    }

    bool GUI::ButtonClicked(uid id)
    {
        return handle->ui_layer.button_clicked.count(id) > 0;
    }

    void GUI::SliderSetValue(uid id, float value)
    {
        UIElement &elem = call_get_element(this->handle, id);

        if(elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER)
        {
            throw ronin_uid_nocast_error();
        }
        SliderResource *sliderResource = reinterpret_cast<SliderResource *>(elem.resources);
        sliderResource->value = Math::Clamp(value, sliderResource->min, sliderResource->max);
    }

    // grouping-----------------------------------------------------------------------------------------------------------

    bool GUI::IsGroup(uid id)
    {
        return (call_get_element(this->handle, id).options & ElementGroupMask) == ElementGroupMask;
    }

    bool GUI::GroupShowAsUnique(uid id) throw()
    {
        if(!IsGroup(id))
            return false;

        handle->ui_layer.layers.remove_if([this](auto v) { return IsGroup(v); });

        return GroupShow(id);
    }

    bool GUI::GroupShow(uid id) throw()
    {
        bool result = false;
        if(IsGroup(id))
        {
            auto iter = std::find_if(
                begin(handle->ui_layer.layers), end(handle->ui_layer.layers), std::bind(std::equal_to<uid>(), std::placeholders::_1, id));

            if(iter == std::end(handle->ui_layer.layers))
            {
                handle->ui_layer.layers.emplace_back(id);
                SetElementVisible(id, true);
                result = true;
            }
        }
        return result;
    }

    bool GUI::GroupClose(uid id) throw()
    {
        if(!IsGroup(id))
            return false;
        handle->ui_layer.layers.remove(id);
        SetElementVisible(id, false);
        return true;
    }

    bool GUI::IsMouseOver()
    {
        return handle->mouse_hover;
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
        for(int x = 0; x < handle->ui_layer.elements.size(); ++x)
        {
            factory_free(&(handle->ui_layer.elements[x]));
        }
        handle->ui_layer.layers.clear();
        handle->ui_layer.elements.clear();
    }

    bool GUI::FocusedGUI()
    {
        return handle->_focusedUI;
    }

    // It's main GUI drawer
    void native_draw_render(GUI *gui)
    {
        GUIResources *resources = gui->handle;

        if(resources->visible == false)
            return;

        uid id;
        UIElement *uielement;
        Vec2Int ms;
        std::deque<uid> ui_drains;
        bool ui_hover;
        bool ui_contex;
        bool ms_hover;
        bool ms_click;

        ms = Input::GetMousePoint();

        resources->_focusedUI = false;
        ui_reset_controls(); // Reset

        for(auto iter = begin(resources->ui_layer.layers); iter != end(resources->ui_layer.layers); ++iter)
        {
            if(gui->GetElementVisible(*iter))
                ui_drains.emplace_back(*iter);
        }
        ms_click = Input::GetMouseUp(MouseState::MouseLeft);

        gui->handle->mouse_hover = false;
        gui->handle->ui_layer.button_clicked.clear();
        // TODO: OPTIMIZE HERE
        while(ui_drains.empty() == false && !resources->owner->irs->request_unloading)
        {
            id = ui_drains.front();
            ui_drains.pop_front();
            uielement = &call_get_element(gui->handle, id);
            for(auto iter = begin(uielement->childs); iter != end(uielement->childs); ++iter)
                ui_drains.push_back(*iter);
            // status mouse hovered
            ms_hover = Vec2Int::HasIntersection(ms, uielement->rect);
            ui_contex = !uielement->contextRect.empty();

            if(!(uielement->options & ElementGroupMask) && uielement->options & ElementVisibleMask)
            {
                ui_hover = id == resources->ui_layer.focusedID;

                // unfocus on click an not hovered
                if(ui_hover && (!ms_hover || !ui_contex) && ms_click)
                {
                    ui_hover = false;
                    resources->ui_layer.focusedID = 0;
                }

                if((general_render_ui_section(gui, *uielement, ms_hover, ms_click && ms_hover, ui_hover)) && resources->interactable)
                {
                    // Избавляемся от перекликов в UI
                    resources->_focusedUI = true;

                    if(ui_hover)
                    {
                        resources->ui_layer.focusedID = id;
                    }

                    if(uielement->options & ElementEnableMask)
                    {
                        if(resources->callback)
                        {
                            // Отправка сообщения о действий.
                            resources->callback(id, resources->callbackData);
                        }
                        // run event
                        event_action(uielement);
                    }
                }
                else
                { // disabled state
                    // TODO: disabled state for UI element's
                    if(id == resources->ui_layer.focusedID && !ui_hover)
                        resources->ui_layer.focusedID = 0;
                }
                if(!resources->_focusedUI)
                    resources->_focusedUI = ui_hover;
            }

            if(ms_hover)
                gui->handle->mouse_hover = ms_hover;
        }
    }

} // namespace RoninEngine::UI
