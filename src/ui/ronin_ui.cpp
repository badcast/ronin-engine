#include "ronin.h"

enum { ElementEnableMask = 1, ElementVisibleMask = 2, ElementGroupMask = 4 };

namespace RoninEngine::UI
{
    extern ui_resource* factory_resource(GUIControlPresents type);
    extern void factory_free(UIElement* element);
    extern bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* render, const bool hovering, bool& focus);
    extern void event_action(UIElement* element);

    extern void ui_controls_init();
    extern void ui_free_controls();
    extern void ui_reset_controls();

    uid call_register_ui(GUI* gui, uid parent = NOPARENT)
    {
        if (!gui->has_ui(parent))
            throw std::runtime_error("Isn't parent");

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
        RoninMemory::alloc_self<GUIResources>(resources);
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
        RoninMemory::free(resources);
    }

    // private--------------------------------------

    std::list<uid> GUI::get_groups()
    {
        std::list<uid> __;

        for (auto& iter : resources->ui_layer.elements) {
            if (is_group(iter.id))
                __.push_back(iter.id);
        };

        return __;
    }

    UIElement& GUI::ui_get_element(uid id) { return call_get_element(this, id); }

    // public---------------------------------------

    bool GUI::has_ui(uid id) { return resources->ui_layer.elements.size() >= id; }

    uid GUI::push_group(const Runtime::Rect& rect)
    {
        uid id = call_register_ui(this);
        auto& data = ui_get_element(id);
        data.rect = rect;
        data.options |= ElementGroupMask;

        return id;
    }
    uid GUI::push_group() { return push_group(Rect::zero); }

    uid GUI::begin_layment(const Runtime::Rect region) { }

    uid GUI::push_label(const std::string& text, const ::Rect& rect, const int& fontWidth, uid parent)
    {
        // todo: fontWidth
        uid id = call_register_ui(this, parent);
        auto& data = ui_get_element(id);
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
        auto& data = ui_get_element(id);
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
        UIElement& element = ui_get_element(id);

        element.text = text;
        element.rect = rect;
        element.prototype = RGUI_EDIT;
        return id;
    }

    uid GUI::push_texture_stick(Texture* texture, const Runtime::Rect& rect, uid parent)
    {
        uid id = call_register_ui(this, parent);

        auto& data = ui_get_element(id);
        data.prototype = RGUI_IMAGE;
        data.rect = rect;
        data.resources = texture;
        return id;
    }
    uid GUI::push_texture_stick(Texture* texture, const Vec2Int& point, uid parent) { return push_texture_stick(texture, { point.x, point.y, texture->width(), texture->height() }, parent); }
    uid GUI::push_texture_animator(Timeline* timeline, const ::Rect& rect, uid parent)
    {
        uid id = call_register_ui(this, parent);

        auto& data = ui_get_element(id);
        data.prototype = RGUI_IMAGE;
        data.rect = rect;
        data.resources = timeline;
        return id;
    }
    uid GUI::push_texture_animator(Timeline* timeline, const Vec2Int& point, uid parent) { return push_texture_animator(timeline, { point.x, point.y, 0, 0 }, parent); }
    uid GUI::push_texture_animator(const std::list<Texture*>& roads, float duration, TimelineOptions option, const ::Rect& rect, uid parent)
    {
        Timeline* timeline;
        uid id = call_register_ui(this, parent);
        auto& data = ui_get_element(id);
        data.prototype = RGUI_IMAGEANIMATOR;
        data.rect = rect;
        data.resources = timeline = (Timeline*)factory_resource(data.prototype);
        timeline->SetOptions(option);

        // todo: wBest и hBest - вычисляется даже когда rect.w != 0
        int wBest = 0;
        int hBest = 0;
        for (auto i = begin(roads); i != end(roads); ++i) {
            timeline->AddRoad(*i, duration);
            if (wBest < (*i)->width())
                wBest = (*i)->width();
            if (hBest < (*i)->height())
                hBest = (*i)->height();
        }
        if (!data.rect.w)
            data.rect.w = wBest;
        if (!data.rect.h)
            data.rect.h = hBest;

        return id;
    }
    uid GUI::push_texture_animator(const std::list<Texture*>& roads, float duration, TimelineOptions option, const Vec2Int& point, uid parent) { return push_texture_animator(roads, duration, option, { point.x, point.y, 0, 0 }, parent); }

    template <typename Container>
    uid internal_push_dropdown(GUI* guiInstance, const Container& container, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent)
    {
        using T = typename std::iterator_traits<decltype(container.cbegin())>::value_type;

        uid id = call_register_ui(guiInstance, parent);
        auto& element = call_get_element(guiInstance, id);
        element.prototype = RGUI_DROPDOWN;
        element.rect = rect;
        element.resources = factory_resource(element.prototype);
        element.event = &changed;

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

    uid GUI::push_drop_down(const std::vector<int>& elements, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent) { return internal_push_dropdown(this, elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::vector<float>& elements, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent) { return internal_push_dropdown(this, elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::vector<std::string>& elements, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent) { return internal_push_dropdown(this, elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::list<float>& elements, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent) { return internal_push_dropdown(this, elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::list<int>& elements, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent) { return internal_push_dropdown(this, elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::list<std::string>& elements, int index, const Runtime::Rect& rect, ui_callback_integer changed, uid parent) { return internal_push_dropdown(this, elements, index, rect, changed, parent); }

    uid GUI::push_slider(float value, float min, float max, const Rect& rect, ui_callback_float changed, uid parent)
    {
        uid id = call_register_ui(this, parent);
        auto& element = call_get_element(this, id);
        element.prototype = RGUI_HSLIDER;
        element.rect = rect;
        element.resources = factory_resource(element.prototype);

        (*(float*)element.resources) = value;
        (*(float*)(element.resources + sizeof(float))) = min;
        (*(float*)(element.resources + sizeof(float) * 2)) = max;

        element.event = &changed;
        return id;
    }

    // property --------------------------------------------------------------------------------------------------------

    void* GUI::get_resources(uid id) { return ui_get_element(id).resources; }

    void GUI::set_resources(uid id, void* data) { ui_get_element(id).resources = data; }

    Rect GUI::get_rect(uid id) { return ui_get_element(id).rect; }
    void GUI::set_rect(uid id, const ::Rect& rect) { ui_get_element(id).rect = rect; }

    std::string GUI::get_text(uid id) { return ui_get_element(id).text; }
    void GUI::set_text(uid id, const std::string& text) { ui_get_element(id).text = text; }

    void GUI::set_visible(uid id, bool state) { ui_get_element(id).options = (ui_get_element(id).options & ~ElementVisibleMask) | (ElementVisibleMask * (state == true)); }
    bool GUI::get_visible(uid id) { return (ui_get_element(id).options & ElementVisibleMask) != 0; }

    void GUI::set_enable(uid id, bool state) { ui_get_element(id).options = ((ui_get_element(id).options & ~ElementEnableMask)) | (ElementEnableMask * (state == true)); }
    bool GUI::get_enable(uid id) { return ui_get_element(id).options & ElementEnableMask != 0; }

    float GUI::get_slider_value(uid id)
    {
        auto& elem = ui_get_element(id);

        if (elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER) {
            throw std::runtime_error("id component is not slider");
        }

        return *static_cast<float*>(elem.resources);
    }

    void GUI::set_slider_value(uid id, float value)
    {
        auto& elem = ui_get_element(id);

        if (elem.prototype != RGUI_HSLIDER && elem.prototype != RGUI_VSLIDER) {
            throw std::runtime_error("id component is not slider");
        }

        memcpy(elem.resources, &value, sizeof value);
    }

    // grouping-----------------------------------------------------------------------------------------------------------

    bool GUI::is_group(uid id) { return ui_get_element(id).options & ElementGroupMask != 0; }

    void GUI::show_group_unique(uid id) throw()
    {
        if (!is_group(id))
            throw std::runtime_error("Isn't group");

        resources->ui_layer.layers.remove_if([this](auto v) { return is_group(v); });

        show_group(id);
    }
    void GUI::show_group(uid id) throw()
    {
        if (!is_group(id))
            throw std::runtime_error("Isn't group");

        auto iter = std::find_if(begin(resources->ui_layer.layers), end(resources->ui_layer.layers), std::bind2nd(std::equal_to<uid>(), id));

        if (iter == std::end(resources->ui_layer.layers)) {
            resources->ui_layer.layers.emplace_back(id);
            set_visible(id, true);
        }
    }

    bool GUI::close_group(uid id) throw()
    {
        if (!is_group(id))
            throw std::runtime_error("Isn't group");
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
    void GUI::native_draw_render(SDL_Renderer* renderer)
    {
        if (resources->visible == false)
            return;

        uid id;
        UIElement* uielement;
        Vec2Int ms;
        std::list<uid> ui_drains;
        bool uiFocus;
        bool uiHover;
        bool uiContex;

        ms = Input::get_mouse_point();

        resources->_focusedUI = false;
        ui_reset_controls(); // Reset

        for (auto iter = begin(resources->ui_layer.layers); iter != end(resources->ui_layer.layers); ++iter)
            ui_drains.emplace_back(*iter);

        while (ui_drains.size()) {
            id = ui_drains.front();
            uielement = &ui_get_element(id);

            uiHover = SDL_PointInRect((SDL_Point*)&ms, (SDL_Rect*)&uielement->rect);
            uiContex = !uielement->contextRect.empty();

            ui_drains.pop_front();
            if (!(uielement->options & ElementGroupMask) && uielement->options & ElementVisibleMask) {
                uiFocus = id == resources->ui_layer.focusedID;

                // unfocus on click an not hovered
                if (uiFocus && (!uiHover || !uiContex) && Input::is_mouse_up()) {
                    uiFocus = false;
                    resources->ui_layer.focusedID = 0;
                }

                if (general_render_ui_section(this, *uielement, renderer, uiHover, uiFocus) && resources->hitCast) {
                    // Избавляемся от перекликов в UI
                    resources->_focusedUI = true;

                    if (uiFocus) {
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
                    if (id == resources->ui_layer.focusedID && !uiFocus)
                        resources->ui_layer.focusedID = 0;
                }
                if (!resources->_focusedUI)
                    resources->_focusedUI = uiFocus;
            }

            if (resources->__level_owner->internal_resources->request_unloading)
                break;

            for (auto iter = begin(uielement->childs); iter != end(uielement->childs); ++iter)
                ui_drains.emplace_back(*iter + 1);
        }
    }
    void GUI::set_color_rgb(std::uint32_t rgb) { set_color_rgba(rgb << 8 | SDL_ALPHA_OPAQUE); }
    void GUI::set_color_rgba(std::uint32_t argb) { SDL_SetRenderDrawColor(Application::get_renderer(), (uid)(argb >> 24) & 0xFF, (uid)(argb >> 16) & 0xFF, (uid)(argb >> 8) & 0xFF, (uid)argb & 0xFF); }
    bool GUI::has_focused_ui() { return resources->_focusedUI; }

} // namespace RoninEngine::UI
