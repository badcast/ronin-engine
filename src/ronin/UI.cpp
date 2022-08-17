#include "inputSystem.h"
#include "framework.h"

constexpr int _mEnabled = 1;
constexpr int _mVisibled = 2;
constexpr int _mGroup = 4;

namespace RoninEngine::UI {
GUI* guiInstance;

GUI::GUI(Level* level)
    : m_level(level), hitCast(true), callback(nullptr), callbackData(nullptr), visible(true), _focusedUI(false) {
    guiInstance = this;
}

GUI::~GUI() {}

// private--------------------------------------

std::list<uid> GUI::get_groups() {
    std::list<uid> __;

    for (auto iter : m_Sticks.controls) {
        if (this->Is_Group(iter.id)) __.push_back(iter.id);
    };

    return __;
}

uid GUI::call_register_ui(const uid& parent) throw() {
    if (parent && !Has_ID(parent)) throw std::runtime_error("Is not ed parent");

    RenderData data{};
    data.parentId = parent;
    data.options = _mVisibled | _mEnabled;
    m_Sticks.controls.emplace_back(data);
    data.id = m_Sticks.controls.size();
    // add the child
    if (parent)
        m_Sticks.controls[parent - 1].childs.emplace_back(m_Sticks.controls.size() - 1);
    else
        m_Sticks._rendering.push_back(data.id);

    return data.id;
}

RenderData& GUI::ID(const uid& id) { return m_Sticks.controls[id - 1]; }

// public---------------------------------------

bool GUI::Has_ID(const uid& id) { return m_Sticks.controls.size() >= id; }
uid GUI::Create_Group(const RoninEngine::Runtime::Rect& rect) {
    uid id = call_register_ui();
    auto& data = ID(id);
    data.rect = rect;
    data.options |= _mGroup;
    return id;
}
uid GUI::Create_Group() { return Create_Group(RoninEngine::Runtime::Rect::zero); }

uid GUI::Push_Label(const std::string& text, const RoninEngine::Runtime::Rect& rect, const int& fontWidth, const uid& parent) {
    // todo: fontWidth
    uid id = call_register_ui(parent);
    auto& data = ID(id);
    data.text = text;
    data.rect = rect;
    data.prototype = findControl(CTEXT);
    return id;
}
uid GUI::Push_Label(const std::string& text, const Vec2Int& point, const int& fontWidth, const uid& parent) {
    return Push_Label(text, {point.x, point.y, 0, 0}, fontWidth, parent);
}

uid GUI::Push_Button(const std::string& text, const RoninEngine::Runtime::Rect& rect, const uid& parent) {
    int id = call_register_ui(parent);
    auto& data = ID(id);
    data.prototype = findControl(CBUTTON);
    data.rect = rect;
    data.text = text;
    return id;
}
uid GUI::Push_Button(const std::string& text, const Vec2Int point, const uid& parent) {
    return Push_Button(text, {point.x, point.y, 256, 32}, parent);
}

uid GUI::Push_DisplayRandomizer(TextRandomizer_Format format, const Vec2Int& point, const uid& parent) {
    uid id = call_register_ui(parent);

    auto& data = ID(id);

    data.prototype = findControl(CTEXTRAND);
    data.resources = (void*)format;
    return id;
}
uid GUI::Push_DisplayRandomizer(TextRandomizer_Format format, const uid& parent) {
    return Push_DisplayRandomizer(format, Vec2Int(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), parent);
}
uid GUI::Push_DisplayRanomizer_Text(const std::string& text, const Vec2Int& point, const uid& parent) {
    uid id = Push_DisplayRandomizer(TextRandomizer_Format::OnlyText, point, parent);
    auto& data = ID(id);

    return id;
}
uid GUI::Push_DisplayRanomizer_Number(const int& min, const int& max, TextAlign textAlign, const uid& parent) {
    uid id = Push_DisplayRandomizer(OnlyNumber, parent);
    // TODO: min and max не реализованы.
    return id;
}

uid GUI::Push_TextureStick(Texture* texture, const RoninEngine::Runtime::Rect& rect, const uid& parent) {
    uid id = call_register_ui(parent);

    auto& data = ID(id);
    data.prototype = findControl(CIMAGE);
    data.rect = rect;
    data.resources = texture;
    return id;
}
uid GUI::Push_TextureStick(Texture* texture, const Vec2Int point, const uid& parent) {
    return Push_TextureStick(texture, {point.x, point.y, texture->width(), texture->height()}, parent);
}
uid GUI::Push_TextureAnimator(Timeline* timeline, const RoninEngine::Runtime::Rect& rect, const uid& parent) {
    uid id = call_register_ui(parent);

    auto& data = ID(id);
    data.prototype = findControl(CIMAGE);
    data.rect = rect;
    data.resources = timeline;
    return id;
}
uid GUI::Push_TextureAnimator(Timeline* timeline, const Vec2Int& point, const uid& parent) {
    return Push_TextureAnimator(timeline, {point.x, point.y, 0, 0}, parent);
}
uid GUI::Push_TextureAnimator(const std::list<Texture*>& roads, float duration, TimelineOptions option, const RoninEngine::Runtime::Rect& rect,
                              const uid& parent) {
    Timeline* timeline = nullptr;
    uid id = call_register_ui(parent);
    auto& data = ID(id);
    data.prototype = findControl(CIMAGEANIMATOR);
    data.rect = rect;
    GC::gc_alloc_lval(timeline);
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
    // can destroy
    data.resources = timeline;
    return id;
}
uid GUI::Push_TextureAnimator(const std::list<Texture*>& roads, float duration, TimelineOptions option, const Vec2Int& point,
                              const uid& parent) {
    return Push_TextureAnimator(roads, duration, option, {point.x, point.y, 0, 0}, parent);
}

void* GUI::Resources(const uid& id) { return ID(id).resources; }
void GUI::Resources(const uid& id, void* data) { ID(id).resources = data; }
RoninEngine::Runtime::Rect GUI::Rect(const uid& id) { return ID(id).rect; }
void GUI::Rect(const uid& id, const RoninEngine::Runtime::Rect& rect) { ID(id).rect = rect; }
std::string GUI::Text(const uid& id) { return ID(id).text; }
void GUI::Text(const uid& id, const std::string& text) { ID(id).text = text; }

void GUI::Visible(const uid& id, bool state) { ID(id).options = ID(id).options & (~_mVisibled) | (_mVisibled * state); }
bool GUI::Visible(const uid& id) { return (ID(id).options & _mVisibled) >> 1; }

void GUI::Enable(const uid& id, bool state) { ID(id).options = ID(id).options & (~_mEnabled) | (_mEnabled * state); }
bool GUI::Enable(const uid& id) { return ID(id).options & _mEnabled; }

// grouping-----------------------------------------------------------------------------------------------------------

bool GUI::Is_Group(const uid& id) { return ID(id).options & _mGroup; }

void GUI::Show_GroupUnique(const uid& id) throw() {
    if (!Is_Group(id)) throw std::runtime_error("Is't group");

    this->m_Sticks._rendering.remove_if([this](auto v) { return this->Is_Group(v); });

    Show_Group(id);
}
void GUI::Show_Group(const uid& id) throw() {
    if (!Is_Group(id)) throw std::runtime_error("Is't group");

    auto iter = find_if(begin(m_Sticks._rendering), end(m_Sticks._rendering), [&id](auto& _id) { return _id == id; });

    if (iter == end(m_Sticks._rendering)) {
        m_Sticks._rendering.emplace_back(id);
        Visible(id, true);
    }
}

bool GUI::Close_Group(const uid& id) throw() {
    if (!Is_Group(id)) throw std::runtime_error("Is't group");
    m_Sticks._rendering.remove(id);
    Visible(id, false);
}

void GUI::Cast(bool state) { hitCast = state; }
bool GUI::Cast() { return hitCast; }

void GUI::Register_Callback(ui_callback callback, void* userData) {
    this->callback = callback;
    this->callbackData = userData;
}
bool GUI::Remove(const uid& id) {
    // TODO: мда. Тут проблема. ID которое удаляется может задеть так же и другие. Нужно исправить и найти способ! T``T

    return false;
}
void GUI::RemoveAll() { m_Sticks.controls.clear(); }
void GUI::Do_Present(SDL_Renderer* renderer) {
    if (visible) {
        uid id;
        RenderData* x;
        std::list<uid> _render;
        ResetControls();  // Reset
        bool targetFocus = false;
        _focusedUI = false;
        for (auto iter = begin(m_Sticks._rendering); iter != end(m_Sticks._rendering); ++iter) _render.emplace_back(*iter);

        while (_render.size()) {
            id = _render.front();
            x = &ID(id);

            _render.pop_front();
            if (!(x->options & _mGroup) && x->options & _mVisibled && x->prototype) {
                if (x->prototype->render_control(this, *x, renderer, &targetFocus) && hitCast) {
                    _focusedUI = true;
                    //Избавляемся от перекликов в UI
                    //  input::event->type = SDL_FIRSTEVENT;
                    // TODO: input::event->type SDL_FIRSTEVENT

                    if (x->options & _mEnabled && callback) {
                        //Отправка сообщения о действий.
                        callback(id, callbackData);
                    }
                } else  // disabled state
                    ;

                if (!_focusedUI) _focusedUI = targetFocus;
            }

            if (this->m_level->m_isUnload) break;

            for (auto iter = begin(x->childs); iter != end(x->childs); ++iter) _render.emplace_back(*iter + 1);
        }
    }
}
void GUI::GUI_SetMainColorRGB(uint32_t RGB) { GUI_SetMainColorRGBA(RGB << 8 | SDL_ALPHA_OPAQUE); }
void GUI::GUI_SetMainColorRGBA(uint32_t ARGB) {
    SDL_SetRenderDrawColor(Application::GetRenderer(), (uid)(ARGB >> 24) & 0xFF, (uid)(ARGB >> 16) & 0xFF,
                           (uid)(ARGB >> 8) & 0xFF, (uid)ARGB & 0xFF);
}
bool GUI::Focused_UI() { return _focusedUI; }
}  // namespace RoninEngine::UI
