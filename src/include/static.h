#pragma once

namespace RoninEngine
{
    namespace Runtime
    {
        constexpr std::uint32_t ronin_default_pixelformat = SDL_PIXELFORMAT_RGBA32;
        // default square pixel/100
        constexpr float pixelsPerPoint = 100;
        constexpr float spaceDiv = pixelsPerPoint / 2.f;
        constexpr float maxWorldScalar = 10000;

    } // namespace Runtime

} // namespace RoninEngine
