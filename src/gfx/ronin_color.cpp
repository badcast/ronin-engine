
#include "ronin.h"

using namespace RoninEngine::Runtime;

Color::Color() : Color(0xFF000000)
{
}

Color::Color(const Color &other) : r(other.r), g(other.g), b(other.b), a(other.a)
{
}

Color::Color(Color &&other) : r(other.r), g(other.g), b(other.b), a(other.a)
{
}

Color::Color(const int rgba) : a((rgba >> 24) & 0xFF), r((rgba >> 16) & 0xFF), g((rgba >> 8) & 0xFF), b(rgba & 0xFF)
{
}

Color::Color(const int rgb, const std::uint8_t alpha) : r((rgb >> 16) & 0xFF), g((rgb >> 8) & 0xFF), b(rgb & 0xFF), a(alpha)
{
}

Color::Color(const char *hex) : Color(HexToColor(hex))
{
}

Color::Color(const std::string &hex) : Color(hex.c_str())
{
}

Color::Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b)
{
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = SDL_ALPHA_OPAQUE;
}

Color::Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a)
{
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

std::string Color::toHex() const
{
    return ColorToHex(*this);
}

int Color::toRGBA() const
{
    // standart method: static_cast<int>(r) | (g << 8) | (b << 16) | (a << 24);
    // fast method: ARGB as the pointer
    return *reinterpret_cast<const int *>(this);
}

int Color::toARGB() const
{
    // standart method: static_cast<int>(a) | (r << 8) | (g << 16) | (b << 24);
    // fast method: RGBA as the pointer
    return static_cast<int>(a) | ((*reinterpret_cast<const int *>(this)) << 8);
}

Color::Color(const native_color_t &color)
{
    (*this) = color;
}

Color Color::operator=(const Color &rhs)
{
    std::memcpy(this, &rhs, sizeof(Color));
    return *this;
}

Color Color::operator=(const int &rhs)
{
    r = ((rhs >> 16) & 0xFF);
    g = ((rhs >> 8) & 0xFF);
    b = (rhs & 0xFF);
    a = ((rhs >> 24) & 0xFF);
    return *this;
}

Color Color::operator=(const native_color_t &rhs)
{
    std::memcpy(this, &rhs, sizeof(rhs));
    return *this;
}

Color Color::operator+(const Color &rhs)
{
    return Color(
        Math::min(static_cast<int>(r) + rhs.r, 0xFF),
        Math::min(static_cast<int>(g) + rhs.g, 0xFF),
        Math::min(static_cast<int>(b) + rhs.b, 0xFF),
        Math::min(static_cast<int>(a) + rhs.a, 0xFF));
}

Color Color::operator-(const Color &rhs)
{
    return Color(
        Math::max(static_cast<int>(r) - rhs.r, 0),
        Math::max(static_cast<int>(g) - rhs.g, 0),
        Math::max(static_cast<int>(b) - rhs.b, 0),
        Math::max(static_cast<int>(a) - rhs.a, 0));
}

bool Color::operator==(const Color &rhs)
{
    return !std::memcmp(this, &rhs, sizeof(Color));
}

bool Color::operator!=(const Color &rhs)
{
    return !(this->operator==(rhs));
}

Color::operator int() const
{
    return toRGBA();
}

Color::operator native_color_t() const
{
    return *reinterpret_cast<const native_color_t *>(this);
}

Color Color::HexToColor(const char *hex)
{
    unsigned int red = 0, green = 0, blue = 0, alpha = 255;
    if(hex[0] == '#')
        hex++;

    sscanf(hex, "%02x%02x%02x%02x", &red, &green, &blue, &alpha);

    return Color(red, green, blue, alpha);
}

Color Color::HexToColor(const std::string &hex)
{
    return HexToColor(hex.c_str());
}

std::string Color::ColorToHex(const Color &color)
{
    char hex[9];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X", color.r, color.g, color.b, color.a);
    return hex;
}

Color Color::HSVToRGB(float h, float s, float v)
{
    h = Math::max(0.0f, Math::min(360.0f, h));
    s = Math::max(0.0f, Math::min(1.0f, s));
    v = Math::max(0.0f, Math::min(1.0f, v));

    float c = v * s;
    float x = c * (1 - std::abs(std::fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float r, g, b;
    if(h >= 0 && h < 60)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if(h >= 60 && h < 120)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if(h >= 120 && h < 180)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if(h >= 180 && h < 240)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if(h >= 240 && h < 300)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }

    return Color(
        static_cast<std::uint8_t>((r + m) * 255), static_cast<std::uint8_t>((g + m) * 255), static_cast<std::uint8_t>((b + m) * 255));
}

Color Color::RGBToHSV(std::uint8_t r, std::uint8_t g, std::uint8_t b)
{
    float rf = static_cast<float>(r) / 255.0f;
    float gf = static_cast<float>(g) / 255.0f;
    float bf = static_cast<float>(b) / 255.0f;

    float cmax = Math::max(Math::max(rf, gf), bf);
    float cmin = Math::min(Math::min(rf, gf), bf);
    float delta = cmax - cmin;

    float h = 0.0f, s = 0.0f, v = 0.0f;

    // Calculate hue
    if(delta != 0.0f)
    {
        if(cmax == rf)
        {
            h = 60.0f * std::fmod(((gf - bf) / delta), 6.0f);
        }
        else if(cmax == gf)
        {
            h = 60.0f * (((bf - rf) / delta) + 2.0f);
        }
        else if(cmax == bf)
        {
            h = 60.0f * (((rf - gf) / delta) + 4.0f);
        }
    }

    // Calculate saturation
    if(cmax != 0.0f)
    {
        s = delta / cmax;
    }

    // Calculate value
    v = cmax;

    return Color(h, s, v);
}

Color Color::FromRGBA(int rgba)
{
    return {rgba};
}

Color Color::FromARGB(int argb)
{
    return {/*r*/ (argb >> 8) & 0xFF, /*g*/ (argb >> 16) & 0xFF, /*b*/ (argb >> 24) & 0xFF, /*a*/ argb & 0xFF};
}
