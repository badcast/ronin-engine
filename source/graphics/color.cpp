#include "Color.h"

#include "ronin.h"

Color::Color() { memset(this, 0, sizeof(Color)); }

Color::Color(const Color &assign) : r(assign.r), g(assign.g), b(assign.b), a(assign.a) {}

Color::Color(const int32_t rgba) { memcpy(this, &rgba, sizeof(Color)); }

Color::Color(const uint32_t rgba) { memcpy(this, &rgba, sizeof(Color)); }

Color::Color(const char *colorHex) {}

Color::Color(const std::string &colorHex) : Color(colorHex.c_str()) {}

Color::Color(const uint8_t r, const uint8_t g, const uint8_t b) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = 255;
}

Color::Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

Color Color::operator=(const Color &rhs) {
    (*reinterpret_cast<int *>(this)) = *reinterpret_cast<int *>(&const_cast<Color &>(rhs));
    return *this;
}

Color Color::operator=(const int &rhs) {
    (*reinterpret_cast<int *>(this)) = *(&const_cast<int &>(rhs));
    return *this;
}

Color Color::operator=(const std::uint32_t &rhs) {
    (*reinterpret_cast<std::uint32_t *>(this)) = *(&const_cast<std::uint32_t &>(rhs));
    return *this;
}

Color::operator int() const { return *(const int *)(this); }

Color::operator std::uint32_t() const { return *(std::uint32_t *)(this); }
