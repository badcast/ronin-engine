#ifndef _FONT__H_
#define _FONT__H_

#include "ronin.h"

namespace RoninEngine::UI
{
    extern const unsigned char raw_arial_font[];
    extern const unsigned int raw_arial_length;

    constexpr int fontWidth = 13;
    constexpr int fontHeight = 18;

    void Initialize_Fonts(bool optimizeDeffects = true);
    int getwidth_metric(const std::string& text, int fontSize);
    Runtime::Rect Multiline_TextWidth_WithCyrilic(const std::string& text, int fontSize);

    void Render_String(SDL_Renderer* renerer, Runtime::Rect rect, const char* text, int len, int fontWidth = fontWidth, RoninEngine::UI::TextAlign textAlign = RoninEngine::UI::TextAlign::Left, bool textWrap = true, bool hilight = false);
} // namespace RoninEngine::UI
#endif
