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

    TTF_Font *pDefaultTTFFont = nullptr;
    LegacyFont_t *pDefaultLegacyFont = nullptr;

    void update_legacy_font(RoninEngine::Runtime::World *world)
    {
        if(world->irs->legacy_font_normal)
            SDL_DestroyTexture(world->irs->legacy_font_normal);
        world->irs->legacy_font_normal = SDL_CreateTextureFromSurface(gscope.renderer, pDefaultLegacyFont->surfNormal);
        if(world->irs->legacy_font_hover)
            SDL_DestroyTexture(world->irs->legacy_font_hover);
        world->irs->legacy_font_hover = SDL_CreateTextureFromSurface(gscope.renderer, pDefaultLegacyFont->surfHilight);
    }

    void init_legacy_font(bool optimizeDeffects)
    {
        if(!pDefaultTTFFont)
        {
            auto raw = SDL_RWFromConstMem(raw_arial_font, raw_arial_length);
            pDefaultTTFFont = TTF_OpenFontRW(raw, SDL_TRUE, 14);
        }

        if(pDefaultLegacyFont)
            return;

        if(RoninMemory::alloc_self(pDefaultLegacyFont) == nullptr)
            RoninSimulator::BreakSimulate();

        if((pDefaultLegacyFont->surfNormal = private_load_surface(font_arealike_png, font_arealike_png_len)) == nullptr)
            throw std::bad_alloc();

        if((pDefaultLegacyFont->surfHilight = private_load_surface(font_arealike_hi_png, font_arealike_hi_png_len)) == nullptr)
            throw std::bad_alloc();

        pDefaultLegacyFont->fontSize = {font_arealike_width, font_arealike_height};

        // Структурирование шрифта
        {
            uint8_t target;
            uint8_t i;
            int deltay;
            Rect *p;
            int maxWidth = pDefaultLegacyFont->surfNormal->w;

            // set unknown symbols
            for(i = 0; i < 32; ++i)
            {
                p = pDefaultLegacyFont->data + i;
                // to unknown '?'
                p->x = 195;
                p->y = 17;
                p->w = pDefaultLegacyFont->fontSize.x;
                p->h = pDefaultLegacyFont->fontSize.y;
            }

            // load Eng & other chars
            maxWidth /= pDefaultLegacyFont->fontSize.x;
            target = i;
            deltay = 0;
            for(i = 0; target != 127;)
            {
                p = pDefaultLegacyFont->data + target++;
                p->x = pDefaultLegacyFont->fontSize.x * i;
                p->y = deltay;
                p->w = pDefaultLegacyFont->fontSize.x;
                p->h = pDefaultLegacyFont->fontSize.y;
                ++i;
                if(!(i = i % maxWidth) || target == 127)
                    deltay += pDefaultLegacyFont->fontSize.y;
            }

            // load Rus
            target = 192;
            for(i = 0; target != 255;)
            {
                p = pDefaultLegacyFont->data + target++;
                p->x = pDefaultLegacyFont->fontSize.x * i;
                p->y = deltay;
                p->w = pDefaultLegacyFont->fontSize.x;
                p->h = pDefaultLegacyFont->fontSize.y;
                ++i;
                if(!(i = i % maxWidth))
                    deltay += pDefaultLegacyFont->fontSize.y;
            }
        }
        if(optimizeDeffects)
        {
            SDL_Surface *model = pDefaultLegacyFont->surfNormal;
            int i, x, y, cx, cy;
            Rect optimizedRect;
            SDL_Color *pixel;
            std::uint8_t flagBreaker;
            for(i = 33; i != 255; ++i)
            {
                flagBreaker = 0;
                optimizedRect = pDefaultLegacyFont->data[i];
                // pitch это строка (ширина с объеденением в байтах) то есть x по оси
                // если умножить pitch * на h-высоту то можно получить последний пиксель
                for(x = 0; x < pDefaultLegacyFont->data[i].w / 2; ++x)
                {
                    // оптимизация с левой по правой
                    if(!(flagBreaker & 1))
                    {
                        for(y = optimizedRect.h - 1; y >= 0; --y)
                        {
                            cx = optimizedRect.x + x;
                            cy = optimizedRect.y + y;
                            // NOTE: Формула пикселей для SDL :: Y Offset * (Pitch/BytesPerPixel) + X Offset
                            pixel = (SDL_Color *) pDefaultLegacyFont->surfNormal->pixels +
                                (cy * (model->pitch / model->format->BytesPerPixel) + cx);
                            if(pixel->a) // isn't transparent
                                break;
                        }
                        if(y == ~0)
                            ++optimizedRect.x;
                        else
                            flagBreaker |= 1;
                    }
                    // оптимизация с права на лева
                    if(!(flagBreaker & 2))
                    {
                        for(y = optimizedRect.h - 1; y >= 0; --y)
                        {
                            // Формула Y Offset * (Pitch/BytesPerPixel) + X Offset
                            cx = optimizedRect.x + pDefaultLegacyFont->data[i].w - 1 - x;
                            cy = optimizedRect.y + y;
                            pixel = (SDL_Color *) pDefaultLegacyFont->surfNormal->pixels +
                                (cy * (model->pitch / model->format->BytesPerPixel) + cx);
                            if(pixel->a) // isn't transparent
                                break;
                        }
                        if(y == -1)
                            --optimizedRect.w;
                        else
                            flagBreaker |= 2;
                    }

                    if(flagBreaker == 3)
                        break;
                }
                pDefaultLegacyFont->data[i] = optimizedRect;
            }
        }
        pDefaultLegacyFont->compressed = static_cast<int>(optimizeDeffects);
    }

    void free_legacy_font()
    {
        RoninMemory::free(pDefaultLegacyFont);
        TTF_CloseFont(pDefaultTTFFont);
    }
} // namespace RoninEngine::UI
