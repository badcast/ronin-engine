#pragma once

#include "dependency.h"

namespace RoninEngine::Runtime {
struct Color {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;

    Color();

    Color(const Color&);

    Color(const std::int32_t rgba);

    Color(const std::uint32_t rgba);

    Color(const char* colorHex);

    Color(const std::string& colorHex);

    Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b);

    Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a);

    Color operator=(const Color& rhs);
    Color operator=(const int& rhs);
    Color operator=(const std::uint32_t& rhs);

    bool operator==(const Color& rhs);
    bool operator!=(const Color& rhs);
    operator int();
    operator std::uint32_t();
    operator SDL_Color();

    static const Color transparent;
    static const Color black;
    static const Color white;
    static const Color red;
    static const Color green;
    static const Color blue;
};
//NOTE: Optimized thats
//TODO: Optimize to static members (release an reinitialize)
inline const Color Color::transparent(0);
inline const Color Color::black(0xff000000);
inline const Color Color::white(0xffffffff);
inline const Color Color::red(0xff0000ff);
inline const Color Color::green(0xff00ff00);
inline const Color Color::blue(0xffff0000);


}  // namespace RoninEngine::Runtime
