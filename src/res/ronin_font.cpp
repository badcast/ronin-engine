#include "ronin.h"

// get internal resources
#include "../res/fonts/ronin_font_arial.hpp"
#include "../res/fonts/ronin_font-arealike.png.hpp"
#include "../res/fonts/ronin_font-arealike-hi.png.hpp"

namespace RoninEngine::UI
{
    extern int Single_TextWidth_WithCyrilic(const std::string &text, int fontSize);
}

namespace RoninEngine::Runtime
{

    constexpr std::uint8_t legacyVH[] {0, 32, 16, 2, 34, 18, 1, 33, 17};

    constexpr int font_arealike_width = 13;
    constexpr int font_arealike_height = 18;

    TTF_Font *pDefaultTTFFont = nullptr;
    font2d_t *f2d_default = nullptr;

    void font2d_update(RoninEngine::Runtime::World *world)
    {
        if(world->irs->legacy_font_normal)
            SDL_DestroyTexture(world->irs->legacy_font_normal);
        world->irs->legacy_font_normal = SDL_CreateTextureFromSurface(gscope.renderer, f2d_default->surfNormal);
        if(world->irs->legacy_font_hover)
            SDL_DestroyTexture(world->irs->legacy_font_hover);
        world->irs->legacy_font_hover = SDL_CreateTextureFromSurface(gscope.renderer, f2d_default->surfHilight);

        _custom_refresh_fonts();
    }

    void font2d_init(bool optimizeDeffects)
    {
        if(!pDefaultTTFFont)
        {
            auto raw = SDL_RWFromConstMem(raw_arial_font, raw_arial_length);
            pDefaultTTFFont = TTF_OpenFontRW(raw, SDL_TRUE, 14);
        }

        if(f2d_default)
            return;

        if(RoninMemory::alloc_self(f2d_default) == nullptr)
        {
            throw RoninEngine::Exception::ronin_out_of_mem();
        }

        if((f2d_default->surfNormal = private_load_surface(font_arealike_png, font_arealike_png_len)) == nullptr)
            throw RoninEngine::Exception::ronin_out_of_mem();

        if((f2d_default->surfHilight = private_load_surface(font_arealike_hi_png, font_arealike_hi_png_len)) == nullptr)
            throw RoninEngine::Exception::ronin_out_of_mem();

        f2d_default->fontSize = {font_arealike_width, font_arealike_height};

        // Структурирование шрифта
        {
            uint8_t target;
            uint8_t i;
            int deltay;
            Rect *p;
            int maxWidth = f2d_default->surfNormal->w;

            // set unknown symbols
            for(i = 0; i < 32; ++i)
            {
                p = f2d_default->data + i;
                // to unknown '?'
                p->x = 195;
                p->y = 17;
                p->w = f2d_default->fontSize.x;
                p->h = f2d_default->fontSize.y;
            }

            // load Eng & other chars
            maxWidth /= f2d_default->fontSize.x;
            target = i;
            deltay = 0;
            for(i = 0; target != 127;)
            {
                p = f2d_default->data + target++;
                p->x = f2d_default->fontSize.x * i;
                p->y = deltay;
                p->w = f2d_default->fontSize.x;
                p->h = f2d_default->fontSize.y;
                ++i;
                if(!(i = i % maxWidth) || target == 127)
                    deltay += f2d_default->fontSize.y;
            }

            // load Rus
            target = 192;
            for(i = 0; target != 255;)
            {
                p = f2d_default->data + target++;
                p->x = f2d_default->fontSize.x * i;
                p->y = deltay;
                p->w = f2d_default->fontSize.x;
                p->h = f2d_default->fontSize.y;
                ++i;
                if(!(i = i % maxWidth))
                    deltay += f2d_default->fontSize.y;
            }
        }
        if(optimizeDeffects)
        {
            SDL_Surface *model = f2d_default->surfNormal;
            int i, x, y, cx, cy;
            Rect optimizedRect;
            SDL_Color *pixel;
            std::uint8_t flagBreaker;
            for(i = 33; i != 255; ++i)
            {
                flagBreaker = 0;
                optimizedRect = f2d_default->data[i];
                // pitch это строка (ширина с объеденением в байтах) то есть x по оси
                // если умножить pitch * на h-высоту то можно получить последний пиксель
                for(x = 0; x < f2d_default->data[i].w / 2; ++x)
                {
                    // оптимизация с левой по правой
                    if(!(flagBreaker & 1))
                    {
                        for(y = optimizedRect.h - 1; y >= 0; --y)
                        {
                            cx = optimizedRect.x + x;
                            cy = optimizedRect.y + y;
                            // NOTE: Формула пикселей для SDL :: Y Offset * (Pitch/BytesPerPixel) + X Offset
                            pixel = (SDL_Color *) f2d_default->surfNormal->pixels + (cy * (model->pitch / model->format->BytesPerPixel) + cx);
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
                            cx = optimizedRect.x + f2d_default->data[i].w - 1 - x;
                            cy = optimizedRect.y + y;
                            pixel = (SDL_Color *) f2d_default->surfNormal->pixels + (cy * (model->pitch / model->format->BytesPerPixel) + cx);
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
                f2d_default->data[i] = optimizedRect;
            }
        }
        f2d_default->compressed = static_cast<int>(optimizeDeffects);
    }

    void font2d_string(Rect rect, const char *text, int len, int fontWidth, RoninEngine::UI::UIAlign textAlign, bool textWrap, bool hilight)
    {
        if(text == nullptr || len <= 0)
            return;

        std::uint8_t temp;
        Rect *src;
        std::uint16_t pos;
        SDL_Rect dst = *reinterpret_cast<SDL_Rect *>(&rect);

        Vec2Int fontSize = f2d_default->fontSize + Vec2Int::one * (fontWidth - f2d_default->fontSize.x);
        int textWidth = RoninEngine::UI::Single_TextWidth_WithCyrilic(text, fontWidth);

        if(!rect.w)
            rect.w = textWidth;
        if(!rect.h)
            rect.h = f2d_default->fontSize.y;

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
            src = (f2d_default->data + temp);
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
                SDL_RenderCopy(gscope.renderer, (hilight ? currentWorld->irs->legacy_font_hover : currentWorld->irs->legacy_font_normal), reinterpret_cast<SDL_Rect *>(src), &dst);
                dst.x += src->w;
            }
            else
            {
                dst.x = begin.x;
                dst.y += src->h;
            }
        }
    }

    font2d_t *font2d_get_basic()
    {
        return f2d_default;
    }

    void free_legacy_font()
    {
        RoninMemory::free(f2d_default);
        TTF_CloseFont(pDefaultTTFFont);
    }
} // namespace RoninEngine::Runtime
