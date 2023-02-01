#pragma once

#include "ronin.h"

namespace RoninEngine::UI
{

    extern unsigned char raw_arial_font[];
    extern unsigned int raw_arial_length;

    constexpr int fontWidth = 13;
    constexpr int fontHeight = 18;

    void Initialize_Fonts(bool optimizeDeffects = true);
    int getwidth_metric(const std::string& text, int fontSize);
    Runtime::Rect Multiline_TextWidth_WithCyrilic(const std::string& text, int fontSize);
    void Render_String(SDL_Renderer* renerer, Runtime::Rect rect, const char* text, int len, int fontWidth = fontWidth, TextAlign textAlign = TextAlign::Left, bool textWrap = true, bool hilight = false);
} // namespace RoninEngine::UI
