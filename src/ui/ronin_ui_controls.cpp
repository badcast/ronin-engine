#include <SDL2/SDL2_gfxPrimitives.h>
#include "ronin.h"

#define DROPDOWN_RESOURCE std::pair<int, std::list<std::string>>

namespace RoninEngine::Runtime
{
    extern void text_get(std::string& text);
    extern void text_start_input();
    extern void text_stop_input();
}

namespace RoninEngine::UI
{
    using namespace RoninEngine::Runtime;

    struct ElementInteraction {
        Color normalState;
        Color hoverState;
        Color pressState;
        Color disabledState = Color::darkgray;
    };

    struct {
        ElementInteraction defaultInteraction = { Color::ghostwhite, Color::white, Color::gainsboro };

        Color dropdownText = Color::black;
        Color dropdownSelectedText = Color::lightslategrey;
        Color editText = Color::black;
        Color buttonText = Color::black;
    } colorSpace;

    uid _controlId;
    uid _focusedId;

    TTF_Font* pfont;

    void ui_controls_init()
    {
        TTF_Init();

        // std::string path = getDataFrom(FolderKind::GFX) + "interface/arial.ttf";
        auto raw = SDL_RWFromConstMem(raw_arial_font, raw_arial_length);
        pfont = TTF_OpenFontRW(raw, SDL_TRUE, 14);
    }

    void ui_free_controls()
    {
        TTF_CloseFont(pfont);
        TTF_Quit();
    }

    void ui_reset_controls() { _controlId = 0; }

    ui_resource* factory_resource(GUIControlPresents type)
    {
        ui_resource* resources;

        switch (type) {
        case RGUI_DROPDOWN:
            resources = reinterpret_cast<ui_resource*>(RoninMemory::alloc<DROPDOWN_RESOURCE>());
            break;
        case RGUI_HSLIDER:
            // value, min, max members
            resources = reinterpret_cast<ui_resource*>(RoninMemory::ronin_memory_alloc(sizeof(float) * 3));
            break;
        default:
            resources = nullptr;
            break;
        }
        return resources;
    }

    void factory_free(UIElement* element)
    {
        switch (element->prototype) {
        case RGUI_DROPDOWN:
            RoninMemory::free(static_cast<DROPDOWN_RESOURCE*>(element->resources));
            break;
        case RGUI_HSLIDER:
            RoninMemory::ronin_memory_free(element->resources);
            break;
        }
    }

    bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* renderer, const bool ms_hover, bool& focus)
    {
        // TODO: OPTIMIZE HERE (HIGH PRIORITY)
        static float dropDownLinear = 0;
        Vec2Int ms = Input::get_mouse_point();
        const bool ms_click = ms_hover && Input::is_mouse_up();
        bool result = false;
        //        {
        //            // TODO: general drawing

        //            if (ms_hover) {
        //                if (Input::get_key_down(SDL_SCANCODE_LCTRL)) {
        //                    element.rect.x = ms.x - element.rect.w / 2;
        //                    element.rect.y = ms.y - element.rect.h / 2;
        //                }
        //            }
        //        }

        switch (element.prototype) {
        case RGUI_TEXT: {
            draw_font_at(renderer, element.text, 15, element.rect.getXY(), Color::white);
            // Render_String(render, element.rect, element.text.c_str(), element.text.size());
            break;
        }

        case RGUI_BUTTON: {
            static uint8_t pSize = 2; // pen size
            static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
            Rect rect;

            // fill
            Color col(ms_hover ? Input::is_mouse_down() ? colorSpace.defaultInteraction.hoverState : colorSpace.defaultInteraction.pressState : colorSpace.defaultInteraction.normalState);
            rect = element.rect;
            rect += inside / 2;
            gui->set_color_rgb(col);
            roundedBoxColor(renderer, rect.x, rect.y, rect.w + rect.x, rect.h + rect.y, 4, col);
            // SDL_RenderFillRect(renderer, (SDL_Rect*)&rect);

            // render text
            // Render_String(render, element.rect, element.text.c_str(), element.text.size(), 13, TextAlign::MiddleCenter, true, uiHover);
            draw_font_at(renderer, element.text, 12, rect.getXY() + rect.getWH() / 2, Color::gray, true);
            result = ms_hover && ms_click;
            break;
        }

        case RGUI_EDIT: {
            // uielement background
            static const int thickness = 2;
            static const Rect thick = { thickness, thickness, -thickness, -thickness };
            Rect r = element.rect;
            r += thick;

            // uielement background
            Gizmos::set_color((ms_hover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
            SDL_RenderFillRect(renderer, (SDL_Rect*)&r);
            Gizmos::set_color(Color::gray);
            for (int x = 0; x < thickness; ++x) {
                r -= Rect(1, 1, -1, -1);
                SDL_RenderDrawRect(renderer, (SDL_Rect*)&r);
            }

            // draw main text
            SDL_Texture* texture;
            SDL_Surface* surf = TTF_RenderUTF8_Solid(pfont, element.text.c_str(), *reinterpret_cast<SDL_Color*>(&colorSpace.editText));
            if ((texture = SDL_CreateTextureFromSurface(renderer, surf)) != nullptr) {
                r = element.rect;
                SDL_QueryTexture(texture, nullptr, nullptr, &r.w, &r.h);
                r.x += 5;
                r.y += element.rect.h / 2 - r.h / 2;

                SDL_RenderCopy(renderer, texture, nullptr, (SDL_Rect*)&(r));

                SDL_DestroyTexture(texture);
                SDL_FreeSurface(surf);
            }

            // focusing intersection
            if (focus) {
                std::string linput;
                RoninEngine::Runtime::text_get(linput);
                element.text += linput;
            } else {
                // clik and focused
                if (focus = result = ms_click) {
                    text_start_input();
                } else {
                    text_stop_input();
                }
            }
            break;
        }

        case RGUI_HSLIDER: {
            float ratio;
            SDL_Rect rect;
            float* value = (float*)element.resources;
            float* min = value + 1;
            float* max = min + 1;
            // clamp mouse point an inside
            ms.x = Math::clamp(ms.x, element.rect.x, element.rect.x + element.rect.w);
            ms.y = Math::clamp(ms.y, element.rect.y, element.rect.y + element.rect.h);
            // get rect
            rect = *reinterpret_cast<SDL_Rect*>(&element.rect);

            if (ms_click) {
                focus = ms_click;
            }

            if (ms_hover && Input::wheel_radix()) {
                *value += Input::wheel_radix() / 10.f; // step wheel mouse = ±0.1
                *value = Math::clamp(*value, *min, *max);
                result = true;
            }

            // focused state
            if (focus) {
                if (Input::is_mouse_down()) {
                    // get *x* component from mouse point for set cursor point
                    *value = Math::map((float)ms.x, (float)rect.x, (float)rect.x + rect.w, *min, *max);
                }

                result = true;
            } else
                *value = Math::clamp(*value, *min, *max);

            ratio = Math::map(*value, *min, *max, 0.f, 1.f);

            rect.h = 4;
            rect.y += (element.rect.h - rect.h) / 2;

            Gizmos::set_color(ms_hover ? Color::lightgray : Color::gray);
            Color color = Gizmos::get_color();
            roundedBoxColor(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 3, Color::slategray);
            roundedRectangleColor(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 3, color);

            // draw cursor
            rect.w = 9;
            rect.h = 11;
            rect.x += (int)element.rect.w * ratio - rect.w / 2;
            rect.y = element.rect.y + element.rect.h / 2 - rect.h / 2;
            color = Color::darkgray;
            roundedBoxColor(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 3, color);
            color = Color::gray;
            roundedRectangleColor(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, 3, color);

            // Draw text
            char __[32];

            TTF_SizeText(pfont, __, &rect.w, &rect.h);
            sprintf(__, "%.1f", *value);
            Vec2Int tpos = { element.rect.x + element.rect.w - 22, element.rect.y + element.rect.h - 8 };
            draw_font_at(renderer, __, 1, tpos, color);

            break;
        }

        case RGUI_VSLIDER: {
            float* value = (float*)element.resources;
            float* min = value + 1;
            float* max = min + 1;
            *value = Math::clamp(*value, *min, *max);

            break;
        }

        case RGUI_IMAGE: {
            SDL_Rect sdlr;
            Sprite* sprite = reinterpret_cast<Sprite*>(element.resources);
            if (sprite && sprite->surface) {
                sdlr.x = element.rect.x;
                sdlr.y = element.rect.y;
                sdlr.w = element.rect.w == 0 ? sprite->width() : element.rect.w;
                sdlr.h = element.rect.h == 0 ? sprite->height() : element.rect.h;
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, sprite->surface);
                SDL_RenderCopy(renderer, tex, nullptr, &sdlr);
                SDL_DestroyTexture(tex);
            }
            break;
        }
        case RGUI_DROPDOWN: {
            static const int thickness = 2;
            static const Rect thick = { thickness, thickness, -thickness, -thickness };
            Rect r = element.rect;
            r += thick;

            // uielement background
            Gizmos::set_color((ms_hover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
            SDL_RenderFillRect(renderer, (SDL_Rect*)&r);
            Gizmos::set_color(Color::gray);
            for (int x = 0; x < thickness; ++x) {
                r -= Rect(1, 1, -1, -1);
                SDL_RenderDrawRect(renderer, (SDL_Rect*)&r);
            }

            // draw main text
            SDL_Texture* texture;
            SDL_Surface* surf = TTF_RenderUTF8_Blended(pfont, element.text.c_str(), *reinterpret_cast<SDL_Color*>(&colorSpace.dropdownText));
            if ((texture = SDL_CreateTextureFromSurface(renderer, surf)) != nullptr) {
                r = element.rect;
                r.x += 5;
                SDL_QueryTexture(texture, nullptr, nullptr, &r.w, &r.h);
                r.y += element.rect.h / 2 - r.h / 2;

                SDL_RenderCopy(renderer, texture, nullptr, (SDL_Rect*)&(r));

                SDL_DestroyTexture(texture);
                texture = nullptr;
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

                r.h = dropDownLinear = Math::ceil(Math::lerp(dropDownLinear, link->second.size() * sz, TimeEngine::deltaTime()));

                Gizmos::set_color(colorSpace.defaultInteraction.hoverState);
                // draw background
                SDL_RenderFillRect(renderer, (SDL_Rect*)&r);
                if (link->second.size() * sz == r.h) {
                    r.h = sz; // set for height
                    elrect = r;
                    r.x += 5;

                    int index = 0;
                    for (auto iter = std::begin(link->second); iter != std::end(link->second); ++iter, ++index) {
                        // draw element highligh
                        if (!result && SDL_PointInRect((SDL_Point*)&(ms), (SDL_Rect*)&elrect)) {
                            Gizmos::set_color(colorSpace.defaultInteraction.pressState);
                            SDL_RenderFillRect(renderer, (SDL_Rect*)&elrect);
                            if (Input::is_mouse_down()) {
                                link->first = index;
                                element.text = *iter;
                                result = true;
                                focus = false;
                                break;
                            }
                        }

                        Gizmos::set_color(colorSpace.defaultInteraction.hoverState);
                        // Draw element text
                        surf = TTF_RenderUTF8_Blended(pfont, iter->c_str(), *reinterpret_cast<SDL_Color*>(&(link->first != index ? colorSpace.dropdownText : colorSpace.dropdownSelectedText)));
                        texture = SDL_CreateTextureFromSurface(renderer, surf);
                        SDL_QueryTexture(texture, nullptr, nullptr, &r.w, &r.h);

                        r.y = elrect.y + elrect.h / 2 - r.h / 2;
                        SDL_RenderCopy(renderer, texture, nullptr, (SDL_Rect*)&(r));

                        SDL_DestroyTexture(texture);
                        texture = nullptr;
                        SDL_FreeSurface(surf);
                        elrect.y += sz;
                    }
                }

                if (ms_click) {
                    focus = false;
                    dropDownLinear = 0;
                }

            } else {
                // clik and shown
                if (ms_click)
                    focus = ms_click;
            }
            break;
        }
        }

        return result;
    }

    void event_action(UIElement* element)
    {
        if (!element->event)
            return;

        switch (element->prototype) {
        case RGUI_DROPDOWN:
            ((ui_callback_integer)(element->event))(element->id, ((DROPDOWN_RESOURCE*)element->resources)->first);
            break;
        case RGUI_BUTTON:
            ((ui_callback)(element->event))(element->id, element->resources);
            break;
        }
    }

    void draw_font_at(SDL_Renderer* renderer, const std::string& text, int fontSize, Runtime::Vec2Int screenPoint, const Color color, bool alignCenter)
    {
        SDL_Rect r;

        SDL_Surface* surf = TTF_RenderUTF8_Blended(pfont, text.c_str(), color);
        if (surf) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
            r.w = surf->w;
            r.h = surf->h;

            r.x = screenPoint.x;
            r.y = screenPoint.y;
            if (alignCenter) {
                r.x -= r.w / 2;
                r.y -= r.h / 2;
            }
            SDL_RenderCopy(renderer, texture, nullptr, &r);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surf);
        }
    }

} // namespace RoninEngine::UI
