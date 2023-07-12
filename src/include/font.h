#pragma once

#include "ronin.h"

namespace RoninEngine::UI
{
    void init_fonts(bool optimizeDeffects = true);
    int getwidth_metric(const std::string &text, int fontSize);

    void render_string_legacy(
        Runtime::Rect rect,
        const char *text,
        int len,
        int fontWidth = -1,
        RoninEngine::UI::TextAlign textAlign = RoninEngine::UI::TextAlign::Left,
        bool textWrap = true,
        bool hilight = false);
} // namespace RoninEngine::UI
