#include "ronin.h"

// get internal resources
#include "../res/fonts/ronin_font_arial.hpp"
#include "../res/fonts/ronin_font-arealike.png.hpp"
#include "../res/fonts/ronin_font-arealike-hi.png.hpp"

namespace RoninEngine::UI
{
    using namespace RoninEngine::Runtime;

    constexpr int font_arealike_width = 13;
    constexpr int font_arealike_height = 18;

    LegacyFont_t *pLegacyFont = nullptr;

    TTF_Font *ttf_arial_font = nullptr;

    void refresh_legacy_font(RoninEngine::Runtime::World *world)
    {
        if(world->irs->legacy_font_normal)
            SDL_DestroyTexture(world->irs->legacy_font_normal);
        world->irs->legacy_font_normal = SDL_CreateTextureFromSurface(env.renderer, pLegacyFont->surfNormal);
        if(world->irs->legacy_font_hover)
            SDL_DestroyTexture(world->irs->legacy_font_hover);
        world->irs->legacy_font_hover = SDL_CreateTextureFromSurface(env.renderer, pLegacyFont->surfHilight);
    }

    void init_legacy_font(bool optimizeDeffects)
    {
        if(!ttf_arial_font)
        {
            auto raw = SDL_RWFromConstMem(raw_arial_font, raw_arial_length);
            ttf_arial_font = TTF_OpenFontRW(raw, SDL_TRUE, 14);
        }

        if(pLegacyFont)
            return;

        if(RoninMemory::alloc_self(pLegacyFont) == nullptr)
            RoninSimulator::BreakSimulate();

        if((pLegacyFont->surfNormal = private_load_surface(font_arealike_png, font_arealike_png_len)) == nullptr)
            throw std::bad_alloc();

        if((pLegacyFont->surfHilight = private_load_surface(font_arealike_hi_png, font_arealike_hi_png_len)) == nullptr)
            throw std::bad_alloc();

        pLegacyFont->fontSize = {font_arealike_width, font_arealike_height};

        // Структурирование шрифта
        {
            uint8_t target;
            uint8_t i;
            int deltay;
            Rect *p;
            int maxWidth = pLegacyFont->surfNormal->w;

            // set unknown symbols
            for(i = 0; i < 32; ++i)
            {
                p = pLegacyFont->data + i;
                // to unknown '?'
                p->x = 195;
                p->y = 17;
                p->w = pLegacyFont->fontSize.x;
                p->h = pLegacyFont->fontSize.y;
            }

            // load Eng & other chars
            maxWidth /= pLegacyFont->fontSize.x;
            target = i;
            deltay = 0;
            for(i = 0; target != 127;)
            {
                p = pLegacyFont->data + target++;
                p->x = pLegacyFont->fontSize.x * i;
                p->y = deltay;
                p->w = pLegacyFont->fontSize.x;
                p->h = pLegacyFont->fontSize.y;
                ++i;
                if(!(i = i % maxWidth) || target == 127)
                    deltay += pLegacyFont->fontSize.y;
            }

            // load Rus
            target = 192;
            for(i = 0; target != 255;)
            {
                p = pLegacyFont->data + target++;
                p->x = pLegacyFont->fontSize.x * i;
                p->y = deltay;
                p->w = pLegacyFont->fontSize.x;
                p->h = pLegacyFont->fontSize.y;
                ++i;
                if(!(i = i % maxWidth))
                    deltay += pLegacyFont->fontSize.y;
            }
        }
        if(optimizeDeffects)
        {
            SDL_Surface *model = pLegacyFont->surfNormal;
            int i, x, y, cx, cy;
            Rect rect_point;
            SDL_Color *pixel;
            for(i = 33; i != 255; ++i)
            {
                // TODO: оптимизация прямоугольника
                rect_point = pLegacyFont->data[i];
                uint8_t flagBreaker = 0;
                // pitch это строка (ширина с объеденением в байтах) то есть x по оси
                // если умножить pitch * на h-высоту то можно получить последний пиксель
                for(x = 0; x < pLegacyFont->data[i].w / 2; ++x)
                {
                    // оптимизация с левой по правой
                    if(!(flagBreaker & 1))
                    {
                        for(y = rect_point.h - 1; y >= 0; --y)
                        {
                            cx = rect_point.x + x;
                            cy = rect_point.y + y;
                            // NOTE: Формула пикселей для SDL :: Y Offset * (Pitch/BytesPerPixel) + X Offset
                            pixel =
                                (SDL_Color *) pLegacyFont->surfNormal->pixels + (cy * (model->pitch / model->format->BytesPerPixel) + cx);
                            if(pixel->a) // isn't transparent
                                break;
                        }
                        if(y == ~0)
                            ++rect_point.x;
                        else
                            flagBreaker |= 1;
                    }
                    // оптимизация с права на лева
                    if(!(flagBreaker & 2))
                    {
                        for(y = rect_point.h - 1; y >= 0; --y)
                        {
                            // Формула Y Offset * (Pitch/BytesPerPixel) + X Offset
                            cx = rect_point.x + pLegacyFont->data[i].w - 1 - x;
                            cy = rect_point.y + y;
                            pixel =
                                (SDL_Color *) pLegacyFont->surfNormal->pixels + (cy * (model->pitch / model->format->BytesPerPixel) + cx);
                            if(pixel->a) // isn't transparent
                                break;
                        }
                        if(y == -1)
                            --rect_point.w;
                        else
                            flagBreaker |= 2;
                    }

                    if(flagBreaker == 3)
                        break;
                }
                pLegacyFont->data[i] = rect_point;
            }
        }
    }

    void free_fonts()
    {
        RoninMemory::free(pLegacyFont);
        TTF_CloseFont(ttf_arial_font);
    }
} // namespace RoninEngine::UI
