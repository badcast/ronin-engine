#ifndef RONIN_FONT_INPUT_H
#define RONIN_FONT_INPUT_H 1

#include "basic_ronin.h"
#include "font.h"

namespace RoninEngine::Runtime
{
    struct font_ext_t
    {
        TTF_Font *ttf;
        font2d_t legacy;
    };
} // namespace RoninEngine::Runtime

#endif
