#include "ronin.h"
#include "ronin_ui_resources.h"

// TODO: Make UI Cache

namespace RoninEngine::Runtime
{
    extern void text_get(std::string &text);
    extern void text_start_input();
    extern void text_stop_input();
} // namespace RoninEngine::Runtime

namespace RoninEngine::UI
{
    using namespace RoninEngine::Runtime;

    std::uint8_t legacyVH[] {0, 32, 16, 2, 34, 18, 1, 33, 17};

    extern TTF_Font *ttf_arial_font;
    extern LegacyFont_t *pLegacyFont;

    bool general_render_ui_section(UIElement &element, const UIState &uiState, bool &ui_focus)
    {
        // TODO: OPTIMIZE HERE (HIGH PRIORITY)
        static float dropDownLinear = 0;
        static Color colors[3];
        bool result = false;
        Vec2Int ms = Input::GetMousePoint();
        SDL_Texture *texture;
        SDL_Surface *surface;
        Rect rect;

        // TODO: general drawing
        // if (ms_hover) {
        //     if (Input::get_key_down(SDL_SCANCODE_LCTRL)) {
        //         element.rect.x = ms.x - element.rect.w / 2;
        //         element.rect.y = ms.y - element.rect.h / 2;
        //     }
        // }

        switch(element.prototype)
        {
            case RGUI_TEXT:
            {
                Gizmos::SetColor(Color::white);
                Render_String_ttf(element.text.data(), 15, element.rect.GetXY(), Color::white);
                // Render_String(render, element.rect, element.text.c_str(), element.text.size());
                break;
            }

            case RGUI_BUTTON:
            {
                static uint8_t pSize = 2; // pen size
                static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
                bool show_down_side = Input::GetMouseDown(MouseState::MouseLeft) == false || !uiState.ms_hover;

                // fill
                Color col(uiState.ms_hover ? colorSpace.defaultInteraction.pressState : colorSpace.defaultInteraction.normalState);
                rect = element.rect;
                rect += inside / 2;
                Gizmos::SetColor(col);

                if(show_down_side)
                {
                    Gizmos::SetColor(colorSpace.buttonDownSide);
                    SDL_RenderFillRect(renderer, reinterpret_cast<SDL_Rect *>(&rect));
                }

                Gizmos::SetColor(col);
                Rect region {rect};
                region.h -= (show_down_side ? 4 : 0);
                SDL_RenderFillRect(renderer, reinterpret_cast<SDL_Rect *>(&region));

                // SDL_RenderFillRect(renderer, (SDL_Rect*)&rect);

                // render text
                // Render_String(render, element.rect, element.text.c_str(), element.text.size(), 13,
                // TextAlign::MiddleCenter, true, uiHover);
                Gizmos::SetColor(Color::gray);
                Render_String_ttf(
                    element.text.data(),
                    12,
                    rect.GetXY() + (rect.GetWH() - (show_down_side ? (Vec2Int::up * 4) : Vec2Int::zero)) / 2,
                    true);
                if((result = uiState.ms_click))
                    uiState.gui->handle->button_clicked.insert(element.id);
                break;
            }

            case RGUI_TEXT_EDIT:
            {
                // uielement background
                static const int thickness = 2;
                static const Rect thick = {thickness, thickness, -thickness, -thickness};
                static const Rect thickPer {1, 1, -1, -1};
                rect = element.rect + thick;

                // uielement background
                Gizmos::SetColor((uiState.ms_hover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
                SDL_RenderFillRect(renderer, (SDL_Rect *) &rect);
                Gizmos::SetColor(Color::gray);
                for(int x = 0; x < thickness; ++x)
                {
                    rect -= thickPer;
                    SDL_RenderDrawRect(renderer, (SDL_Rect *) &rect);
                }

                // draw main text
                surface = TTF_RenderUTF8_Solid(ttf_arial_font, element.text.c_str(), *reinterpret_cast<SDL_Color *>(&colorSpace.editText));
                if((texture = SDL_CreateTextureFromSurface(renderer, surface)) != nullptr)
                {
                    rect = element.rect;
                    SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);
                    rect.x += 5;
                    rect.y += element.rect.h / 2 - rect.h / 2;

                    SDL_RenderCopy(renderer, texture, nullptr, (SDL_Rect *) &(rect));

                    SDL_DestroyTexture(texture);
                    SDL_FreeSurface(surface);
                }

                // focusing intersection
                if(ui_focus)
                {
                    std::string linput;
                    RoninEngine::Runtime::text_get(linput);
                    element.text += linput;
                }
                else
                {
                    // clik and focused
                    if(ui_focus = result = uiState.ms_click)
                    {
                        text_start_input();
                    }
                    else
                    {
                        text_stop_input();
                    }
                }
                break;
            }

            case RGUI_HSLIDER:
            {
                float ratio;
                SliderResource *resource = &element.resource.slider;
                // clamp mouse point an inside
                ms.x = Math::Clamp(ms.x, element.rect.x, element.rect.x + element.rect.w);
                ms.y = Math::Clamp(ms.y, element.rect.y, element.rect.y + element.rect.h);
                // get rect
                rect = *reinterpret_cast<SDL_Rect *>(&element.rect);

                if(uiState.ms_click)
                {
                    ui_focus = uiState.ms_click;
                }

                if(uiState.ms_hover && Input::GetMouseWheel())
                {
                    // step wheel mouse = ±0.1
                    float percentage = (resource->min + resource->max) * resource->stepPercentage * Input::GetMouseWheel();
                    resource->value = Math::Clamp(resource->value + percentage, resource->min, resource->max);
                }
                else if(ui_focus || uiState.ms_hover)
                {
                    if((result = Input::GetMouseDown(MouseState::MouseLeft)))
                    {
                        // get *x* component from mouse point for set cursor point
                        resource->value =
                            Math::Map<float>((float) ms.x, (float) rect.x, (float) rect.x + rect.w, resource->min, resource->max);
                    }
                }

                // Draw Slider Background

                ratio = Math::Map(resource->value, resource->min, resource->max, 0.f, 1.f);

                rect.h = 4;
                rect.y += (element.rect.h - rect.h) / 2;

                Gizmos::SetColor(uiState.ms_hover ? Color::lightgray : Color::gray);
                Color color = Gizmos::GetColor();
                boxColor(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, Color::slategray);
                rectangleColor(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color);

                // Draw Slider Cursor

                // Cursor width
                rect.w = 20;
                // Cursor Height
                rect.h = 11;
                // Cursor Horizontal position
                rect.x += (int) element.rect.w * ratio - rect.w / 2;
                // Cursor Vertical position
                rect.y = element.rect.y + (element.rect.h - rect.h) / 2;

                // Cursor background
                color = Color::darkgray;
                boxColor(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color);

                // Cursor thickness
                color = uiState.ms_hover ? Color::lightgray : Color::gray;
                rectangleColor(renderer, rect.x, rect.y, rect.x + rect.w + 1, rect.y + rect.h + 1, color);

                // Draw Text under Cursor point
                Gizmos::SetColor(Color::black);
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.1f", resource->value);
                rect.x += rect.w / 2;
                rect.y += rect.h / 2;
                Render_String_ttf(buffer, 9, rect.GetXY(), true);

                break;
            }

            case RGUI_VSLIDER:
            {
                break;
            }

            case RGUI_PICTURE_BOX:
            {
                SDL_Rect sdlr;
                Sprite *sprite = element.resource.picturebox;
                if(sprite && sprite->surface)
                {
                    sdlr.x = element.rect.x;
                    sdlr.y = element.rect.y;
                    sdlr.w = element.rect.w == 0 ? sprite->width() : element.rect.w;
                    sdlr.h = element.rect.h == 0 ? sprite->height() : element.rect.h;
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, sprite->surface);
                    SDL_RenderCopy(renderer, tex, nullptr, &sdlr);
                    SDL_DestroyTexture(tex);
                }
                break;
            }

            case RGUI_DROPDOWN:
            {
                static const int thickness = 2;
                static const Rect thick = {thickness, thickness, -thickness, -thickness};
                Rect r = element.rect;
                r += thick;

                // uielement background
                Gizmos::SetColor((uiState.ms_hover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
                SDL_RenderFillRect(renderer, (SDL_Rect *) &r);
                Gizmos::SetColor(Color::gray);
                for(int x = 0; x < thickness; ++x)
                {
                    r -= Rect(1, 1, -1, -1);
                    SDL_RenderDrawRect(renderer, (SDL_Rect *) &r);
                }

                // draw main text
                SDL_Texture *texture;
                SDL_Surface *surf =
                    TTF_RenderUTF8_Blended(ttf_arial_font, element.text.c_str(), *reinterpret_cast<SDL_Color *>(&colorSpace.dropdownText));
                if((texture = SDL_CreateTextureFromSurface(renderer, surf)) != nullptr)
                {
                    r = element.rect;
                    r.x += 5;
                    SDL_QueryTexture(texture, nullptr, nullptr, &r.w, &r.h);
                    r.y += element.rect.h / 2 - r.h / 2;

                    SDL_RenderCopy(renderer, texture, nullptr, (SDL_Rect *) &(r));

                    SDL_DestroyTexture(texture);
                    texture = nullptr;
                    SDL_FreeSurface(surf);
                }
                // get resources
                DropDownResource *link = element.resource.dropdown;

                // show dropdown list
                if(ui_focus)
                {
                    static int sz = 15;
                    Rect elrect;
                    r = element.rect;
                    r.y += r.h;

                    r.h = dropDownLinear = Math::Ceil(Math::Lerp(dropDownLinear, link->second.size() * sz, TimeEngine::deltaTime()));

                    Gizmos::SetColor(colorSpace.defaultInteraction.hoverState);
                    // draw background
                    SDL_RenderFillRect(renderer, (SDL_Rect *) &r);
                    if(link->second.size() * sz == r.h)
                    {
                        r.h = sz; // set for height
                        elrect = r;
                        r.x += 5;

                        int index = 0;
                        for(auto iter = std::begin(link->second); iter != std::end(link->second); ++iter, ++index)
                        {
                            // draw element highligh
                            if(!result && SDL_PointInRect((SDL_Point *) &(ms), (SDL_Rect *) &elrect))
                            {
                                Gizmos::SetColor(colorSpace.defaultInteraction.pressState);
                                SDL_RenderFillRect(renderer, (SDL_Rect *) &elrect);
                                if(Input::GetMouseDown(MouseState::MouseLeft))
                                {
                                    link->first = index;
                                    element.text = *iter;
                                    result = true;
                                    ui_focus = false;
                                    break;
                                }
                            }

                            Gizmos::SetColor(colorSpace.defaultInteraction.hoverState);
                            // Draw element text
                            surf = TTF_RenderUTF8_Blended(
                                ttf_arial_font,
                                iter->c_str(),
                                *reinterpret_cast<SDL_Color *>(
                                    &(link->first != index ? colorSpace.dropdownText : colorSpace.dropdownSelectedText)));
                            texture = SDL_CreateTextureFromSurface(renderer, surf);
                            SDL_QueryTexture(texture, nullptr, nullptr, &r.w, &r.h);

                            r.y = elrect.y + elrect.h / 2 - r.h / 2;
                            SDL_RenderCopy(renderer, texture, nullptr, (SDL_Rect *) &(r));

                            SDL_DestroyTexture(texture);
                            texture = nullptr;
                            SDL_FreeSurface(surf);
                            elrect.y += sz;
                        }
                    }

                    if(uiState.ms_click)
                    {
                        ui_focus = false;
                        dropDownLinear = 0;
                    }
                }
                else
                {
                    // clik and shown
                    if(uiState.ms_click)
                        ui_focus = uiState.ms_click;
                }
                break;
            }

            case RGUI_CHECKBOX:
            {
                rect = element.rect;

                Gizmos::SetColor(uiState.ms_hover ? Color::white : Color::gray);
                Render_String_ttf(element.text.c_str(), 14, rect.GetXY() + Vec2Int {17, 0}, false);

                rect.w = 14;
                rect.h = 14;
                SDL_RenderFillRect(renderer, reinterpret_cast<SDL_Rect *>(&rect));

                if((result = uiState.ms_click))
                {
                    element.resource.checkbox = !element.resource.checkbox;
                }

                if(element.resource.checkbox)
                {
                    Gizmos::SetColor(0xFF3a3a3a);

                    // Vertical
                    rect.w = 4;
                    rect.h = 12;
                    rect.x += 7 - 2;
                    rect.y++;
                    SDL_RenderFillRect(renderer, reinterpret_cast<SDL_Rect *>(&rect));

                    // Horizontal
                    rect.h = 4;
                    rect.w = 12;
                    rect.y = element.rect.y + 7 - 2;
                    rect.x = element.rect.x + 1;
                    SDL_RenderFillRect(renderer, reinterpret_cast<SDL_Rect *>(&rect));
                }
                break;
            }
        }

        return result;
    }

    void ui_event_action(UIElement *element)
    {
        if(element->event == nullptr)
            return;

        switch(element->prototype)
        {
            case RGUI_BUTTON:
                (reinterpret_cast<UIEventVoid>(element->event))(element->id);
                break;
            case RGUI_DROPDOWN:
                (reinterpret_cast<UIEventInteger>(element->event))(element->id, (element->resource.dropdown->first));
                break;
            case RGUI_HSLIDER:
            case RGUI_VSLIDER:
                (reinterpret_cast<UIEventFloat>(element->event))(element->id, element->resource.slider.value);
                break;
            case RGUI_TEXT:
            case RGUI_TEXT_EDIT:
            case RGUI_PICTURE_BOX:
            case RGUI_CHECKBOX:
                break;
        }
    }

    void Render_String_ttf(const char *text, int fontSize, const Runtime::Vec2Int &screenPoint, bool alignCenter)
    {
        SDL_Texture *texture;
        SDL_Surface *surf;

        TTF_SetFontSize(ttf_arial_font, fontSize);
        surf = TTF_RenderUTF8_Blended(ttf_arial_font, text, Gizmos::GetColor());
        if(surf)
        {
            SDL_Rect r {screenPoint.x, screenPoint.y, surf->w, surf->h};
            texture = SDL_CreateTextureFromSurface(renderer, surf);

            if(alignCenter)
            {
                r.x -= r.w / 2;
                r.y -= r.h / 2;
            }
            SDL_RenderCopy(renderer, texture, nullptr, &r);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surf);
        }
    }

    int Single_TextWidth_WithCyrilic(const std::string &text, int fontSize)
    {
        int width = 0;
        for(auto iter = begin(text); iter != end(text); ++iter)
            // TODO: Учитывать размер шрифта (fontSize)
            width += pLegacyFont->data[(unsigned char) *iter].w;
        return width;
    }

    void Render_String_Legacy(Rect rect, const char *text, int len, int fontWidth, Align textAlign, bool textWrap, bool hilight)
    {
        if(text == nullptr || len <= 0)
            return;

        std::uint8_t temp;
        Rect *src;
        std::uint16_t pos;
        SDL_Rect dst = *reinterpret_cast<SDL_Rect *>(&rect);

        Vec2Int fontSize = pLegacyFont->fontSize + Vec2Int::one * (fontWidth - pLegacyFont->fontSize.x);
        int textWidth = Single_TextWidth_WithCyrilic(text, fontWidth);

        if(!rect.w)
            rect.w = textWidth;
        if(!rect.h)
            rect.h = pLegacyFont->fontSize.y;

        // x
        temp = (legacyVH[textAlign] >> 4 & 15);
        if(temp)
        {
            dst.x += rect.w / temp;
            if(temp == 2)
                dst.x += -textWidth / 2;
            else if(temp == 1)
                dst.x += -textWidth;
        }
        // y
        temp = (legacyVH[textAlign] & 15);
        if(temp)
        {
            dst.y += rect.h / temp - fontSize.y / 2;
            if(temp == 1)
                dst.y += -textWidth / 2;
        }

        Vec2Int begin = *reinterpret_cast<Vec2Int *>(&dst);
        int deltax;
        for(pos = 0; pos < len; ++pos)
        {
            memcpy(&temp, text + pos, 1);
            src = (pLegacyFont->data + temp);
            if(temp != '\n')
            {
                dst.w = src->w;
                dst.h = src->h;
                deltax = rect.x + rect.w;

                if(dst.x >= deltax)
                {
                    for(++pos; pos < len;)
                    {
                        temp = *(text + pos);
                        if(temp != '\n')
                            ++pos;
                        else
                            break;
                    }
                    continue;
                }

                dst.w = Math::Max(0, Math::Min(deltax - dst.x, dst.w));
                SDL_RenderCopy(
                    renderer,
                    (hilight ? switched_world->irs->legacy_font_hover : switched_world->irs->legacy_font_normal),
                    reinterpret_cast<SDL_Rect *>(src),
                    &dst);
                dst.x += src->w;
            }
            else
            {
                dst.x = begin.x;
                dst.y += src->h;
            }
        }
    }

} // namespace RoninEngine::UI
