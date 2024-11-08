#include "ronin.h"
#include "ronin_ui_resources.h"
#include "ronin_render_cache.h"

// TODO: Make UI Cache

namespace RoninEngine::Runtime
{
    extern TTF_Font *pDefaultTTFFont;
    extern font2d_t *f2d_default;

    extern void text_get(std::string &text);

} // namespace RoninEngine::Runtime

namespace RoninEngine::UI
{
    using namespace RoninEngine::Runtime;

    bool general_ui_render(UIElement &element, const UIState &uiState, bool &ui_focus)
    {
        // TODO: OPTIMIZE HERE (HIGH PRIORITY)
        static float dropDownLinear = 0;
        bool result = false;
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
                RenderUtility::SetColor(Color::white);
                Render_String_ttf(element.text.data(), 15, element.rect.getXY(), Color::white);
                // Render_String(render, element.rect, element.text.c_str(), element.text.size());
                break;
            }

            case RGUI_BUTTON:
            {
                static uint8_t pSize = 2; // pen size
                static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
                bool show_down_side = Input::GetMouseDown(MouseButton::MouseLeft) == false || !uiState.ms_hover;

                // fill
                Color col(uiState.ms_hover ? colorSpace.defaultInteraction.pressState : colorSpace.defaultInteraction.normalState);
                rect = element.rect;
                rect += inside / 2;
                RenderUtility::SetColor(col);

                if(show_down_side)
                {
                    RenderUtility::SetColor(colorSpace.buttonDownSide);
                    SDL_RenderFillRect(gscope.renderer, reinterpret_cast<SDL_Rect *>(&rect));
                }

                RenderUtility::SetColor(col);
                Rect region {rect};
                region.h -= (show_down_side ? 4 : 0);
                SDL_RenderFillRect(gscope.renderer, reinterpret_cast<SDL_Rect *>(&region));

                // SDL_RenderFillRect(renderer, (SDL_Rect*)&rect);

                // render text
                // Render_String(render, element.rect, element.text.c_str(), element.text.size(), 13,
                // TextAlign::MiddleCenter, true, uiHover);
                RenderUtility::SetColor(Color::gray);
                Render_String_ttf(element.text.data(), 12, rect.getXY() + (rect.getWH() - (show_down_side ? (Vec2Int::up * 4) : Vec2Int::zero)) / 2, true);
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
                RenderUtility::SetColor((uiState.ms_hover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
                SDL_RenderFillRect(gscope.renderer, (SDL_Rect *) &rect);
                RenderUtility::SetColor(Color::gray);
                for(int x = 0; x < thickness; ++x)
                {
                    rect -= thickPer;
                    SDL_RenderDrawRect(gscope.renderer, (SDL_Rect *) &rect);
                }

                // draw main text
                surface = TTF_RenderUTF8_Solid(pDefaultTTFFont, element.text.c_str(), *reinterpret_cast<SDL_Color *>(&colorSpace.editText));
                if((texture = SDL_CreateTextureFromSurface(gscope.renderer, surface)) != nullptr)
                {
                    rect = element.rect;
                    SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);
                    rect.x += 5;
                    rect.y += element.rect.h / 2 - rect.h / 2;

                    SDL_RenderCopy(gscope.renderer, texture, nullptr, (SDL_Rect *) &(rect));

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
                    if((ui_focus = result = uiState.ms_click))
                    {
                        // SDL_StartTextInput();
                    }
                    else
                    {
                        // SDL_StopTextInput();
                    }
                }
                break;
            }

            case RGUI_HSLIDER:
            {
                float ratio;
                Vec2Int ms;
                // clamp mouse point an inside
                ms.x = Math::Clamp(uiState.ms.x, element.rect.x, element.rect.x + element.rect.w);
                ms.y = Math::Clamp(uiState.ms.y, element.rect.y, element.rect.y + element.rect.h);
                // get rect
                rect = *reinterpret_cast<SDL_Rect *>(&element.rect);

                if(uiState.ms_click)
                {
                    ui_focus = uiState.ms_click;
                }

                if(uiState.ms_hover && Input::GetMouseWheel())
                {
                    // step wheel mouse = ±0.1
                    float percentage = ((&element.resource.slider)->min + (&element.resource.slider)->max) * (&element.resource.slider)->stepPercentage * Input::GetMouseWheel();
                    (&element.resource.slider)->value = Math::Clamp((&element.resource.slider)->value + percentage, (&element.resource.slider)->min, (&element.resource.slider)->max);
                }
                else if(ui_focus || uiState.ms_hover)
                {
                    if((result = Input::GetMouseDown(MouseButton::MouseLeft)))
                    {
                        // get *x* component from mouse point for set cursor point
                        (&element.resource.slider)->value = Math::Map<float>((float) ms.x, (float) rect.x, (float) rect.x + rect.w, (&element.resource.slider)->min, (&element.resource.slider)->max);
                    }
                }

                // Draw Slider Background

                ratio = Math::Map((&element.resource.slider)->value, (&element.resource.slider)->min, (&element.resource.slider)->max, 0.f, 1.f);

                rect.h = 4;
                rect.y += (element.rect.h - rect.h) / 2;

                RenderUtility::SetColor(uiState.ms_hover ? Color::lightgray : Color::gray);
                Color color = RenderUtility::GetColor();
                boxColor(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, Color::slategray);
                rectangleColor(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color);

                // Draw Slider Cursor

                // Cursor width
                rect.w = 20;
                // Cursor Height
                rect.h = 11;
                // Cursor Horizontal position
                rect.x += (int) element.rect.w * ratio - rect.w / 2;
                // Cursor Vertical position
                rect.y = element.rect.y + (element.rect.h - rect.h) / 2;

                // is mouse down
                if(result)
                {
                    rect.y += 2;
                }

                // Cursor background
                color = Color::darkgray;
                boxColor(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color);

                // Cursor thickness
                color = uiState.ms_hover ? Color::lightgray : Color::gray;
                rectangleColor(gscope.renderer, rect.x, rect.y, rect.x + rect.w + 1, rect.y + rect.h + 1, color);

                // Draw Text under Cursor point
                RenderUtility::SetColor(Color::black);
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.1f", (&element.resource.slider)->value);
                rect.x += rect.w / 2;
                rect.y += rect.h / 2;
                Render_String_ttf(buffer, 9, rect.getXY(), true);

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
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(gscope.renderer, sprite->surface);
                    SDL_RenderCopy(gscope.renderer, tex, nullptr, &sdlr);
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
                RenderUtility::SetColor((uiState.ms_hover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
                SDL_RenderFillRect(gscope.renderer, (SDL_Rect *) &r);
                RenderUtility::SetColor(Color::gray);
                for(int x = 0; x < thickness; ++x)
                {
                    r -= Rect(1, 1, -1, -1);
                    SDL_RenderDrawRect(gscope.renderer, (SDL_Rect *) &r);
                }

                // draw main text
                SDL_Texture *texture;
                SDL_Surface *surf = TTF_RenderUTF8_Blended(pDefaultTTFFont, element.text.c_str(), *reinterpret_cast<SDL_Color *>(&colorSpace.dropdownText));
                if((texture = SDL_CreateTextureFromSurface(gscope.renderer, surf)) != nullptr)
                {
                    r = element.rect;
                    r.x += 5;
                    SDL_QueryTexture(texture, nullptr, nullptr, &r.w, &r.h);
                    r.y += element.rect.h / 2 - r.h / 2;

                    SDL_RenderCopy(gscope.renderer, texture, nullptr, (SDL_Rect *) &(r));

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

                    r.h = dropDownLinear = Math::Ceil(Math::Lerp(dropDownLinear, link->second.size() * sz, Time::deltaTime()));

                    RenderUtility::SetColor(colorSpace.defaultInteraction.hoverState);
                    // draw background
                    SDL_RenderFillRect(gscope.renderer, (SDL_Rect *) &r);
                    if(link->second.size() * sz == r.h)
                    {
                        r.h = sz; // set for height
                        elrect = r;
                        r.x += 5;

                        int index = 0;
                        for(auto iter = std::begin(link->second); iter != std::end(link->second); ++iter, ++index)
                        {
                            // draw element highligh
                            if(!result && SDL_PointInRect((SDL_Point *) &(uiState.ms), (SDL_Rect *) &elrect))
                            {
                                RenderUtility::SetColor(colorSpace.defaultInteraction.pressState);
                                SDL_RenderFillRect(gscope.renderer, (SDL_Rect *) &elrect);
                                if(Input::GetMouseDown(MouseButton::MouseLeft))
                                {
                                    link->first = index;
                                    element.text = *iter;
                                    result = true;
                                    ui_focus = false;
                                    break;
                                }
                            }

                            RenderUtility::SetColor(colorSpace.defaultInteraction.hoverState);
                            // Draw element text
                            surf = TTF_RenderUTF8_Blended(pDefaultTTFFont, iter->c_str(), *reinterpret_cast<SDL_Color *>(&(link->first != index ? colorSpace.dropdownText : colorSpace.dropdownSelectedText)));
                            texture = SDL_CreateTextureFromSurface(gscope.renderer, surf);
                            SDL_QueryTexture(texture, nullptr, nullptr, &r.w, &r.h);

                            r.y = elrect.y + elrect.h / 2 - r.h / 2;
                            SDL_RenderCopy(gscope.renderer, texture, nullptr, (SDL_Rect *) &(r));

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
                Rect startRect = element.rect;
                if(uiState.ms_hover && Input::GetMouseDown(MouseButton::MouseLeft))
                    startRect.y++;

                rect = startRect;

                RenderUtility::SetColor(uiState.ms_hover ? Color::white : Color::gray);
                Render_String_ttf(element.text.c_str(), 14, rect.getXY() + Vec2Int {17, 0}, false);

                rect.w = 14;
                rect.h = 14;
                SDL_RenderFillRect(gscope.renderer, reinterpret_cast<SDL_Rect *>(&rect));

                if((result = uiState.ms_click))
                {
                    element.resource.checkbox = !element.resource.checkbox;
                }

                if(element.resource.checkbox)
                {
                    RenderUtility::SetColor(0xFF3a3a3a);

                    // Vertical
                    rect.w = 4;
                    rect.h = 12;
                    rect.x += 7 - 2;

                    rect.y++;
                    SDL_RenderFillRect(gscope.renderer, reinterpret_cast<SDL_Rect *>(&rect));

                    // Horizontal
                    rect.h = 4;
                    rect.w = 12;
                    rect.y = startRect.y + 7 - 2;
                    rect.x = startRect.x + 1;
                    SDL_RenderFillRect(gscope.renderer, reinterpret_cast<SDL_Rect *>(&rect));
                }
                break;
            }

            case RGUI_SPRITE_BUTTON:
            {
                Sprite *sprite = uiState.ms_hover ? element.resource.spriteButton.hover : element.resource.spriteButton.normal;

                SDL_Texture *texture = render_cache_texture(sprite);

                Vec2Int baseSize = element.rect.getWH();
                render_texture_extension(texture, &baseSize, reinterpret_cast<SDL_Rect *>(&sprite->m_rect), reinterpret_cast<SDL_Rect *>(&element.rect), 0.0F);
                render_cache_unref(texture);

                sprite = element.resource.spriteButton.icon;
                if(sprite == nullptr)
                    break;

                texture = render_cache_texture(sprite);
                render_texture_extension(texture, &baseSize, reinterpret_cast<SDL_Rect *>(&sprite->m_rect), reinterpret_cast<SDL_Rect *>(&element.rect), 0.0F);
                render_cache_unref(texture);

                result = uiState.ms_click;

                break;
            }
            case RGUI_CUSTOM_OVERLAY:
                UIData data {};
                data.id = element.id;
                data.rect = element.rect;
                data.isMouseDown = uiState.ms_click;
                if(data.isMouseHover = uiState.ms_hover)
                    data.mousePoint = uiState.ms;
                if(!element.resource.overlay.isInit)
                {
                    element.resource.overlay.isInit = true;
                    element.resource.overlay.inspector->OnInit();
                }
                element.resource.overlay.inspector->OnDraw(&data);
                break;
        }

        return result;
    }

    void general_ui_event_action(UIElement *element)
    {
        if(element->event == nullptr)
            return;

        switch(element->prototype)
        {
            case RGUI_BUTTON:
            case RGUI_SPRITE_BUTTON:
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

    void Render_String_ttf(const char *text, int fontSize, const Runtime::Vec2Int &screenPoint, bool alignCenter, bool blend)
    {
        SDL_Texture *texture;
        SDL_Surface *surf;

        TTF_SetFontSize(pDefaultTTFFont, fontSize);

        surf = blend ? TTF_RenderUTF8_Blended(pDefaultTTFFont, text, RenderUtility::GetColor()) : TTF_RenderUTF8_Solid(pDefaultTTFFont, text, RenderUtility::GetColor());
        if(surf)
        {
            SDL_Rect r {screenPoint.x, screenPoint.y, surf->w, surf->h};
            texture = SDL_CreateTextureFromSurface(gscope.renderer, surf);

            if(alignCenter)
            {
                r.x -= r.w / 2;
                r.y -= r.h / 2;
            }
            SDL_RenderCopy(gscope.renderer, texture, nullptr, &r);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surf);
        }
    }

    int Single_TextWidth_WithCyrilic(const std::string &text, int fontSize)
    {
        int width = 0;
        for(auto iter = begin(text); iter != end(text); ++iter)
        {
            width += f2d_default->data[(unsigned char) *iter].w;
        }
        return width;
    }
} // namespace RoninEngine::UI
