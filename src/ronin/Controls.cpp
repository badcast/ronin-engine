#include "framework.h"
#include "inputSystem.h"

namespace RoninEngine::UI {
uid _controlId;
uid _focusedId;

void InitalizeControls() {
    // TODO: init controls?
}

void Free_Controls() {
    // TODO: free controls?
}

void ResetControls() { _controlId = 0; }

void* factory_resource(ControlType type) {
    void* resources;

    switch (type) {
        case CDROPDOWN:
            resources = GC::gc_alloc<std::list<std::string>>();
            break;
        case CIMAGEANIMATOR:
            resources = GC::gc_alloc<Timeline>();
        default:
            resources = nullptr;
    }
    return resources;
}

void factory_free(UIElement* element) {
    switch (element->prototype) {
        case CDROPDOWN:
            GC::gc_unalloc(reinterpret_cast<std::list<std::string>*>(element->resources));
            break;
        case CIMAGEANIMATOR:
            GC::gc_unalloc(reinterpret_cast<std::list<std::string>*>(element->resources));
            break;
    }
}

bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* render, bool* hover) {
    switch (element.prototype) {
        case CTEXT: {
            Render_String(render, element.rect, element.text.c_str(), element.text.size());
            return false;
        }

        case CBUTTON: {
            static uint8_t pSize = 2;  // pen size
            static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
            auto ms = input::getMousePoint();
            Rect rect;
            *hover = SDL_PointInRect((SDL_Point*)&ms, (SDL_Rect*)&element.rect);
            // border
            guiInstance->GUI_SetMainColorRGB(0xa75100);
            SDL_RenderDrawRect(render, (SDL_Rect*)&element.rect);
            // fill
            guiInstance->GUI_SetMainColorRGB(*hover ? input::isMouseDown() ? 0xab0000 : 0xe70000 : 0xc90000);
            rect = element.rect;
            rect += inside / 2;
            SDL_RenderFillRect(render, (SDL_Rect*)&rect);

            // render text
            Render_String(render, element.rect, element.text.c_str(), element.text.size(), 13, TextAlign::MiddleCenter, true, *hover);
            bool msClick = input::isMouseUp();
            return (*hover) && msClick;
        }

        case CEDIT: {
            return false;
        }

        case CHSLIDER: {
            float* value = (float*)element.resources;
            // Minimal support

            return false;
        }

        case CVSLIDER: {
            float* value = (float*)element.resources;
            // Minimal support

            return false;
        }

        case CIMAGE: {
            SDL_Rect sdlr;
            Texture* tex = reinterpret_cast<Texture*>(element.resources);
            if (tex) {
                sdlr.x = element.rect.x;
                sdlr.y = element.rect.y;
                sdlr.w = element.rect.w == 0 ? tex->width() : element.rect.w;
                sdlr.h = element.rect.h == 0 ? tex->height() : element.rect.h;

                SDL_RenderCopy(render, tex->native(), nullptr, &sdlr);
            }
            return false;
        }
        case CIMAGEANIMATOR: {
            Timeline* timeline = (Timeline*)element.resources;
            Texture* texture = timeline->Evaluate(Time::time())->texture;
            SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&element.rect);
            return false;
        }
        case CTEXTRAND: {
            static float lasttime = 0;

            // todo: Доработать рандомайзера!
            TextRandomizer_Format format = TextRandomizer_Format::OnlyNumber;  // (int)(data.resources);

            element.text.resize(15);
            size_t i;

            if (Time::time() > lasttime) {
                lasttime = Time::time() + 0.1f;
                switch (format) {
                    case TextRandomizer_Format::OnlyNumber:
                        for (i = 0; i < element.text.length(); ++i) {
                            element.text[i] = '0' + (rand() % ('9'));
                        }
                        break;
                    case TextRandomizer_Format::All:
                    default:
                        for (i = 0; i < element.text.length(); ++i) {
                            element.text[i] = 32 + rand() % 128;
                        }
                        break;
                }
            }

            Render_String(Application::GetRenderer(), element.rect, element.text.c_str(), element.text.size());

            return false;
        }
        case CDROPDOWN: {
            return false;
        }
    }

    return false;
}

//***************** destructors

// void UC::destructor(void* resources) {}
// void CText::destructor(void* resources) {}
// void CButton::destructor(void* resources) {}
// void CEdit::destructor(void* resources) {}
// void CHorizontalSlider::destructor(void* resources) {}
// void CVerticalSlider::destructor(void* resources) {}
// void CImage::destructor(void* resources) {}
// void CImageAnimator::destructor(void* resources) {}
// void CTextRandomizerDisplay::destructor(void* resources) {}

}  // namespace RoninEngine::UI
