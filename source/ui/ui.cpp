#include "ronin.h"

enum { ElementEnableMask = 1, ElementVisibleMask = 2, ElementGroupMask = 4 };

namespace RoninEngine::UI {
GUI* guiInstance;

extern void* factory_resource(ControlType type);
extern void factory_free(UIElement* element);
extern bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* render, const bool hovering, bool& focus);
extern void event_action(UIElement* element);
uid call_register_ui(GUI* gui, uid parent = NOPARENT) throw() {
    if (parent && !gui->Has_ID(parent)) throw std::runtime_error("Is not end parent");

    UIElement data{};
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

GUI::GUI(Level* level)
    : m_level(level), hitCast(true), callback(nullptr), callbackData(nullptr), visible(true), _focusedUI(false) {
    guiInstance = this;
}

GUI::~GUI() { RemoveAll(); }

// private--------------------------------------

std::list<uid> GUI::get_groups() {
    std::list<uid> __;

    for (auto& iter : ui_layer.elements) {
        if (this->Is_Group(iter.id)) __.push_back(iter.id);
    };

    return __;
}

UIElement& GUI::getElement(uid id) { return call_get_element(this, id); }

// public---------------------------------------

bool GUI::Has_ID(uid id) { return ui_layer.elements.size() >= id; }
uid GUI::Create_Group(const Runtime::Rect& rect) {
    uid id = call_register_ui(this);
    auto& data = getElement(id);
    data.rect = rect;
    data.options |= ElementGroupMask;
    return id;
}
uid GUI::Create_Group() { return Create_Group(Rect::zero); }

uid GUI::Push_Label(const std::string& text, const RoninEngine::Runtime::Rect& rect, const int& fontWidth, uid parent) {
    // todo: fontWidth
    uid id = call_register_ui(this, parent);
    auto& data = getElement(id);
    data.text = text;
    data.rect = rect;
    data.prototype = CTEXT;
    return id;
}
uid GUI::Push_Label(const std::string& text, const Vec2Int& point, const int& fontWidth, uid parent) {
    return Push_Label(text, {point.x, point.y, 0, 0}, fontWidth, parent);
}

uid GUI::Push_Button(const std::string& text, const RoninEngine::Runtime::Rect& rect, uid parent) {
    int id = call_register_ui(this, parent);
    auto& data = getElement(id);
    data.rect = rect;
    data.text = text;
    data.prototype = CBUTTON;
    return id;
}
uid GUI::Push_Button(const std::string& text, const Vec2Int& point, uid parent) {
    return Push_Button(text, {point.x, point.y, defaultMakets.buttonSize.x, defaultMakets.buttonSize.y}, parent);
}

uid GUI::Push_Edit(const std::string& text, const Vec2Int& point, uid parent) {
    return Push_Edit(text, {point.x, point.y, defaultMakets.editSize.x, defaultMakets.editSize.y}, parent);
}
uid GUI::Push_Edit(const std::string& text, const Runtime::Rect& rect, uid parent) {
    uid id = call_register_ui(this, parent);
    UIElement& element = getElement(id);

    element.text = text;
    element.rect = rect;
    element.prototype = CEDIT;
    return id;
}

uid GUI::Push_DisplayRandomizer(TextRandomizer_Format format, const Vec2Int& point, uid parent) {
    uid id = call_register_ui(this, parent);

    auto& data = getElement(id);

    data.resources = (void*)format;
    data.prototype = CTEXTRAND;
    return id;
}
uid GUI::Push_DisplayRandomizer(TextRandomizer_Format format, uid parent) {
    return Push_DisplayRandomizer(format, Vec2Int(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), parent);
}
uid GUI::Push_DisplayRandomizer_Text(const std::string& text, const Vec2Int& point, uid parent) {
    uid id = Push_DisplayRandomizer(TextRandomizer_Format::OnlyText, point, parent);
    auto& data = getElement(id);
    data.text = text;
    return id;
}
uid GUI::Push_DisplayRandomizer_Number(const int& min, const int& max, TextAlign textAlign, uid parent) {
    uid id = Push_DisplayRandomizer(OnlyNumber, parent);
    // TODO: min and max не реализованы.
    return id;
}

uid GUI::Push_TextureStick(Texture* texture, const Runtime::Rect& rect, uid parent) {
    uid id = call_register_ui(this, parent);

    auto& data = getElement(id);
    data.prototype = CIMAGE;
    data.rect = rect;
    data.resources = texture;
    return id;
}
uid GUI::Push_TextureStick(Texture* texture, const Vec2Int& point, uid parent) {
    return Push_TextureStick(texture, {point.x, point.y, texture->width(), texture->height()}, parent);
}
uid GUI::Push_TextureAnimator(Timeline* timeline, const RoninEngine::Runtime::Rect& rect, uid parent) {
    uid id = call_register_ui(this, parent);

    auto& data = getElement(id);
    data.prototype = CIMAGE;
    data.rect = rect;
    data.resources = timeline;
    return id;
}
uid GUI::Push_TextureAnimator(Timeline* timeline, const Vec2Int& point, uid parent) {
    return Push_TextureAnimator(timeline, {point.x, point.y, 0, 0}, parent);
}
uid GUI::Push_TextureAnimator(const std::list<Texture*>& roads, float duration, TimelineOptions option,
                              const RoninEngine::Runtime::Rect& rect, uid parent) {
    Timeline* timeline;
    uid id = call_register_ui(this, parent);
    auto& data = getElement(id);
    data.prototype = CIMAGEANIMATOR;
    data.rect = rect;
    data.resources = timeline = (Timeline*)factory_resource(data.prototype);
    timeline->SetOptions(option);

    // todo: wBest и hBest - вычисляется даже когда rect.w != 0
    int wBest = 0;
    int hBest = 0;
    for (auto i = begin(roads); i != end(roads); ++i) {
        timeline->AddRoad(*i, duration);
        if (wBest < (*i)->width()) wBest = (*i)->width();
        if (hBest < (*i)->height()) hBest = (*i)->height();
    }
    if (!data.rect.w) data.rect.w = wBest;
    if (!data.rect.h) data.rect.h = hBest;

    return id;
}
uid GUI::Push_TextureAnimator(const std::list<Texture*>& roads, float duration, TimelineOptions option, const Vec2Int& point,
                              uid parent) {
    return Push_TextureAnimator(roads, duration, option, {point.x, point.y, 0, 0}, parent);
}

template <typename Container>
uid internal_push_dropdown(const Container& container, int index, const Runtime::Rect& rect, event_index_changed* changed,
                           uid parent) {
    using T = typename std::iterator_traits<decltype(container.cbegin())>::value_type;

    uid id = call_register_ui(guiInstance, parent);
    auto& element = call_get_element(guiInstance, id);
    element.prototype = CDROPDOWN;
    element.rect = rect;
    element.resources = factory_resource(element.prototype);
    element.event = changed;

    if (!element.resources) Application::fail_OutOfMemory();

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

template <>
uid GUI::Push_DropDown(const std::vector<int>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed,
                       uid parent) {
    return internal_push_dropdown(elements, index, rect, changed, parent);
}

template <>
uid GUI::Push_DropDown(const std::vector<float>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed,
                       uid parent) {
    return internal_push_dropdown(elements, index, rect, changed, parent);
}

template <>
uid GUI::Push_DropDown(const std::vector<std::string>& elements, int index, const Runtime::Rect& rect,
                       event_index_changed* changed, uid parent) {
    return internal_push_dropdown(elements, index, rect, changed, parent);
}

template <>
uid GUI::Push_DropDown(const std::list<float>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed,
                       uid parent) {
    return internal_push_dropdown(elements, index, rect, changed, parent);
}

template <>
uid GUI::Push_DropDown(const std::list<int>& elements, int index, const Runtime::Rect& rect, event_index_changed* changed,
                       uid parent) {
    return internal_push_dropdown(elements, index, rect, changed, parent);
}

template <>
uid GUI::Push_DropDown(const std::list<std::string>& elements, int index, const Runtime::Rect& rect,
                       event_index_changed* changed, uid parent) {
    return internal_push_dropdown(elements, index, rect, changed, parent);
}

uid GUI::Push_Slider(float value, float min, float max, const Rect& rect, event_value_changed* changed, uid parent) {
    uid id = call_register_ui(guiInstance, parent);
    auto& element = call_get_element(guiInstance, id);
    element.prototype = CHSLIDER;
    element.rect = rect;
    element.resources = factory_resource(element.prototype);

    (*(float*)element.resources) = value;
    (*(float*)(element.resources + sizeof(float))) = min;
    (*(float*)(element.resources + sizeof(float) * 2)) = max;

    element.event = changed;
    return id;
}

// property --------------------------------------------------------------------------------------------------------

void* GUI::getResources(uid id) { return getElement(id).resources; }
void GUI::setResources(uid id, void* data) { getElement(id).resources = data; }

Rect GUI::getRect(uid id) { return getElement(id).rect; }
void GUI::setRect(uid id, const RoninEngine::Runtime::Rect& rect) { getElement(id).rect = rect; }

std::string GUI::getText(uid id) { return getElement(id).text; }
void GUI::setText(uid id, const std::string& text) { getElement(id).text = text; }

void GUI::setVisible(uid id, bool state) {
    getElement(id).options = (getElement(id).options & ~ElementVisibleMask) | (ElementVisibleMask * (state == true));
}
bool GUI::getVisible(uid id) { return (getElement(id).options & ElementVisibleMask) != 0; }

void GUI::setEnable(uid id, bool state) {
    getElement(id).options = ((getElement(id).options & ~ElementEnableMask)) | (ElementEnableMask * (state == true));
}
bool GUI::getEnable(uid id) { return getElement(id).options & ElementEnableMask != 0; }

// grouping-----------------------------------------------------------------------------------------------------------

bool GUI::Is_Group(uid id) { return getElement(id).options & ElementGroupMask != 0; }

void GUI::Show_GroupUnique(uid id) throw() {
    if (!Is_Group(id)) throw std::runtime_error("Is't group");

    this->ui_layer.layers.remove_if([this](auto v) { return this->Is_Group(v); });

    Show_Group(id);
}
void GUI::Show_Group(uid id) throw() {
    if (!Is_Group(id)) throw std::runtime_error("Is't group");

    auto iter = find_if(begin(ui_layer.layers), end(ui_layer.layers), [&id](auto& _id) { return _id == id; });

    if (iter == end(ui_layer.layers)) {
        ui_layer.layers.emplace_back(id);
        setVisible(id, true);
    }
}

bool GUI::Close_Group(uid id) throw() {
    if (!Is_Group(id)) throw std::runtime_error("Is't group");
    ui_layer.layers.remove(id);
    setVisible(id, false);
}

void GUI::setCast(bool state) { hitCast = state; }
bool GUI::getCast() { return hitCast; }

void GUI::Register_Callback(ui_callback callback, void* userData) {
    this->callback = callback;
    this->callbackData = userData;
}
bool GUI::Pop_Element(uid id) {
    // TODO: мда. Тут проблема. ID которое удаляется может задеть так же и другие. Нужно исправить и найти способ! T``T

    return false;
}
void GUI::RemoveAll() {
    for (int x = 0; x < ui_layer.elements.size(); ++x) {
        factory_free(&ui_layer.elements[x]);
    }
    ui_layer.layers.clear();
    ui_layer.elements.clear();
}
void GUI::Do_Present(SDL_Renderer* renderer) {
    if (!visible) return;

    uid id;
    UIElement* uielement;
    Vec2Int ms;
    std::list<uid> drain;
    ResetControls();  // Reset
    bool uiFocus;
    bool uiHover;
    bool uiContex;

    ms = input::getMousePoint();
    _focusedUI = false;

    for (auto iter = begin(ui_layer.layers); iter != end(ui_layer.layers); ++iter) drain.emplace_back(*iter);

    while (drain.size()) {
        id = drain.front();
        uielement = &getElement(id);

        uiHover = SDL_PointInRect((SDL_Point*)&ms, (SDL_Rect*)&uielement->rect);
        uiContex = !uielement->contextRect.empty();

        drain.pop_front();
        if (!(uielement->options & ElementGroupMask) && uielement->options & ElementVisibleMask) {
            uiFocus = id == ui_layer.focusedID;

            // unfocus on click an not hovered
            if (uiFocus && (!uiHover || !uiContex) && input::isMouseUp()) {
                uiFocus = false;
                ui_layer.focusedID = 0;
            }

            if (general_render_ui_section(this, *uielement, renderer, uiHover, uiFocus) && hitCast) {
                //Избавляемся от перекликов в UI
                _focusedUI = true;

                if (uiFocus) {
                    ui_layer.focusedID = id;
                }

                if (uielement->options & ElementEnableMask) {
                    if (callback) {
                        //Отправка сообщения о действий.
                        callback(id, callbackData);
                    }
                    // run event
                    event_action(uielement);
                }
            } else {  // disabled state
                // TODO: disabled state for UI element's
                if (id == ui_layer.focusedID && !uiFocus) ui_layer.focusedID = 0;
            }
            if (!_focusedUI) _focusedUI = uiFocus;
        }

        if (this->m_level->m_isUnload) break;

        for (auto iter = begin(uielement->childs); iter != end(uielement->childs); ++iter) drain.emplace_back(*iter + 1);
    }
}
void GUI::GUI_SetMainColorRGB(uint32_t RGB) { GUI_SetMainColorRGBA(RGB << 8 | SDL_ALPHA_OPAQUE); }
void GUI::GUI_SetMainColorRGBA(uint32_t ARGB) {
    SDL_SetRenderDrawColor(Application::GetRenderer(), (uid)(ARGB >> 24) & 0xFF, (uid)(ARGB >> 16) & 0xFF,
                           (uid)(ARGB >> 8) & 0xFF, (uid)ARGB & 0xFF);
}
bool GUI::Focused_UI() { return _focusedUI; }

}  // namespace RoninEngine::UI
