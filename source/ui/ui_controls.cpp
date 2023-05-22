#include <SDL2/SDL2_gfxPrimitives.h>
#include "ronin.h"

// TODO: Change ALL interface SDL_Color as Color

#define DROPDOWN_RESOURCE std::pair<int, std::list<std::string>>

namespace RoninEngine::Runtime
{
    extern void text_get(std::string& text);
    extern void text_start_input();
    extern void text_stop_input();
}

namespace RoninEngine::UI
{
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
        // TODO: free controls?
        TTF_CloseFont(pfont);
        TTF_Quit();
    }

    void ui_reset_controls() { _controlId = 0; }

    void* factory_resource(GUIControlPresents type)
    {
        void* resources;

        switch (type) {
        case RGUI_DROPDOWN:
            resources = RoninMemory::alloc<DROPDOWN_RESOURCE>();
            break;
        case RGUI_IMAGEANIMATOR:
            resources = RoninMemory::alloc<Timeline>();
            break;
        case RGUI_HSLIDER:
            // value, min, max members
            resources = RoninMemory::ronin_memory_alloc(sizeof(float) * 3);
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
        case RGUI_IMAGEANIMATOR:
            RoninMemory::free(static_cast<Timeline*>(element->resources));
            break;
        case RGUI_HSLIDER:
            RoninMemory::ronin_memory_free(element->resources);
            break;
        }
    }

    bool general_control_default_state() { return false; }

    bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* render, const bool uiHover, bool& focus)
    {
        static float dropDownLinear = 0;
        Vec2Int ms = Input::get_mouse_point();
        bool result = false;
        {
            // TODO: general drawing

            if (uiHover) {
                if (Input::get_key_down(SDL_SCANCODE_LCTRL)) {
                    element.rect.x = ms.x - element.rect.w / 2;
                    element.rect.y = ms.y - element.rect.h / 2;
                }
            }
        }

        switch (element.prototype) {
        case RoninEngine::UI::_UC:
            break;
        case RGUI_TEXT: {
            DrawFontAt(render, element.text, 15, element.rect.getXY(), Color::white);
            // Render_String(render, element.rect, element.text.c_str(), element.text.size());
            break;
        }

        case RGUI_BUTTON: {
            static uint8_t pSize = 2; // pen size
            static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
            Rect rect;

            // border
            gui->set_color_rgb(0xa75100);
            SDL_RenderDrawRect(render, (SDL_Rect*)&element.rect);
            // fill
            gui->set_color_rgb(uiHover ? Input::is_mouse_down() ? 0xab0000 : 0xe70000 : 0xc90000);
            rect = element.rect;
            rect += inside / 2;
            SDL_RenderFillRect(render, (SDL_Rect*)&rect);

            // render text
            // Render_String(render, element.rect, element.text.c_str(), element.text.size(), 13, TextAlign::MiddleCenter, true, uiHover);
            DrawFontAt(render, element.text, 12, element.rect.getXY(), Color::white);
            bool msClick = Input::is_mouse_up();
            result = uiHover && msClick;
            break;
        }

        case RGUI_EDIT: {
            // uielement background
            static const int thickness = 2;
            static const Rect thick = { thickness, thickness, -thickness, -thickness };
            bool msClick = uiHover && Input::is_mouse_up();
            Rect r = element.rect;
            r += thick;

            // uielement background
            Gizmos::set_color((uiHover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
            SDL_RenderFillRect(render, (SDL_Rect*)&r);
            Gizmos::set_color(Color::gray);
            for (int x = 0; x < thickness; ++x) {
                r -= Rect(1, 1, -1, -1);
                SDL_RenderDrawRect(render, (SDL_Rect*)&r);
            }

            // draw main text
            Texture* texture;
            SDL_Surface* surf = TTF_RenderUTF8_Solid(pfont, element.text.c_str(), *reinterpret_cast<SDL_Color*>(&colorSpace.editText));
            if (ResourceManager::gc_alloc_texture_from(&texture, surf) != GCInvalidID) {
                r = element.rect;
                r.x += 5;
                r.y += +r.h / 2 - texture->height() / 2;
                r.w = texture->width();
                r.h = texture->height();

                SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&(r));

                RoninMemory::free(texture);
                SDL_FreeSurface(surf);
            }

            // focusing intersection
            if (focus) {
                std::string linput;
                RoninEngine::Runtime::text_get(linput);
                element.text += linput;
            } else {
                // clik and focused
                if (focus = result = msClick) {
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
            bool msClick = uiHover && Input::is_mouse_down();
            // clamp mouse point an inside
            ms.x = Math::clamp(ms.x, element.rect.x, element.rect.x + element.rect.w);
            ms.y = Math::clamp(ms.y, element.rect.y, element.rect.y + element.rect.h);
            // get rect
            rect = *reinterpret_cast<SDL_Rect*>(&element.rect);

            if (msClick) {
                focus = msClick;
            }

            if (uiHover && Input::wheel_radix()) {
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

            Gizmos::set_color(uiHover ? Color::lightgray : Color::gray);
            Color color = Gizmos::get_color();
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

        case RGUI_VSLIDER: {
            float* value = (float*)element.resources;
            float* min = value + 1;
            float* max = min + 1;
            *value = Math::clamp(*value, *min, *max);

            // Minimal support

            break;
        }

        case RGUI_IMAGE: {
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
        case RGUI_IMAGEANIMATOR: {
            Timeline* timeline = (Timeline*)element.resources;
            Texture* texture = timeline->Evaluate(TimeEngine::time())->texture;
            SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&element.rect);
            break;
        }
        case RGUI_TEXTRAND: {
            static float lasttime = 0;

            // todo: Доработать рандомайзера!
            TextRandomizer_Format format = TextRandomizer_Format::OnlyNumber; // (int)(data.resources);

            element.text.resize(15);
            size_t i;

            if (TimeEngine::time() > lasttime) {
                lasttime = TimeEngine::time() + 0.1f;
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

            Render_String(Application::get_renderer(), element.rect, element.text.c_str(), element.text.size());

            break;
        }
        case RGUI_DROPDOWN: {
            static const int thickness = 2;
            static const Rect thick = { thickness, thickness, -thickness, -thickness };
            bool msClick = uiHover && Input::is_mouse_up();
            Rect r = element.rect;
            r += thick;

            // uielement background
            Gizmos::set_color((uiHover) ? colorSpace.defaultInteraction.normalState : colorSpace.defaultInteraction.hoverState);
            SDL_RenderFillRect(render, (SDL_Rect*)&r);
            Gizmos::set_color(Color::gray);
            for (int x = 0; x < thickness; ++x) {
                r -= Rect(1, 1, -1, -1);
                SDL_RenderDrawRect(render, (SDL_Rect*)&r);
            }

            // draw main text
            Texture* texture;
            SDL_Surface* surf = TTF_RenderUTF8_Solid(pfont, element.text.c_str(), *reinterpret_cast<SDL_Color*>(&colorSpace.dropdownText));
            if (ResourceManager::gc_alloc_texture_from(&texture, surf) != GCInvalidID) {
                r = element.rect;
                r.x += 5;
                r.y += +r.h / 2 - texture->height() / 2;
                r.w = texture->width();
                r.h = texture->height();

                SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&(r));

                RoninMemory::free(texture);
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

                r.h = dropDownLinear = Math::ceil(Math::lerp_unclamped(dropDownLinear, link->second.size() * sz, 2 * TimeEngine::deltaTime()));

                Gizmos::set_color(colorSpace.defaultInteraction.hoverState);
                // draw background
                SDL_RenderFillRect(render, (SDL_Rect*)&r);
                if (link->second.size() * sz == r.h) {
                    r.h = sz; // set for height
                    elrect = r;
                    r.x += 5;

                    int index = 0;
                    for (auto iter = std::begin(link->second); iter != std::end(link->second); ++iter, ++index) {
                        // draw element highligh
                        if (!result && SDL_PointInRect((SDL_Point*)&(ms), (SDL_Rect*)&elrect)) {
                            Gizmos::set_color(colorSpace.defaultInteraction.pressState);
                            SDL_RenderFillRect(render, (SDL_Rect*)&elrect);
                            if (Input::is_mouse_up()) {
                                link->first = index;
                                element.text = *iter;
                                result = focus = false;
                            }
                        }

                        Gizmos::set_color(colorSpace.defaultInteraction.hoverState);
                        // Draw element text
                        surf = TTF_RenderUTF8_Solid(pfont, iter->c_str(), *reinterpret_cast<SDL_Color*>(&(link->first != index ? colorSpace.dropdownText : colorSpace.dropdownSelectedText)));
                        ResourceManager::gc_alloc_texture_from(&texture, surf);
                        r.h = texture->height();
                        r.w = texture->width();

                        r.y = elrect.y + elrect.h / 2 - r.h / 2;
                        SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&(r));

                        RoninMemory::free(texture);
                        texture = nullptr;
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
                if (focus = result = msClick) { }
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

            ((event_index_changed)(element->event))(element->id, ((DROPDOWN_RESOURCE*)element->resources)->first);
            break;
        }
    }

    void DrawFontAt(SDL_Renderer* renderer, const std::string& text, int fontSize, const Runtime::Vec2Int& screenPoint, const Color color)
    {
        Texture* texture;
        SDL_Rect r;
        SDL_Surface* surf = TTF_RenderUTF8_Solid(pfont, text.c_str(), SDL_Color(*reinterpret_cast<const SDL_Color*>(&color)));
        if (surf) {
            ResourceManager::gc_alloc_texture_from(&texture, surf);
            r.h = texture->height();
            r.w = texture->width();

            r.x = screenPoint.x;
            r.y = screenPoint.y;
            SDL_RenderCopy(renderer, texture->native(), nullptr, &r);
            RoninMemory::free(texture);
            SDL_FreeSurface(surf);
        }
    }

} // namespace RoninEngine::UI
