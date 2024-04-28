#pragma once

namespace RoninEngine
{
    namespace Runtime
    {
        constexpr std::uint32_t defaultPixelFormat = SDL_PIXELFORMAT_RGBA32;
        constexpr float defaultPixelsPerPoint = 100;
        constexpr float defaultSpaceDiv = defaultPixelsPerPoint / 2.f;
        constexpr float defaultMaxWorldScalar = 10000;

    } // namespace Runtime

} // namespace RoninEngine
