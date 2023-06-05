#include "ronin.h"

enum { ElementEnableMask = 1, ElementVisibleMask = 2, ElementGroupMask = 4 };

namespace RoninEngine::UI
{
    extern void* factory_resource(GUIControlPresents type);
    extern void factory_free(UIElement* element);
    extern bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* render, const bool hovering, bool& focus);
    extern void event_action(UIElement* element);

    extern void ui_controls_init();
    extern void ui_free_controls();
    extern void ui_reset_controls();

    uid call_register_ui(GUI* gui, uid parent = NOPARENT)
    {
        if (!gui->has_ui(parent))
            throw std::runtime_error("Is not end parent");

        UIElement data {};
        data.parentId = parent;
        data.options = ElementVisibleMask | ElementEnableMask;
        gui->ui_layer.elements.emplace_back(data);
        data.id = gui->ui_layer.elements.size();
        // add the child
        if (parent)
            gui->ui_layer.elements[parent - 1].childs.emplace_back(gui->ui_layer.elements.size() - 1);
        else
            gui->ui_layer.layers.push_back(data.id);

        return data.id;
    }
    inline UIElement& call_get_element(GUI* gui, uid id) { return gui->ui_layer.elements[id - 1]; }

    GUI::GUI(RoninEngine::Runtime::World *level)
        : m_level(level)
        , hitCast(true)
        , callback(nullptr)
        , callbackData(nullptr)
        , visible(true)
        , _focusedUI(false)
    {
    }

    GUI::~GUI() { remove_all(); }

    // private--------------------------------------

    std::list<uid> GUI::get_groups()
    {
        std::list<uid> __;

        for (auto& iter : ui_layer.elements) {
            if (this->is_group(iter.id))
                __.push_back(iter.id);
        };

        return __;
    }

    UIElement& GUI::getElement(uid id) { return call_get_element(this, id); }

    // public---------------------------------------

    bool GUI::has_ui(uid id) { return ui_layer.elements.size() >= id; }
    uid GUI::push_group(const Runtime::Rect& rect)
    {
        uid id = call_register_ui(this);
        auto& data = getElement(id);
        data.rect = rect;
        data.options |= ElementGroupMask;
        return id;
    }
    uid GUI::push_group() { return push_group(Rect::zero); }

    uid GUI::push_label(const std::string& text, const ::Rect& rect, const int& fontWidth, uid parent)
    {
        // todo: fontWidth
        uid id = call_register_ui(this, parent);
        auto& data = getElement(id);
        data.text = text;
        data.rect = rect;
        data.prototype = RGUI_TEXT;
        return id;
    }
    uid GUI::push_label(const std::string& text, const Vec2Int& point, const int& fontWidth, uid parent) { return push_label(text, { point.x, point.y, 0, 0 }, fontWidth, parent); }

    uid GUI::push_button(const std::string& text, const Vec2Int& point, ui_callback* event_callback, uid parent) { return push_button(text, { point.x, point.y, defaultMakets.buttonSize.x, defaultMakets.buttonSize.y }, event_callback, parent); }

    uid GUI::push_button(const std::string& text, const Rect& rect, ui_callback* event_callback, uid parent)
    {
        int id = call_register_ui(this, parent);
        auto& data = getElement(id);
        data.rect = rect;
        data.text = text;
        data.prototype = RGUI_BUTTON;
        data.event = event_callback;
        return id;
    }

    uid GUI::push_edit(const std::string& text, const Vec2Int& point, uid parent) { return push_edit(text, { point.x, point.y, defaultMakets.editSize.x, defaultMakets.editSize.y }, parent); }
    uid GUI::push_edit(const std::string& text, const Runtime::Rect& rect, uid parent)
    {
        uid id = call_register_ui(this, parent);
        UIElement& element = getElement(id);

        element.text = text;
        element.rect = rect;
        element.prototype = RGUI_EDIT;
        return id;
    }

    uid GUI::push_display_randomizer(TextRandomizer_Format format, const Vec2Int& point, uid parent)
    {
        uid id = call_register_ui(this, parent);

        auto& data = getElement(id);

        data.resources = reinterpret_cast<void*>(format);
        data.prototype = RGUI_TEXTRAND;
        return id;
    }
    uid GUI::push_display_randomizer(TextRandomizer_Format format, uid parent) { return push_display_randomizer(format, Vec2Int(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), parent); }
    uid GUI::push_display_randomizer_text(const std::string& text, const Vec2Int& point, uid parent)
    {
        uid id = push_display_randomizer(TextRandomizer_Format::OnlyText, point, parent);
        auto& data = getElement(id);
        data.text = text;
        return id;
    }
    uid GUI::Push_DisplayRandomizer_Number(const int& min, const int& max, TextAlign textAlign, uid parent)
    {
        uid id = push_display_randomizer(OnlyNumber, parent);
        // TODO: min and max не реализованы.
        return id;
    }

    uid GUI::push_texture_stick(Texture* texture, const Runtime::Rect& rect, uid parent)
    {
        uid id = call_register_ui(this, parent);

        auto& data = getElement(id);
        data.prototype = RGUI_IMAGE;
        data.rect = rect;
        data.resources = texture;
        return id;
    }
    uid GUI::push_texture_stick(Texture* texture, const Vec2Int& point, uid parent) { return push_texture_stick(texture, { point.x, point.y, texture->width(), texture->height() }, parent); }
    uid GUI::push_texture_animator(Timeline* timeline, const ::Rect& rect, uid parent)
    {
        uid id = call_register_ui(this, parent);

        auto& data = getElement(id);
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
        auto& data = getElement(id);
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
    uid internal_push_dropdown(GUI* guiInstance, const Container& container, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent)
    {
        using T = typename std::iterator_traits<decltype(container.cbegin())>::value_type;

        uid id = call_register_ui(guiInstance, parent);
        auto& element = call_get_element(guiInstance, id);
        element.prototype = RGUI_DROPDOWN;
        element.rect = rect;
        element.resources = factory_resource(element.prototype);
        element.event = changed;

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

    uid GUI::push_drop_down(const std::vector<int>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent) { return internal_push_dropdown(this,elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::vector<float>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent) { return internal_push_dropdown(this,elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::vector<std::string>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent) { return internal_push_dropdown(this,elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::list<float>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent) { return internal_push_dropdown(this,elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::list<int>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent) { return internal_push_dropdown(this,elements, index, rect, changed, parent); }

    uid GUI::push_drop_down(const std::list<std::string>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed, uid parent) { return internal_push_dropdown(this,elements, index, rect, changed, parent); }

    uid GUI::push_slider(float value, float min, float max, const Rect& rect, event_value_changed* changed, uid parent)
    {
        uid id = call_register_ui(this, parent);
        auto& element = call_get_element(this, id);
        element.prototype = RGUI_HSLIDER;
        element.rect = rect;
        element.resources = factory_resource(element.prototype);

        (*(float*)element.resources) = value;
        (*(float*)(element.resources + sizeof(float))) = min;
        (*(float*)(element.resources + sizeof(float) * 2)) = max;

        element.event = changed;
        return id;
    }

    // property --------------------------------------------------------------------------------------------------------

    void* GUI::get_resources(uid id) { return getElement(id).resources; }

    void GUI::set_resources(uid id, void* data) { getElement(id).resources = data; }

    Rect GUI::get_rect(uid id) { return getElement(id).rect; }
    void GUI::set_rect(uid id, const ::Rect& rect) { getElement(id).rect = rect; }

    std::string GUI::get_text(uid id) { return getElement(id).text; }
    void GUI::set_text(uid id, const std::string& text) { getElement(id).text = text; }

    void GUI::set_visible(uid id, bool state) { getElement(id).options = (getElement(id).options & ~ElementVisibleMask) | (ElementVisibleMask * (state == true)); }
    bool GUI::get_visible(uid id) { return (getElement(id).options & ElementVisibleMask) != 0; }

    void GUI::set_enable(uid id, bool state) { getElement(id).options = ((getElement(id).options & ~ElementEnableMask)) | (ElementEnableMask * (state == true)); }
    bool GUI::get_enable(uid id) { return getElement(id).options & ElementEnableMask != 0; }

    // grouping-----------------------------------------------------------------------------------------------------------

    bool GUI::is_group(uid id) { return getElement(id).options & ElementGroupMask != 0; }

    void GUI::show_group_unique(uid id) throw()
    {
        if (!is_group(id))
            throw std::runtime_error("Is't group");

        this->ui_layer.layers.remove_if([this](auto v) { return this->is_group(v); });

        show_group(id);
    }
    void GUI::show_group(uid id) throw()
    {
        if (!is_group(id))
            throw std::runtime_error("Is't group");

        auto iter = find_if(begin(ui_layer.layers), end(ui_layer.layers), [&id](auto& _id) { return _id == id; });

        if (iter == end(ui_layer.layers)) {
            ui_layer.layers.emplace_back(id);
            set_visible(id, true);
        }
    }

    bool GUI::close_group(uid id) throw()
    {
        if (!is_group(id))
            throw std::runtime_error("Is't group");
        ui_layer.layers.remove(id);
        set_visible(id, false);
    }

    void GUI::set_cast(bool state) { hitCast = state; }
    bool GUI::get_cast() { return hitCast; }

    void GUI::register_callback(ui_callback callback, void* userData)
    {
        this->callback = callback;
        this->callbackData = userData;
    }
    bool GUI::pop_element(uid id)
    {
        // TODO: мда. Тут проблема. ID которое удаляется может задеть так же и другие. Нужно исправить и найти способ! T``T

        return false;
    }
    void GUI::remove_all()
    {
        for (int x = 0; x < ui_layer.elements.size(); ++x) {
            factory_free(&ui_layer.elements[x]);
        }
        ui_layer.layers.clear();
        ui_layer.elements.clear();
    }
    void GUI::native_draw_render(SDL_Renderer* renderer)
    {
        if (!visible)
            return;

        uid id;
        UIElement* uielement;
        Vec2Int ms;
        std::list<uid> ui_drains;
        ui_reset_controls(); // Reset
        bool uiFocus;
        bool uiHover;
        bool uiContex;

        ms = Input::get_mouse_point();
        _focusedUI = false;

        for (auto iter = begin(ui_layer.layers); iter != end(ui_layer.layers); ++iter)
            ui_drains.emplace_back(*iter);

        while (ui_drains.size()) {
            id = ui_drains.front();
            uielement = &getElement(id);

            uiHover = SDL_PointInRect((SDL_Point*)&ms, (SDL_Rect*)&uielement->rect);
            uiContex = !uielement->contextRect.empty();

            ui_drains.pop_front();
            if (!(uielement->options & ElementGroupMask) && uielement->options & ElementVisibleMask) {
                uiFocus = id == ui_layer.focusedID;

                // unfocus on click an not hovered
                if (uiFocus && (!uiHover || !uiContex) && Input::is_mouse_up()) {
                    uiFocus = false;
                    ui_layer.focusedID = 0;
                }

                if (general_render_ui_section(this, *uielement, renderer, uiHover, uiFocus) && hitCast) {
                    // Избавляемся от перекликов в UI
                    _focusedUI = true;

                    if (uiFocus) {
                        ui_layer.focusedID = id;
                    }

                    if (uielement->options & ElementEnableMask) {
                        if (callback) {
                            // Отправка сообщения о действий.
                            callback(id, callbackData);
                        }
                        // run event
                        event_action(uielement);
                    }
                } else { // disabled state
                    // TODO: disabled state for UI element's
                    if (id == ui_layer.focusedID && !uiFocus)
                        ui_layer.focusedID = 0;
                }
                if (!_focusedUI)
                    _focusedUI = uiFocus;
            }

            if (this->m_level->internal_resources->request_unloading)
                break;

            for (auto iter = begin(uielement->childs); iter != end(uielement->childs); ++iter)
                ui_drains.emplace_back(*iter + 1);
        }
    }
    void GUI::set_color_rgb(uint32_t RGB) { set_color_rgba(RGB << 8 | SDL_ALPHA_OPAQUE); }
    void GUI::set_color_rgba(uint32_t ARGB) { SDL_SetRenderDrawColor(Application::get_renderer(), (uid)(ARGB >> 24) & 0xFF, (uid)(ARGB >> 16) & 0xFF, (uid)(ARGB >> 8) & 0xFF, (uid)ARGB & 0xFF); }
    bool GUI::has_focused_ui() { return _focusedUI; }

} // namespace RoninEngine::UI
