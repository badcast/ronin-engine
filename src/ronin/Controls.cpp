#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>

#include "framework.h"
#include "inputSystem.h"

namespace RoninEngine::UI {
uid _controlId;
uid _focusedId;

TTF_Font* font;

class ColorE {};

void InitalizeControls() {
    // TODO: init controls?

    TTF_Init();
    std::string path = getDataFrom(FolderKind::GFX) + "interface/arial.ttf";

    font = TTF_OpenFont(path.c_str(), 12);
}

void Free_Controls() {
    // TODO: free controls?
    TTF_CloseFont(font);
    TTF_Quit();
}

void ResetControls() { _controlId = 0; }

void* factory_resource(ControlType type) {
    void* resources;

    switch (type) {
        case CDROPDOWN:
            resources = GC::gc_alloc<std::pair<int, std::list<std::string>>>();
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
            GC::gc_unalloc(static_cast<std::pair<int, std::list<std::string>>*>(element->resources));
            break;
        case CIMAGEANIMATOR:
            GC::gc_unalloc(static_cast<Timeline*>(element->resources));
            break;
    }
}

bool general_render_ui_section(GUI* gui, UIElement& element, SDL_Renderer* render, const bool hovering, bool& focus) {
    bool result = false;

    {
        // TODO: general drawing
    }

    switch (element.prototype) {
        case CTEXT: {
            Render_String(render, element.rect, element.text.c_str(), element.text.size());
            break;
        }

        case CBUTTON: {
            static uint8_t pSize = 2;  // pen size
            static Rect inside = Rect(pSize, pSize, -pSize * 2, -pSize * 2);
            auto ms = input::getMousePoint();
            Rect rect;

            // border
            guiInstance->GUI_SetMainColorRGB(0xa75100);
            SDL_RenderDrawRect(render, (SDL_Rect*)&element.rect);
            // fill
            guiInstance->GUI_SetMainColorRGB(hovering ? input::isMouseDown() ? 0xab0000 : 0xe70000 : 0xc90000);
            rect = element.rect;
            rect += inside / 2;
            SDL_RenderFillRect(render, (SDL_Rect*)&rect);

            // render text
            Render_String(render, element.rect, element.text.c_str(), element.text.size(), 13, TextAlign::MiddleCenter, true, hovering);
            bool msClick = input::isMouseUp();
            result = hovering && msClick;
            break;
        }

        case CEDIT: {
            break;
        }

        case CHSLIDER: {
            float* value = (float*)element.resources;
            // Minimal support

            break;
        }

        case CVSLIDER: {
            float* value = (float*)element.resources;
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
            bool msClick = hovering && input::isMouseUp();
            Rect r = element.rect;
            r += thick;

            Gizmos::setColor((hovering) ? Color::red : Color::darkred);
            SDL_RenderFillRect(render, (SDL_Rect*)&r);
            Gizmos::setColor((hovering) ? Color::darkred : Color::red);
            for (int x = 0; x < thickness; ++x) {
                r -= Rect(1, 1, -1, -1);
                SDL_RenderDrawRect(render, (SDL_Rect*)&r);
            }

            // draw text
            Texture* texture;
            SDL_Surface* surf = TTF_RenderText_Solid(font, element.text.c_str(), Color::white);
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
                Vec2Int vi;
                Rect elrect;
                r = element.rect;
                r.y += r.h;
                r.h = link->second.size() * sz;

                Gizmos::setColor(Color::darkred);
                // draw background
                SDL_RenderFillRect(render, (SDL_Rect*)&r);

                r.h = sz;  // set for height
                elrect = r;
                r.x += 5;

                int index = 0;
                for (auto iter = std::begin(link->second); iter != std::end(link->second); ++iter, ++index) {
                    // draw highligh
                    vi = input::getMousePoint();
                    if (SDL_PointInRect((SDL_Point*)&vi, (SDL_Rect*)&elrect)) {
                        Gizmos::setColor(Color::mediumvioletred);
                        SDL_RenderFillRect(render, (SDL_Rect*)&elrect);
                        if (input::isMouseUp()) {
                            link->first = index;
                            element.text = *iter;
                            focus = false;
                        }
                    }

                    Gizmos::setColor(Color::darkred);
                    surf = TTF_RenderUTF8_Solid(font, iter->c_str(), Color::white);
                    GC::gc_alloc_texture_from(&texture, surf);
                    r.h = texture->height();
                    r.w = texture->width();

                    r.y = elrect.y + elrect.h / 2 - r.h / 2;
                    SDL_RenderCopy(render, texture->native(), nullptr, (SDL_Rect*)&(r));

                    GC::gc_unalloc(texture);
                    SDL_FreeSurface(surf);
                    elrect.y += sz;
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

}  // namespace RoninEngine::UI
