#pragma once

#include "ronin.h"

namespace RoninEngine::UI
{
    extern const unsigned char raw_arial_font[];
    extern const unsigned int raw_arial_length;

    extern const unsigned char font_arealike_png[];
    extern const unsigned int font_arealike_png_len;

    extern const unsigned char font_arealike_hi_png[];
    extern const unsigned int font_arealike_hi_png_len;

    constexpr int fontWidth = 13;
    constexpr int fontHeight = 18;

    void init_fonts(bool optimizeDeffects = true);
    int getwidth_metric(const std::string& text, int fontSize);

    void render_string_legacy(SDL_Renderer* renerer, Runtime::Rect rect, const char* text, int len, int fontWidth = fontWidth, RoninEngine::UI::TextAlign textAlign = RoninEngine::UI::TextAlign::Left, bool textWrap = true, bool hilight = false);
} // namespace RoninEngine::UI
