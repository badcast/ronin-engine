#include "ronin.h"
#include "ronin_ui_resources.h"
#include "ronin_render_cache.h"

// TODO: Make UI Cache

namespace RoninEngine::Runtime
{
    extern TTF_Font *pDefaultTTFFont;
    extern font2d_t *f2d_default;

    extern void text_get(std::string &text);

    static void utf8_pop_back(std::string &s)
    {
        if(s.empty()) return;
        while(!s.empty() && ((static_cast<unsigned char>(s.back()) & 0xC0) == 0x80))
        {
            s.pop_back();
        }
        if(!s.empty())
            s.pop_back();
    }
} // namespace RoninEngine::Runtime

namespace RoninEngine::UI
{
    using namespace RoninEngine::Runtime;

    bool general_ui_render(UIElement &element, const UIState &uiState, bool &ui_focus)
    {
        bool result = false;
        bool isEnabled = (element.options & ElementEnableMask) && uiState.gui->handle->interactable;
        SDL_Texture *texture;
        SDL_Surface *surface;
        Rect rect;

        switch(element.prototype)
        {
            case RGUI_TEXT:
            {
                RenderUtility::SetColor(isEnabled ? Color::white : colorSpace.defaultInteraction.disabledState);
                int fSize = element.fontWidth > 0 ? element.fontWidth : 15;
                Render_String_ttf(element.text.data(), fSize, element.rect.getXY(), false);
                if(isEnabled && uiState.ms_click)
                    result = true;
                break;
            }

            case RGUI_BUTTON:
            {
                static uint8_t pSize = 2; // pen size
                static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
                bool show_down_side = isEnabled && (Input::GetMouseDown(MouseButton::MouseLeft) == false || !uiState.ms_hover);

                // fill
                Color col;
                if(!isEnabled)
                    col = colorSpace.defaultInteraction.disabledState;
                else if(uiState.ms_hover && Input::GetMouseDown(MouseButton::MouseLeft))
                    col = colorSpace.defaultInteraction.pressState;
                else if(uiState.ms_hover)
                    col = colorSpace.defaultInteraction.hoverState;
                else
                    col = colorSpace.defaultInteraction.normalState;

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

                RenderUtility::SetColor(isEnabled ? colorSpace.buttonText : Color::gray);
                Render_String_ttf(element.text.data(), 12, rect.getXY() + (rect.getWH() - (show_down_side ? (Vec2Int::up * 4) : Vec2Int::zero)) / 2, true);
                if(isEnabled && (result = uiState.ms_click))
                    uiState.gui->handle->button_clicked.insert(element.id);
                break;
            }

            case RGUI_TEXT_EDIT:
            {
                static const int thickness = 2;
                static const Rect thick = {thickness, thickness, -thickness, -thickness};
                static const Rect thickPer {1, 1, -1, -1};
                rect = element.rect + thick;

                RenderUtility::SetColor(isEnabled ? ((uiState.ms_hover || ui_focus) ? colorSpace.defaultInteraction.hoverState : colorSpace.defaultInteraction.normalState) : colorSpace.defaultInteraction.disabledState);
                SDL_RenderFillRect(gscope.renderer, (SDL_Rect *) &rect);
                RenderUtility::SetColor(Color::gray);
                for(int x = 0; x < thickness; ++x)
                {
                    rect -= thickPer;
                    SDL_RenderDrawRect(gscope.renderer, (SDL_Rect *) &rect);
                }

                if(isEnabled)
                {
                    if(!ui_focus && uiState.ms_click)
                    {
                        ui_focus = true;
                        std::string flushInput;
                        RoninEngine::Runtime::text_get(flushInput); // flush old keypresses
                    }
                    else if(ui_focus)
                    {
                        if(Input::GetKeyDown(KB_BACKSPACE))
                        {
                            utf8_pop_back(element.text);
                            result = true;
                        }
                        if(Input::GetKeyDown(KB_RETURN) || Input::GetKeyDown(KB_KP_ENTER) || Input::GetKeyDown(KB_ESCAPE))
                        {
                            ui_focus = false;
                        }

                        std::string linput;
                        RoninEngine::Runtime::text_get(linput);
                        if(!linput.empty())
                        {
                            element.text += linput;
                            result = true;
                        }
                    }
                }
                else
                {
                    ui_focus = false;
                }

                // draw main text
                int editFontSize = element.fontWidth > 0 ? element.fontWidth : 13;
                TTF_SetFontSize(pDefaultTTFFont, editFontSize);
                std::string displayText = element.text;
                if(ui_focus && static_cast<int>(Time::time() * 2.0f) % 2 == 0)
                    displayText += "|";

                if(!displayText.empty())
                {
                    surface = TTF_RenderUTF8_Solid(pDefaultTTFFont, displayText.c_str(), *reinterpret_cast<SDL_Color *>(&colorSpace.editText));
                    if(surface)
                    {
                        texture = SDL_CreateTextureFromSurface(gscope.renderer, surface);
                        if(texture)
                        {
                            SDL_Rect srcR = {0, 0, surface->w, surface->h};
                            rect = element.rect;
                            rect.x += 5;
                            rect.y += (element.rect.h - surface->h) / 2;
                            rect.w = std::min(surface->w, std::max(1, element.rect.w - 10));
                            rect.h = surface->h;
                            srcR.w = rect.w;

                            SDL_RenderCopy(gscope.renderer, texture, &srcR, (SDL_Rect *) &rect);
                            SDL_DestroyTexture(texture);
                        }
                        SDL_FreeSurface(surface);
                    }
                }
                break;
            }

            case RGUI_HSLIDER:
            {
                SliderResource *slider = &element.resource.slider;
                float prevVal = slider->value;
                rect = *reinterpret_cast<SDL_Rect *>(&element.rect);

                if(isEnabled)
                {
                    if(uiState.ms_hover && Input::GetMouseWheel() != 0)
                    {
                        float delta = (slider->max - slider->min) * slider->stepPercentage * Input::GetMouseWheel();
                        slider->value = Math::Clamp(slider->value + delta, slider->min, slider->max);
                        result = (slider->value != prevVal);
                    }
                    else if(uiState.ms_hover && Input::GetMouseDown(MouseButton::MouseLeft))
                    {
                        ui_focus = true;
                        int msX = Math::Clamp(uiState.ms.x, element.rect.x, element.rect.x + element.rect.w);
                        float ratio = (element.rect.w > 0) ? (static_cast<float>(msX - element.rect.x) / element.rect.w) : 0.0f;
                        slider->value = Math::Clamp(Math::Lerp(slider->min, slider->max, ratio), slider->min, slider->max);
                        result = (slider->value != prevVal);
                    }
                }

                float ratio = (slider->max > slider->min) ? ((slider->value - slider->min) / (slider->max - slider->min)) : 0.0f;
                ratio = Math::Clamp01(ratio);

                rect.h = 4;
                rect.y += (element.rect.h - rect.h) / 2;

                RenderUtility::SetColor(isEnabled ? (uiState.ms_hover ? Color::lightgray : Color::gray) : colorSpace.defaultInteraction.disabledState);
                Color color = RenderUtility::GetColor();
                boxRGBA(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, Color::slategray.r, Color::slategray.g, Color::slategray.b, Color::slategray.a);
                rectangleRGBA(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color.r, color.g, color.b, color.a);

                // Cursor
                rect.w = 20;
                rect.h = 11;
                rect.x = element.rect.x + static_cast<int>(element.rect.w * ratio) - rect.w / 2;
                rect.y = element.rect.y + (element.rect.h - rect.h) / 2;

                color = isEnabled ? Color::darkgray : Color::lightgray;
                boxRGBA(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color.r, color.g, color.b, color.a);
                color = isEnabled ? (uiState.ms_hover ? Color::lightgray : Color::gray) : Color::darkgray;
                rectangleRGBA(gscope.renderer, rect.x, rect.y, rect.x + rect.w + 1, rect.y + rect.h + 1, color.r, color.g, color.b, color.a);

                RenderUtility::SetColor(Color::black);
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.1f", slider->value);
                rect.x += rect.w / 2;
                rect.y += rect.h / 2;
                Render_String_ttf(buffer, 9, rect.getXY(), true);
                break;
            }

            case RGUI_VSLIDER:
            {
                SliderResource *slider = &element.resource.slider;
                float prevVal = slider->value;
                rect = *reinterpret_cast<SDL_Rect *>(&element.rect);

                if(isEnabled)
                {
                    if(uiState.ms_hover && Input::GetMouseWheel() != 0)
                    {
                        float delta = (slider->max - slider->min) * slider->stepPercentage * Input::GetMouseWheel();
                        slider->value = Math::Clamp(slider->value + delta, slider->min, slider->max);
                        result = (slider->value != prevVal);
                    }
                    else if(uiState.ms_hover && Input::GetMouseDown(MouseButton::MouseLeft))
                    {
                        ui_focus = true;
                        int msY = Math::Clamp(uiState.ms.y, element.rect.y, element.rect.y + element.rect.h);
                        // Bottom to top
                        float ratio = (element.rect.h > 0) ? (static_cast<float>((element.rect.y + element.rect.h) - msY) / element.rect.h) : 0.0f;
                        slider->value = Math::Clamp(Math::Lerp(slider->min, slider->max, ratio), slider->min, slider->max);
                        result = (slider->value != prevVal);
                    }
                }

                float ratio = (slider->max > slider->min) ? ((slider->value - slider->min) / (slider->max - slider->min)) : 0.0f;
                ratio = Math::Clamp01(ratio);

                rect.w = 4;
                rect.x += (element.rect.w - rect.w) / 2;

                RenderUtility::SetColor(isEnabled ? (uiState.ms_hover ? Color::lightgray : Color::gray) : colorSpace.defaultInteraction.disabledState);
                Color color = RenderUtility::GetColor();
                boxRGBA(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, Color::slategray.r, Color::slategray.g, Color::slategray.b, Color::slategray.a);
                rectangleRGBA(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color.r, color.g, color.b, color.a);

                // Cursor
                rect.w = 11;
                rect.h = 20;
                rect.x = element.rect.x + (element.rect.w - rect.w) / 2;
                rect.y = element.rect.y + static_cast<int>(element.rect.h * (1.0f - ratio)) - rect.h / 2;

                color = isEnabled ? Color::darkgray : Color::lightgray;
                boxRGBA(gscope.renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, color.r, color.g, color.b, color.a);
                color = isEnabled ? (uiState.ms_hover ? Color::lightgray : Color::gray) : Color::darkgray;
                rectangleRGBA(gscope.renderer, rect.x, rect.y, rect.x + rect.w + 1, rect.y + rect.h + 1, color.r, color.g, color.b, color.a);

                RenderUtility::SetColor(Color::black);
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.1f", slider->value);
                rect.x += rect.w / 2;
                rect.y += rect.h / 2;
                Render_String_ttf(buffer, 9, rect.getXY(), true);
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
                    SDL_Texture *tex = render_cache_texture(sprite);
                    if(tex)
                    {
                        SDL_Rect srcR = *reinterpret_cast<SDL_Rect *>(&sprite->m_rect);
                        SDL_RenderCopy(gscope.renderer, tex, &srcR, &sdlr);
                        render_cache_unref(tex);
                    }
                }
                if(isEnabled && uiState.ms_click)
                    result = true;
                break;
            }

            case RGUI_DROPDOWN:
            {
                static const int thickness = 2;
                static const Rect thick = {thickness, thickness, -thickness, -thickness};
                Rect r = element.rect + thick;

                RenderUtility::SetColor(isEnabled ? ((uiState.ms_hover || ui_focus) ? colorSpace.defaultInteraction.hoverState : colorSpace.defaultInteraction.normalState) : colorSpace.defaultInteraction.disabledState);
                SDL_RenderFillRect(gscope.renderer, (SDL_Rect *) &r);
                RenderUtility::SetColor(Color::gray);
                for(int x = 0; x < thickness; ++x)
                {
                    r -= Rect(1, 1, -1, -1);
                    SDL_RenderDrawRect(gscope.renderer, (SDL_Rect *) &r);
                }

                // draw header text
                TTF_SetFontSize(pDefaultTTFFont, 13);
                SDL_Surface *surf = TTF_RenderUTF8_Blended(pDefaultTTFFont, element.text.c_str(), *reinterpret_cast<SDL_Color *>(&colorSpace.dropdownText));
                if(surf)
                {
                    texture = SDL_CreateTextureFromSurface(gscope.renderer, surf);
                    if(texture)
                    {
                        r = element.rect;
                        r.x += 5;
                        SDL_QueryTexture(texture, nullptr, nullptr, &r.w, &r.h);
                        r.y += (element.rect.h - r.h) / 2;
                        SDL_RenderCopy(gscope.renderer, texture, nullptr, (SDL_Rect *) &r);
                        SDL_DestroyTexture(texture);
                    }
                    SDL_FreeSurface(surf);
                }

                DropDownResource *link = element.resource.dropdown;
                if(!link)
                    break;

                if(isEnabled && uiState.ms_click && Vec2Int::HasIntersection(uiState.ms, element.rect))
                {
                    ui_focus = !ui_focus;
                }

                static const int sz = 24;
                float targetH = (ui_focus && isEnabled) ? static_cast<float>(link->second.size() * sz) : 0.0f;
                link->animHeight = Math::MoveTowards(link->animHeight, targetH, std::max(1.0f, Time::deltaTime() * 800.0f));

                if(link->animHeight > 1.0f)
                {
                    element.contextRect = Rect(element.rect.x, element.rect.y + element.rect.h, element.rect.w, static_cast<int>(link->animHeight));

                    r = element.contextRect;
                    RenderUtility::SetColor(colorSpace.defaultInteraction.normalState);
                    SDL_RenderFillRect(gscope.renderer, (SDL_Rect *) &r);
                    RenderUtility::SetColor(Color::darkgray);
                    SDL_RenderDrawRect(gscope.renderer, (SDL_Rect *) &r);

                    Rect elrect = {element.rect.x, element.rect.y + element.rect.h, element.rect.w, sz};
                    int index = 0;
                    for(auto iter = link->second.begin(); iter != link->second.end(); ++iter, ++index)
                    {
                        if(elrect.y + elrect.h > element.contextRect.y + element.contextRect.h + 2)
                            break;

                        bool itemHover = Vec2Int::HasIntersection(uiState.ms, elrect);
                        if(itemHover)
                        {
                            RenderUtility::SetColor(colorSpace.defaultInteraction.hoverState);
                            SDL_RenderFillRect(gscope.renderer, (SDL_Rect *) &elrect);
                            if(isEnabled && Input::GetMouseUp(MouseButton::MouseLeft))
                            {
                                link->first = index;
                                element.text = *iter;
                                result = true;
                                ui_focus = false;
                                link->animHeight = 0.0f;
                                element.contextRect = Rect::zero;
                                break;
                            }
                        }

                        surf = TTF_RenderUTF8_Blended(pDefaultTTFFont, iter->c_str(), *reinterpret_cast<SDL_Color *>(&(link->first == index ? colorSpace.dropdownSelectedText : colorSpace.dropdownText)));
                        if(surf)
                        {
                            texture = SDL_CreateTextureFromSurface(gscope.renderer, surf);
                            if(texture)
                            {
                                SDL_Rect textR = {elrect.x + 5, elrect.y + (elrect.h - surf->h) / 2, surf->w, surf->h};
                                SDL_RenderCopy(gscope.renderer, texture, nullptr, &textR);
                                SDL_DestroyTexture(texture);
                            }
                            SDL_FreeSurface(surf);
                        }

                        elrect.y += sz;
                    }
                }
                else
                {
                    element.contextRect = Rect::zero;
                }
                break;
            }

            case RGUI_CHECKBOX:
            {
                Rect startRect = element.rect;
                if(isEnabled && uiState.ms_hover && Input::GetMouseDown(MouseButton::MouseLeft))
                    startRect.y++;

                rect = startRect;

                RenderUtility::SetColor(isEnabled ? (uiState.ms_hover ? Color::white : Color::gray) : colorSpace.defaultInteraction.disabledState);
                Render_String_ttf(element.text.c_str(), 14, rect.getXY() + Vec2Int {17, 0}, false);

                rect.w = 14;
                rect.h = 14;
                SDL_RenderFillRect(gscope.renderer, reinterpret_cast<SDL_Rect *>(&rect));

                if(isEnabled && (result = uiState.ms_click))
                {
                    element.resource.checkbox = !element.resource.checkbox;
                }

                if(element.resource.checkbox)
                {
                    RenderUtility::SetColor(0xFF3a3a3a);

                    rect.w = 4;
                    rect.h = 12;
                    rect.x += 7 - 2;
                    rect.y++;
                    SDL_RenderFillRect(gscope.renderer, reinterpret_cast<SDL_Rect *>(&rect));

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
                Sprite *sprite = nullptr;
                if(!isEnabled && element.resource.spriteButton.disable)
                    sprite = element.resource.spriteButton.disable;
                else if(uiState.ms_hover && element.resource.spriteButton.hover)
                    sprite = element.resource.spriteButton.hover;
                else
                    sprite = element.resource.spriteButton.normal;

                Vec2Int baseSize = element.rect.getWH();
                if(sprite && sprite->surface)
                {
                    SDL_Texture *texture = render_cache_texture(sprite);
                    if(texture)
                    {
                        render_texture_extension(texture, &baseSize, reinterpret_cast<SDL_Rect *>(&sprite->m_rect), reinterpret_cast<SDL_Rect *>(&element.rect), 0.0F);
                        render_cache_unref(texture);
                    }
                }

                Sprite *icon = element.resource.spriteButton.icon;
                if(icon && icon->surface)
                {
                    SDL_Texture *texture = render_cache_texture(icon);
                    if(texture)
                    {
                        render_texture_extension(texture, &baseSize, reinterpret_cast<SDL_Rect *>(&icon->m_rect), reinterpret_cast<SDL_Rect *>(&element.rect), 0.0F);
                        render_cache_unref(texture);
                    }
                }

                if(isEnabled && (result = uiState.ms_click))
                {
                    uiState.gui->handle->button_clicked.insert(element.id);
                }
                break;
            }
            case RGUI_CUSTOM_OVERLAY:
                UIData data {};
                data.id = element.id;
                data.rect = element.rect;
                data.isMouseDown = isEnabled && uiState.ms_click;
                if((data.isMouseHover = uiState.ms_hover))
                    data.mousePoint = uiState.ms;
                if(!element.resource.overlay.isInit && element.resource.overlay.inspector)
                {
                    element.resource.overlay.isInit = true;
                    element.resource.overlay.inspector->OnInit();
                }
                if(element.resource.overlay.inspector)
                    element.resource.overlay.inspector->OnDraw(&data);
                if(isEnabled && uiState.ms_click)
                    result = true;
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
            case RGUI_TEXT:
            case RGUI_PICTURE_BOX:
            case RGUI_CUSTOM_OVERLAY:
                (reinterpret_cast<UIEventVoid>(element->event))(element->id);
                break;
            case RGUI_DROPDOWN:
                if(element->resource.dropdown)
                    (reinterpret_cast<UIEventInteger>(element->event))(element->id, (element->resource.dropdown->first));
                break;
            case RGUI_HSLIDER:
            case RGUI_VSLIDER:
                (reinterpret_cast<UIEventFloat>(element->event))(element->id, element->resource.slider.value);
                break;
            case RGUI_CHECKBOX:
                (reinterpret_cast<UIEventBool>(element->event))(element->id, element->resource.checkbox);
                break;
            case RGUI_TEXT_EDIT:
                break;
        }
    }

    void Render_String_ttf(const char *text, int fontSize, const Runtime::Vec2Int &screenPoint, bool alignCenter, bool blend)
    {
        if(text == nullptr || *text == '\0' || pDefaultTTFFont == nullptr || gscope.renderer == nullptr)
            return;

        TTF_SetFontSize(pDefaultTTFFont, fontSize);

        SDL_Surface *surf = blend ? TTF_RenderUTF8_Blended(pDefaultTTFFont, text, RenderUtility::GetColor()) : TTF_RenderUTF8_Solid(pDefaultTTFFont, text, RenderUtility::GetColor());
        if(surf)
        {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(gscope.renderer, surf);
            if(texture)
            {
                SDL_Rect r {screenPoint.x, screenPoint.y, surf->w, surf->h};
                if(alignCenter)
                {
                    r.x -= r.w / 2;
                    r.y -= r.h / 2;
                }
                SDL_RenderCopy(gscope.renderer, texture, nullptr, &r);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surf);
        }
    }

    int Single_TextWidth_WithCyrilic(const std::string &text, int fontSize)
    {
        if(!f2d_default || !f2d_default->data)
            return 0;

        int width = 0;
        for(auto iter = begin(text); iter != end(text); ++iter)
        {
            width += f2d_default->data[(unsigned char) *iter].w;
        }
        return width;
    }
} // namespace RoninEngine::UI
