
#include "ronin.h"

using namespace RoninEngine::Runtime;

Color::Color()
    : Color(0)
{
}

Color::Color(const Color& other)
    : r(other.r)
    , g(other.g)
    , b(other.b)
    , a(other.a)
{
}

Color::Color(Color&& other)
    : r(other.r)
    , g(other.g)
    , b(other.b)
    , a(other.a)
{
}

Color::Color(int rgb)
{
    memcpy(this, &rgb, sizeof(Color));
    a = SDL_ALPHA_OPAQUE;
}

Color::Color(std::uint32_t rgb)
{
    memcpy(this, &rgb, sizeof(std::uint32_t));
    a = SDL_ALPHA_OPAQUE;
}

Color::Color(const char* colorHex)
{
}

Color::Color(const std::string& colorHex)
    : Color(colorHex.c_str())
{
}

Color::Color(const native_color_t& color)
{
    (*this) = color;
}

Color::Color(native_color_t&& color)
{
    (*this) = color;
}

Color::Color(const uint8_t r, const uint8_t g, const uint8_t b)
{
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = 255;
}

Color::Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

Color Color::operator=(const Color& rhs)
{
    (*reinterpret_cast<int*>(this)) = *reinterpret_cast<int*>(&const_cast<Color&>(rhs));
    return *this;
}

Color Color::operator=(const int& rhs)
{
    (*reinterpret_cast<int*>(this)) = *(&const_cast<int&>(rhs));
    return *this;
}

Color Color::operator=(const SDL_Color& rhs)
{
    memcpy(this, &rhs, sizeof(rhs));
    return *this;
}

Color Color::operator=(const std::uint32_t& rhs)
{
    (*reinterpret_cast<std::uint32_t*>(this)) = *(&const_cast<std::uint32_t&>(rhs));
    return *this;
}

Color::operator int() const
{
    return *reinterpret_cast<const int*>(this);
}

Color::operator std::uint32_t() const
{
    return *reinterpret_cast<const std::uint32_t*>(this);
}

Color::operator native_color_t() const
{
    native_color_t col;
    memcpy(&col, this, sizeof(col));
    return col;
}
