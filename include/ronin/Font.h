#pragma once

#include "dependency.h"

namespace RoninEngine::UI {

constexpr int fontWidth = 13;
constexpr int fontHeight = 18;

enum TextAlign { Left, Center, Right, MiddleLeft, MiddleCenter, MiddleRight, BottomLeft, BottomCenter, BottomRight };

void Initialize_Fonts(bool optimizeDeffects = true);
int getwidth_metric(const std::string& text, int fontSize);
Runtime::Rect Multiline_TextWidth_WithCyrilic(const std::string& text, int fontSize);
void Render_String(SDL_Renderer* renerer, Runtime::Rect rect, const char* text, int len,
                   int fontWidth = RoninEngine::UI::fontWidth, TextAlign textAlign = TextAlign::Left, bool textWrap = true,
                   bool hilight = false);
}  // namespace RoninEngine::UI
