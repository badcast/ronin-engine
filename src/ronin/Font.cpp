#include "framework.h"

namespace RoninEngine::UI {

using RoninEngine::Runtime::GC;

struct Font_t {
    SDL_Surface *surfNormal;
    SDL_Surface *surfHilight;
    Vec2Int fontSize;
    Rect data[255];
};

Font_t *pfont = nullptr;
uint8_t VH[]{0, 32, 16, 2, 34, 18, 1, 33, 17};

SDL_Texture *pfontTexture = nullptr;
SDL_Texture *pfontTextureHilight = nullptr;

void Initialize_Fonts(bool optimizeDeffects) {
    if (pfontTexture) {
        throw std::runtime_error("Re initialization fail");
    }

    if (GC::gc_alloc_lval(pfont) == nullptr) Application::fail_OutOfMemory();

    if ((pfont->surfNormal = GC::GetSurface("font-arealike")) == nullptr) throw std::bad_alloc();

    if ((pfont->surfHilight = GC::GetSurface("font-arealike-hi")) == nullptr) throw std::bad_alloc();

    pfont->fontSize = {fontWidth, fontHeight};

    //Структурирование шрифта
    {
        uint8_t target;
        uint8_t i;
        int deltay;
        Rect *p;
        int maxWidth = pfont->surfNormal->w;

        // set unknown symbols
        for (i = 0; i < 32; ++i) {
            p = pfont->data + i;
            // to unknown '?'
            p->x = 195;
            p->y = 17;
            p->w = pfont->fontSize.x;
            p->h = pfont->fontSize.y;
        }

        // load Eng & other chars
        maxWidth /= pfont->fontSize.x;
        target = i;
        deltay = 0;
        for (i = 0; target != 127;) {
            p = pfont->data + target++;
            p->x = pfont->fontSize.x * i;
            p->y = deltay;
            p->w = pfont->fontSize.x;
            p->h = pfont->fontSize.y;
            ++i;
            if (!(i = i % maxWidth) || target == 127) deltay += pfont->fontSize.y;
        }

        // load Rus
        target = 192;
        for (i = 0; target != 255;) {
            p = pfont->data + target++;
            p->x = pfont->fontSize.x * i;
            p->y = deltay;
            p->w = pfont->fontSize.x;
            p->h = pfont->fontSize.y;
            ++i;
            if (!(i = i % maxWidth)) deltay += pfont->fontSize.y;
        }
    }

    // convert surface to texture
    GC::gc_alloc_sdl_texture(&pfontTexture, pfont->surfNormal);
    GC::gc_alloc_sdl_texture(&pfontTextureHilight, pfont->surfHilight);

    if (pfontTexture == nullptr || pfontTextureHilight == nullptr) Application::fail("error initialization fonts");

    if (optimizeDeffects && false) {
        SDL_Surface *model = pfont->surfNormal;
        int i, x, y, cx, cy;
        Rect rect_point;
        SDL_Color *pixel;
        for (i = 33; i != 255; ++i) {
            // TODO: оптимизация прямоугольника
            rect_point = pfont->data[i];
            uint8_t flagBreaker = 0;
            // pitch это строка (ширина с объеденением в байтах) то есть x по оси
            //если умножить pitch * на h-высоту то можно получить последний пиксель
            for (x = 0; x < pfont->data[i].w / 2; ++x) {
                //оптимизация с левой до правой
                if (!(flagBreaker & 1)) {
                    for (y = rect_point.h - 1; y >= 0; --y) {
                        cx = rect_point.x + x;
                        cy = rect_point.y + y;
                        //Формула Y Offset * (Pitch/BytesPerPixel) + X Offset
                        pixel =
                            (SDL_Color *)pfont->surfNormal->pixels + (cy * (model->pitch / model->format->BytesPerPixel) + cx);
                        if (pixel->a)  // isn't transparent
                            break;
                    }
                    if (y == ~0)
                        ++rect_point.x;
                    else
                        flagBreaker |= 1;
                }
                //оптимизация с права до лева
                if (!(flagBreaker & 2)) {
                    for (y = rect_point.h - 1; y >= 0; --y) {
                        //Формула Y Offset * (Pitch/BytesPerPixel) + X Offset
                        cx = rect_point.x + pfont->data[i].w - 1 - x;
                        cy = rect_point.y + y;
                        pixel =
                            (SDL_Color *)pfont->surfNormal->pixels + (cy * (model->pitch / model->format->BytesPerPixel) + cx);
                        if (pixel->a)  // isn't transparent
                            break;
                    }
                    if (y == -1)
                        --rect_point.w;
                    else
                        flagBreaker |= 2;
                }

                if (flagBreaker == 3) break;
            }
            pfont->data[i] = rect_point;
        }
    }
}

int Single_TextWidth_WithCyrilic(const std::string &text, int fontSize) {
    int width = 0;
    for (auto iter = begin(text); iter != end(text); ++iter)
        // TODO: Учитывать размер шрифта (fontSize)
        width += pfont->data[(unsigned char)*iter].w;
    return width;
}

Rect Multiline_TextWidth_WithCyrilic(const std::string &text, int fontSize) {
    // todo: вычисление мультистрок
    Rect rect;
    throw std::exception();
    return rect;
}

void Render_String(SDL_Renderer *renderer, Rect rect, const char *text, int len, int pfontWidth, TextAlign textAlign,
                   bool textWrap, bool hilight) {
    if (len <= 0 || !pfontTexture || hilight && !pfontTextureHilight) return;

    std::uint8_t temp;
    Rect *src;
    std::uint16_t pos;
    SDL_Rect dst = *(SDL_Rect *)&rect;
    Vec2Int fontSize = pfont->fontSize + Vec2Int(1, 1) * (pfontWidth - fontWidth);
    SDL_Texture *targetpfont = nullptr;
    int textWidth = Single_TextWidth_WithCyrilic(text, pfontWidth);

    if (!rect.w) rect.w = textWidth;
    if (!rect.h) rect.h = pfont->fontSize.y;  // todo: для мультий строк требуется вычислить h высоту

    targetpfont = hilight ? pfontTextureHilight : pfontTexture;

    // x
    temp = (VH[textAlign] >> 4 & 15);
    if (temp) {
        dst.x += rect.w / temp;
        if (temp == 2)
            dst.x += -textWidth / 2;
        else if (temp == 1)
            dst.x += -textWidth;
    }
    // y
    temp = (VH[textAlign] & 15);
    if (temp) {
        dst.y += rect.h / temp - fontSize.y / 2;
        if (temp == 1) dst.y += -textWidth / 2;
    }

    char *cc = "Hello";

    Vec2Int begin = *(Vec2Int *)&dst;
    int deltax;
    for (pos = 0; pos < len; ++pos) {
        memcpy(&temp, text + pos, 1);
        src = (pfont->data + temp);
        if (temp != '\n') {
            dst.w = src->w;
            dst.h = src->h;
            deltax = rect.x + rect.w;

            if (dst.x >= deltax) {
                for (++pos; pos < len;) {
                    temp = *(text + pos);
                    if (temp != '\n')
                        ++pos;
                    else
                        break;
                }
                continue;
            }

            //отрисовываем остаток входящую в область
            dst.w = Math::max(0, Math::min(deltax - dst.x, dst.w));
            // if (dst.x <= src.x + src.w && dst.y <= src.y + src.h)
            SDL_RenderCopy(renderer, targetpfont, (SDL_Rect *)src, &dst);
            dst.x += src->w;
        } else {
            dst.x = begin.x;
            dst.y += src->h;
        }
    }
}
}  // namespace RoninEngine::UI
