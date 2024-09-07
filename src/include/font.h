#pragma once

#include "ronin.h"

namespace RoninEngine::UI
{
    // first init legacy 2d font
    void font2d_init(bool optimizeDeffects = true);
    // update strongly for set world
    void font2d_update(RoninEngine::Runtime::World *world);
    // get metrics (width,height) for string lenght with data
    int font2d_metric(const std::string &text, int fontSize);
    // draw font to at point with specified args
    void font2d_string(Runtime::Rect rect, const char *text, int len, int fontWidth = -1, RoninEngine::UI::Align textAlign = RoninEngine::UI::Align::Left, bool textWrap = true, bool hilight = false);
} // namespace RoninEngine::UI
