#ifndef RONIN_FONTBASIC_H
#define RONIN_FONTBASIC_H 1

#include "basic_ronin.h"

namespace RoninEngine::Runtime
{
    struct font2d_t;

    // create font2d
    font2d_t *font2d_create(SDL_Surface *surfaceNormal, SDL_Surface *surfaceHiNormal, bool optimizeDeffects = true);
    // first init legacy 2d font
    void font2d_init(bool optimizeDeffects = true);
    // update strongly for set world
    void font2d_update(World *world);
    // get metrics (width,height) for string lenght with data
    int font2d_metric(const std::string &text, int fontSize);
    // get internal font2d
    font2d_t *font2d_get_basic();
    // draw font to at point with specified args
    void font2d_string(Rect rect, const char *text, int len, int fontWidth = -1, RoninEngine::UI::UIAlign textAlign = RoninEngine::UI::UIAlign::Left, bool textWrap = true, bool hilight = false);
} // namespace RoninEngine::Runtime

#endif
