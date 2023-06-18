#include <deque>
#include "ronin.h"

enum { ElementEnableMask = 1, ElementVisibleMask = 2, ElementGroupMask = 4 };

namespace RoninEngine::UI
{
    using namespace RoninEngine::Exception;
    using namespace RoninEngine::Runtime;

    extern ui_resource* factory_resource(GUIControlPresents type);
    extern void factory_free(UIElement* element);
    extern bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* renderer, const bool ms_hover, const bool ms_click, bool& ui_focus);
    extern void event_action(UIElement* element);

    extern void ui_controls_init();
    extern void ui_free_controls();
    extern void ui_reset_controls();

    uid call_register_ui(GUI* gui, uid parent = NOPARENT)
    {
        if (!gui->has_ui(parent))
            throw ronin_ui_group_parent_error();

        UIElement& data = gui->resources->ui_layer.elements.emplace_back();
        data.parentId = parent;
        data.options = ElementVisibleMask | ElementEnableMask;
        data.id = gui->resources->ui_layer.elements.size();
        // add the child
        if (parent)
            gui->resources->ui_layer.elements[parent - 1].childs.emplace_back(gui->resources->ui_layer.elements.size() - 1);
        else
            gui->resources->ui_layer.layers.push_back(data.id);

        return data.id;
    }
    inline UIElement& call_get_element(GUI* gui, uid id) { return gui->resources->ui_layer.elements[id - 1]; }

    GUI::GUI(RoninEngine::Runtime::World* world)
    {
        Runtime::RoninMemory::alloc_self<GUIResources>(resources);
        resources->__level_owner = world;
        resources->hitCast = true;
        resources->visible = true;
        // resources->callback = nullptr;
        // resources->callbackData = nullptr;
        // resources->_focusedUI = false;
    };

    GUI::~GUI()
    {
        remove_all();
        Runtime::RoninMemory::free(resources);
    }

    // private--------------------------------------

    std::list<uid> get_groups(GUI* gui)
    {
        std::list<uid> groups;

        for (auto& iter : gui->resources->ui_layer.elements) {
            if (gui->is_group(iter.id))
                groups.push_back(iter.id);
        };

        return groups;
    }

    // public---------------------------------------

    bool GUI::has_ui(uid id) { return resources->ui_layer.elements.size() >= id; }

    uid GUI::push_group(const Runtime::Rect& rect)
    {
        uid id = call_register_ui(this);
        UIElement& data = call_get_element(this, id);
        data.rect = rect;
        data.options |= ElementGroupMask;

        return id;
    }
    uid GUI::push_group() { return push_group(Rect::zero); }

    uid GUI::begin_layment(const Runtime::Rect region) { }

    uid GUI::push_label(const std::string& text, const Rect& rect, const int& fontWidth, uid parent)
    {
        // todo: fontWidth
        uid id = call_register_ui(this, parent);
        UIElement& data = call_get_element(this, id);
        data.text = text;
        data.rect = rect;
        data.prototype = RGUI_TEXT;
        data.id = id;
        return id;
    }
    uid GUI::push_label(const std::string& text, const Vec2Int& point, const int& fontWidth, uid parent) { return push_label(text, { point.x, point.y, 0, 0 }, fontWidth, parent); }

    uid GUI::push_button(const std::string& text, const Vec2Int& point, ui_callback event_callback, uid parent) { return push_button(text, { point.x, point.y, defaultMakets.buttonSize.x, defaultMakets.buttonSize.y }, event_callback, parent); }

    uid GUI::push_button(const std::string& text, const Rect& rect, ui_callback event_callback, uid parent)
    {
        uid id = call_register_ui(this, parent);
        UIElement& data = call_get_element(this, id);
        data.rect = rect;
        data.text = text;
        data.prototype = RGUI_BUTTON;
        data.event = reinterpret_cast<void*>(event_callback);

        return id;
    }

    uid GUI::push_edit(const std::string& text, const Vec2Int& point, uid parent) { return push_edit(text, { point.x, point.y, defaultMakets.editSize.x, defaultMakets.editSize.y }, parent); }
    uid GUI::push_edit(const std::string& text, const Runtime::Rect& rect, uid parent)
    {
        uid id = call_register_ui(this, parent);
        UIElement& element = call_get_element(this, id);

        element.text = text;
        element.rect = rect;
        element.prototype = RGUI_EDIT;
        return id;
    }

    uid GUI::push_picture(RoninEngine::Runtime::Sprite* sprite, const Runtime::Rect& rect, uid parent)
    {
        uid id = call_register_ui(this, parent);

        UIElement& data = call_get_element(this, id);
        data.prototype = RGUI_IMAGE;
        data.rect = rect;
        data.resources = sprite;
        return id;
    }
    uid GUI::push_picture(RoninEngine::Runtime::Sprite* sprite, const Vec2Int& point, uid parent) { return push_picture(sprite, { point.x, point.y, sprite->width(), sprite->height() }, parent); }

    template <typename Container>
    uid internal_push_dropdown(GUI* guiInstance, const Container& container, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent)
    {
        using T = typename std::iterator_traits<decltype(container.cbegin())>::value_type;

        uid id = call_register_ui(guiInstance, parent);
        UIElement& element = call_get_element(guiInstance, id);
        element.prototype = RGUI_DROPDOWN;
        element.rect = rect;
        element.resources = factory_resource(element.prototype);
        element.event = (void*)changed;

        if (!element.resources)
            Application::fail_oom_kill();

        auto link = static_cast<std::pair<int, std::list<std::string>>*>(element.resources);
        link->first = index;

        for (auto iter = std::cbegin(container); iter != std::cend(container); ++iter) {
            if constexpr (std::is_same<T, std::string>()) {
                link->second.emplace_back(*iter);
            } else {
                link->second.emplace_back(std::to_string(*iter));
            }
        }

        if (!link->second.empty()) {
            auto iter = link->second.begin();
            std::advance(iter, Math::min(static_cast<int>(link->second.size() - 1), index));
            element.text = *iter;
        }

        return id;
    }

    uid GUI::push_drop_down(const std::list<std::string>& elements, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent) { return internal_push_dropdown(this, elements, index, rect, changed, parent); }

    uid GUI::push_slider(float value, float min, float max, const Rect& rect, ui_callback_float changed, uid parent)
    {
        uid id = call_register_ui(this, parent);
        UIElement& element = call_get_element(this, id);
        element.prototype = RGUI_HSLIDER;
        element.rect = rect;
        element.resources = factory_resource(element.prototype);

        (*(float*)element.resources) = value;
        (*(float*)(element.resources + sizeof(float))) = min;
        (*(float*)(element.resources + sizeof(float) * 2)) = max;

        element.event = (void*)changed;
        return id;
    }

    // property --------------------------------------------------------------------------------------------------------

    void* GUI::get_resources(uid id) { return call_get_element(this, id).resources; }

    void GUI::set_resources(uid id, void* data) { call_get_element(this, id).resources = data; }

    Rect GUI::get_rect(uid id) { return call_get_element(this, id).rect; }
    void GUI::set_rect(uid id, const Rect& rect) { call_get_element(this, id).rect = rect; }

    std::string GUI::get_text(uid id) { return call_get_element(this, id).text; }
    void GUI::set_text(uid id, const std::string& text) { call_get_element(this, id).text = text; }

    void GUI::set_visible(uid id, bool state) { call_get_element(this, id).options = (call_get_element(this, id).options & ~ElementVisibleMask) | (ElementVisibleMask * (state == true)); }
    bool GUI::get_visible(uid id) { return (call_get_element(this, id).options & ElementVisibleMask) != 0; }

    void GUI::set_enable(uid id, bool state) { call_get_element(this, id).options = ((call_get_element(this, id).options & ~ElementEnableMask)) | (ElementEnableMask * (state == true)); }
    bool GUI::get_enable(uid id) { return (call_get_element(this, id).options & ElementEnableMask) != 0; }

    float GUI::get_slider_value(uid id)
    {
        UIElement& elem = call_get_element(this, id);

        if (elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER) {
            throw std::runtime_error("id component is not slider");
        }

        return *static_cast<float*>(elem.resources);
    }

    void GUI::set_slider_value(uid id, float value)
    {
        UIElement& elem = call_get_element(this, id);

        if (elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER) {
            throw std::runtime_error("id component is not slider");
        }

        memcpy(elem.resources, &value, sizeof value);
    }

    // grouping-----------------------------------------------------------------------------------------------------------

    bool GUI::is_group(uid id) { return (call_get_element(this, id).options & ElementGroupMask) == ElementGroupMask; }

    void GUI::show_group_unique(uid id) throw()
    {
        if (!is_group(id))
            throw ronin_ui_cast_group_error();

        resources->ui_layer.layers.remove_if([this](auto v) { return is_group(v); });

        show_group(id);
    }
    void GUI::show_group(uid id) throw()
    {
        if (!is_group(id))
            throw ronin_ui_cast_group_error();

        auto iter = std::find_if(begin(resources->ui_layer.layers), end(resources->ui_layer.layers), std::bind2nd(std::equal_to<uid>(), id));

        if (iter == std::end(resources->ui_layer.layers)) {
            resources->ui_layer.layers.emplace_back(id);
            set_visible(id, true);
        }
    }

    bool GUI::close_group(uid id) throw()
    {
        if (!is_group(id))
            throw ronin_ui_cast_group_error();
        resources->ui_layer.layers.remove(id);
        set_visible(id, false);
    }

    void GUI::set_cast(bool state) { resources->hitCast = state; }
    bool GUI::get_cast() { return resources->hitCast; }

    void GUI::register_general_callback(ui_callback callback, void* userData)
    {
        resources->callback = callback;
        resources->callbackData = userData;
    }
    bool GUI::pop_element(uid id)
    {
        // TODO: мда. Тут проблема. ID которое удаляется может задеть так же и другие. Нужно исправить и найти способ! T``T
        // BUG: GUI PopElement
        return false;
    }
    void GUI::remove_all()
    {
        for (int x = 0; x < resources->ui_layer.elements.size(); ++x) {
            factory_free(&(resources->ui_layer.elements[x]));
        }
        resources->ui_layer.layers.clear();
        resources->ui_layer.elements.clear();
    }
    void native_draw_render(GUI* gui, SDL_Renderer* renderer)
    {
        GUIResources* resources = gui->resources;

        if (resources->visible == false)
            return;

        uid id;
        UIElement* uielement;
        Vec2Int ms;
        std::deque<uid> ui_drains;
        bool ui_hover;
        bool ui_contex;
        bool ms_hover;
        bool ms_click;

        ms = Input::get_mouse_point();

        resources->_focusedUI = false;
        ui_reset_controls(); // Reset

        for (auto iter = begin(resources->ui_layer.layers); iter != end(resources->ui_layer.layers); ++iter)
            ui_drains.emplace_back(*iter);

        ms_click = Input::is_mouse_up();

        while (ui_drains.empty() == false) {
            id = ui_drains.front();
            uielement = &call_get_element(gui, id);

            ms_hover = SDL_PointInRect((SDL_Point*)&ms, (SDL_Rect*)&uielement->rect);
            ui_contex = !uielement->contextRect.empty();

            ui_drains.pop_front();
            if (!(uielement->options & ElementGroupMask) && uielement->options & ElementVisibleMask) {
                ui_hover = id == resources->ui_layer.focusedID;

                // unfocus on click an not hovered
                if (ui_hover && (!ms_hover || !ui_contex) && ms_click) {
                    ui_hover = false;
                    resources->ui_layer.focusedID = 0;
                }

                if ((general_render_ui_section(gui, *uielement, renderer, ms_hover, ms_click && ms_hover, ui_hover)) && resources->hitCast) {
                    // Избавляемся от перекликов в UI
                    resources->_focusedUI = true;

                    if (ui_hover) {
                        resources->ui_layer.focusedID = id;
                    }

                    if (uielement->options & ElementEnableMask) {
                        if (resources->callback) {
                            // Отправка сообщения о действий.
                            resources->callback(id, resources->callbackData);
                        }
                        // run event
                        event_action(uielement);
                    }
                } else { // disabled state
                    // TODO: disabled state for UI element's
                    if (id == resources->ui_layer.focusedID && !ui_hover)
                        resources->ui_layer.focusedID = 0;
                }
                if (!resources->_focusedUI)
                    resources->_focusedUI = ui_hover;
            }

            if (resources->__level_owner->internal_resources->request_unloading)
                break;

            for (auto iter = begin(uielement->childs); iter != end(uielement->childs); ++iter)
                ui_drains.emplace_back(*iter);
        }
    }
    void GUI::set_color_rgb(std::uint32_t rgb) { set_color_rgba(rgb << 8 | SDL_ALPHA_OPAQUE); }
    void GUI::set_color_rgba(std::uint32_t argb) { SDL_SetRenderDrawColor(RoninEngine::renderer, (uid)(argb >> 24) & 0xFF, (uid)(argb >> 16) & 0xFF, (uid)(argb >> 8) & 0xFF, (uid)argb & 0xFF); }
    bool GUI::has_focused_ui() { return resources->_focusedUI; }

} // namespace RoninEngine::UI
