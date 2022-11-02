#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>

#include "FontInternal.h"
#include "framework.h"
#include "inputSystem.h"

#define DROPDOWN_RESOURCE std::pair<int, std::list<std::string>>

namespace RoninEngine::UI {
struct ElementInteraction {
    Color normalState;
    Color hoverState;
    Color pressState;
    Color disabledState = Color::darkgray;
};

struct {
    ElementInteraction defaultInteraction = {Color::ghostwhite, Color::white, Color::gainsboro};
    Color dropdownText = Color::black;
    Color dropdownSelectedText = Color::lightslategrey;
    Color editText = Color::black;
} colorSpace;

uid _controlId;
uid _focusedId;

TTF_Font* pfont;

void InitalizeControls() {
    TTF_Init();

    // std::string path = getDataFrom(FolderKind::GFX) + "interface/arial.ttf";
    auto raw = SDL_RWFromConstMem(raw_arial_font, raw_arial_length);
    pfont = TTF_OpenFontRW(raw, SDL_TRUE, 14);
}

void Free_Controls() {
    // TODO: free controls?
    TTF_CloseFont(pfont);
    TTF_Quit();
}

void ResetControls() { _controlId = 0; }

void* factory_resource(ControlType type) {
    void* resources;

    switch (type) {
        case CDROPDOWN:
            resources = GC::gc_alloc<DROPDOWN_RESOURCE>();
            break;
        case CIMAGEANIMATOR:
            resources = GC::gc_alloc<Timeline>();
            break;
        case CHSLIDER:
            // value, min, max members
            resources = GC::gc_malloc(sizeof(float) * 3);
            break;
        default:
            resources = nullptr;
            break;
    }
    return resources;
}

void factory_free(UIElement* element) {
    switch (element->prototype) {
        case CDROPDOWN:
            GC::gc_unalloc(static_cast<DROPDOWN_RESOURCE*>(element->resources));
            break;
        case CIMAGEANIMATOR:
            GC::gc_unalloc(static_cast<Timeline*>(element->resources));
            break;
        case CHSLIDER:
            GC::gc_free(element->resources);
            break;
    }
}

bool general_control_default_state() {}

bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* render, const bool uiHover, bool& focus) {
    static float dropDownLinear = 0;
    Vec2Int ms = input::getMousePoint();
    bool result = false;
    {
        // TODO: general drawing

        if (uiHover) {
            if (input::get_key_down(SDL_SCANCODE_LCTRL)) {
                element.rect.x = ms.x - element.rect.w / 2;
                element.rect.y = ms.y - element.rect.h / 2;
            }
        }
    }

    switch (element.prototype) {
        case RoninEngine::UI::_UC:
            break;
        case CTEXT: {
            Render_String(render, element.rect, element.text.c_str(), element.text.size());
            break;
        }

        case CBUTTON: {
            static uint8_t pSize = 2;  // pen size
            static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
            Rect rect;

            // border
            guiInstance->GUI_SetMainColorRGB(0xa75100);
            SDL_RenderDrawRect(render, (SDL_Rect*)&element.rect);
            // fill
            guiInstance->GUI_SetMainColorRGB(uiHover ? input::isMouseDown() ? 0xab0000 : 0xe70000 : 0xc90000);
            rect = element.rect;
            rect += inside / 2;
            SDL_RenderFillRect(render, (SDL_Rect*)&rect);

            // render text
            Render_String(render, element.rect, element.text.c_str(), element.text.size(), 13, TextAlign::MiddleCenter, true,
                          uiHover);
            bool msClick = input::isMouseUp();
            result = uiHover && msClick;
            break;
        }

        case CEDIT: {
            // uielement background
            static const int thickness = 2;
            static const Rect thick = {thickness, thickness, -thickness, -thickness};
            bool msClick = uiHover && input::isMouseUp();
            Rect r = element.rect;
            r += thick;

            // uielement background
            Gizmos::setColor((uiHover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
            SDL_RenderFillRect(render, (SDL_Rect*)&r);
            Gizmos::setColor(Color::gray);
            for (int x = 0; x < thickness; ++x) {
                r -= Rect(1, 1, -1, -1);
                SDL_RenderDrawRect(render, (SDL_Rect*)&r);
            }

            // draw main text
            Texture* texture;
            SDL_Surface* surf = TTF_RenderUTF8_Solid(pfont, element.text.c_str(), colorSpace.editText);
            if (GC::gc_alloc_texture_from(&texture, surf) != GCInvalidID) {
                r = element.rect;
                r.x += 5;
                r.y += +r.h / 2 - texture->height() / 2;
                r.w = texture->width();
                r.h = texture->height();

                SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&(r));

                GC::gc_unalloc(texture);
                SDL_FreeSurface(surf);
            }

            // focusing intersection
            if (focus) {
                std::string linput;
                input::text_get(linput);
                element.text += linput;
            } else {
                // clik and focused
                if (focus = result = msClick) {
                    input::text_start_input();
                } else {
                    input::text_stop_input();
                }
            }
            break;
        }

        case CHSLIDER: {
            float ratio;
            SDL_Rect rect;
            float* value = (float*)element.resources;
            float* min = value + 1;
            float* max = min + 1;
            bool msClick = uiHover && input::isMouseDown();
            // clamp mouse point an inside
            ms.x = Math::Clamp(ms.x, element.rect.x, element.rect.x + element.rect.w);
            ms.y = Math::Clamp(ms.y, element.rect.y, element.rect.y + element.rect.h);
            // get rect
            rect = *reinterpret_cast<SDL_Rect*>(&element.rect);

            if (msClick) {
                focus = msClick;
            }

            if (uiHover && input::wheelRadix()) {
                *value += input::wheelRadix() / 10.f;  // step wheel mouse = ±0.1
                *value = Math::Clamp(*value, *min, *max);
                result = true;
            }

            // focused state
            if (focus) {
                if (input::isMouseDown()) {
                    // get *x* component from mouse point for set cursor point
                    *value = Math::map((float)ms.x, (float)rect.x, (float)rect.x + rect.w, *min, *max);
                }

                result = true;
            } else
                *value = Math::Clamp(*value, *min, *max);

            ratio = Math::map(*value, *min, *max, 0.f, 1.f);

            rect.h = 4;
            rect.y += (element.rect.h - rect.h) / 2;

            Gizmos::setColor(uiHover ? Color::lightgray : Color::gray);
            Color color = Gizmos::getColor();
            roundedBoxColor(render, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 3, Color::slategray);
            roundedRectangleColor(render, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 3, color);
            // SDL_RenderFillRect(render, &rect);

            // draw cursor
            rect.w = 9;
            rect.h = 11;
            rect.x += (int)element.rect.w * ratio - rect.w / 2;
            rect.y = element.rect.y + element.rect.h / 2 - rect.h / 2;
            color = Color::darkgray;
            roundedBoxColor(render, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 3, color);
            color = Color::gray;
            roundedRectangleColor(render, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 3, color);
            break;
        }

        case CVSLIDER: {
            float* value = (float*)element.resources;
            float* min = value + 1;
            float* max = min + 1;
            *value = Math::Clamp(*value, *min, *max);

            // Minimal support

            break;
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
            break;
        }
        case CIMAGEANIMATOR: {
            Timeline* timeline = (Timeline*)element.resources;
            Texture* texture = timeline->Evaluate(Time::time())->texture;
            SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&element.rect);
            break;
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

            break;
        }
        case CDROPDOWN: {
            static const int thickness = 2;
            static const Rect thick = {thickness, thickness, -thickness, -thickness};
            bool msClick = uiHover && input::isMouseUp();
            Rect r = element.rect;
            r += thick;

            // uielement background
            Gizmos::setColor((uiHover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
            SDL_RenderFillRect(render, (SDL_Rect*)&r);
            Gizmos::setColor(Color::gray);
            for (int x = 0; x < thickness; ++x) {
                r -= Rect(1, 1, -1, -1);
                SDL_RenderDrawRect(render, (SDL_Rect*)&r);
            }

            // draw main text
            Texture* texture;
            SDL_Surface* surf = TTF_RenderUTF8_Solid(pfont, element.text.c_str(), colorSpace.dropdownText);
            if (GC::gc_alloc_texture_from(&texture, surf) != GCInvalidID) {
                r = element.rect;
                r.x += 5;
                r.y += +r.h / 2 - texture->height() / 2;
                r.w = texture->width();
                r.h = texture->height();

                SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&(r));

                GC::gc_unalloc(texture);
                SDL_FreeSurface(surf);
            }
            // get resources
            auto link = static_cast<std::pair<int, std::list<std::string>>*>(element.resources);

            // show dropdown list
            if (focus) {
                static int sz = 15;
                Rect elrect;
                r = element.rect;
                r.y += r.h;

                r.h = dropDownLinear =
                    Math::ceil(Math::LerpUnclamped(dropDownLinear, link->second.size() * sz, 2 * Time::deltaTime()));

                Gizmos::setColor(colorSpace.defaultInteraction.hoverState);
                // draw background
                SDL_RenderFillRect(render, (SDL_Rect*)&r);
                if (link->second.size() * sz == r.h) {
                    r.h = sz;  // set for height
                    elrect = r;
                    r.x += 5;

                    int index = 0;
                    for (auto iter = std::begin(link->second); iter != std::end(link->second); ++iter, ++index) {
                        // draw element highligh
                        if (!result && SDL_PointInRect((SDL_Point*)&(ms), (SDL_Rect*)&elrect)) {
                            Gizmos::setColor(colorSpace.defaultInteraction.pressState);
                            SDL_RenderFillRect(render, (SDL_Rect*)&elrect);
                            if (input::isMouseUp()) {
                                link->first = index;
                                element.text = *iter;
                                result = focus = false;
                            }
                        }

                        Gizmos::setColor(colorSpace.defaultInteraction.hoverState);
                        // Draw element text
                        surf = TTF_RenderUTF8_Solid(
                            pfont, iter->c_str(),
                            link->first != index ? colorSpace.dropdownText : colorSpace.dropdownSelectedText);
                        GC::gc_alloc_texture_from(&texture, surf);
                        r.h = texture->height();
                        r.w = texture->width();

                        r.y = elrect.y + elrect.h / 2 - r.h / 2;
                        SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&(r));

                        GC::gc_unalloc(texture);
                        SDL_FreeSurface(surf);
                        elrect.y += sz;
                    }
                }

                if (msClick) {
                    focus = false;
                    dropDownLinear = 0;
                }

            } else {
                // clik and shown
                if (focus = result = msClick) {
                }
            }
            break;
        }
    }

    return result;
}

void event_action(UIElement* element) {
    if (!element->event) return;

    switch (element->prototype) {
        case CDROPDOWN:

            ((event_index_changed)(element->event))(element->id, ((DROPDOWN_RESOURCE*)element->resources)->first);
            break;
    }
}

void DrawFontAt(SDL_Renderer* renderer, const std::string& text, int fontSize, const Runtime::Vec2Int& screenPoint) {
    Texture* texture;
    SDL_Rect r;
    SDL_Surface* surf = TTF_RenderUTF8_Solid(pfont, text.c_str(), Color::white);
    GC::gc_alloc_texture_from(&texture, surf);
    r.h = texture->height();
    r.w = texture->width();

    r.x = screenPoint.x;
    r.y = screenPoint.y;
    SDL_RenderCopy(renderer, texture->native(), nullptr, &r);
    GC::gc_unalloc(texture);
    SDL_FreeSurface(surf);
}

}  // namespace RoninEngine::UI
