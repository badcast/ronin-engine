#pragma once

#include "ronin.h"

namespace RoninEngine::UI
{
    void init_legacy_font(bool optimizeDeffects = true);
    void update_legacy_font(RoninEngine::Runtime::World *world);
    int getwidth_metric(const std::string &text, int fontSize);

    void Render_String_Legacy(
        Runtime::Rect rect,
        const char *text,
        int len,
        int fontWidth = -1,
        RoninEngine::UI::Align textAlign = RoninEngine::UI::Align::Left,
        bool textWrap = true,
        bool hilight = false);
} // namespace RoninEngine::UI
