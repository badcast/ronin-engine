#include "framework.h"
#include "inputSystem.h"

namespace RoninEngine::UI {
std::list<UC*> m_controls;
std::list<UC*> nativeControls() { return m_controls; }

uint32_t _controlId;
uint32_t _focusedId;

void InitalizeControls() {
    m_controls = {GC::gc_alloc<CText>(),
                  GC::gc_alloc<CEdit>(),
                  GC::gc_alloc<CButton>(),
                  GC::gc_alloc<CVerticalSlider>(),
                  GC::gc_alloc<CImage>(),
                  GC::gc_alloc<CImageAnimator>(),
                  GC::gc_alloc<CTextRandomizerDisplay>()};

    for (auto i : m_controls) {
        if (i == nullptr) Application::fail_OutOfMemory();
    }
}

void Free_Controls() {
    for (auto x : m_controls) {
        GC::gc_unalloc(x);
    }
    m_controls.clear();
}

UC* findControl(ControlType control) {
    auto iter = find_if(begin(m_controls), end(m_controls), [control](UC* ref) { return control == ref->id(); });

    if (iter == end(m_controls)) return nullptr;

    return *iter;
}

void ResetControls() { _controlId = 0; }

bool UC::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) { return false; }

bool CText::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) {
    Render_String(render, data.rect, data.text.c_str(), data.text.size());
    return false;
}

bool CButton::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) {
    static uint8_t pSize = 2;  // pen size
    static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
    auto ms = input::getMousePoint();
    Rect rect;
    *hover = SDL_PointInRect((SDL_Point*)&ms, (SDL_Rect*)&data.rect);
    // border
    guiInstance->GUI_SetMainColorRGB(0xa75100);
    SDL_RenderDrawRect(render, (SDL_Rect*)&data.rect);
    // fill
    guiInstance->GUI_SetMainColorRGB(*hover ? input::isMouseDown() ? 0xab0000 : 0xe70000 : 0xc90000);
    rect = data.rect;
    rect += inside / 2;
    SDL_RenderFillRect(render, (SDL_Rect*)&rect);

    // render text
    Render_String(render, data.rect, data.text.c_str(), data.text.size(), 13, TextAlign::MiddleCenter, true, *hover);
    bool msClick = input::isMouseUp();
    return (*hover) && msClick;
}

bool CEdit::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) { return false; }

bool CHorizontalSlider::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) {
    float* value = (float*)data.resources;
    // Minimal support

    return false;
}

bool CVerticalSlider::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) { return false; }

bool CImage::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) {
    SDL_Rect sdlr;
    Texture* tex = reinterpret_cast<Texture*>(data.resources);
    sdlr.x = data.rect.x;
    sdlr.y = data.rect.y;
    sdlr.w = data.rect.w == 0 ? tex->width() : data.rect.w;
    sdlr.h = data.rect.h == 0 ? tex->height() : data.rect.h;


    SDL_RenderCopy(render, tex->native(), nullptr, &sdlr);
    return false;
}

bool CImageAnimator::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) {
    Timeline* timeline = (Timeline*)data.resources;
    Texture* texture = timeline->Evaluate(Time::time())->texture;
    SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&data.rect);
    return false;
}

bool CTextRandomizerDisplay::render_control(GUI* gui, RenderData& data, SDL_Renderer* render, bool* hover) {
    static float lasttime = 0;

    // todo: Доработать рандомайзера!
    TextRandomizer_Format format = TextRandomizer_Format::OnlyNumber;  // (int)(data.resources);

    data.text.resize(15);
    size_t i;

    if (Time::time() > lasttime) {
        lasttime = Time::time() + 0.1f;
        switch (format) {
            case TextRandomizer_Format::OnlyNumber:
                for (i = 0; i < data.text.length(); ++i) {
                    data.text[i] = '0' + (rand() % ('9'));
                }
                break;
            case TextRandomizer_Format::All:
            default:
                for (i = 0; i < data.text.length(); ++i) {
                    data.text[i] = 32 + rand() % 128;
                }
                break;
        }
    }

    Render_String(Application::GetRenderer(), data.rect, data.text.c_str(), data.text.size());

    return false;
}

//***************** destructors

void UC::destructor(void* resources) {}
void CText::destructor(void* resources) {}
void CButton::destructor(void* resources) {}
void CEdit::destructor(void* resources) {}
void CHorizontalSlider::destructor(void* resources) {}
void CVerticalSlider::destructor(void* resources) {}
void CImage::destructor(void* resources) {}
void CImageAnimator::destructor(void* resources) {}
void CTextRandomizerDisplay::destructor(void* resources) {}

}  // namespace RoninEngine::UI
