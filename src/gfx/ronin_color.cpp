
#include "ronin.h"
#include "ronin_exception.h"

#include "Color.h"

#define COLOR_IMPL_EXT API_EXPORT const Color

constexpr const char* HexColorFormatRGB = "%02X%02X%02X%02X";
constexpr const char* HexColorFormatRGBA = "%02X%02X%02X%02X%02X";

using namespace RoninEngine::Runtime;
using namespace RoninEngine::Exception;

Color::Color() : Color(0xFF000000)
{
}

Color::Color(const Color &other) : r(other.r), g(other.g), b(other.b), a(other.a)
{
}

Color::Color(Color &&other) : r(other.r), g(other.g), b(other.b), a(other.a)
{
}

Color::Color(const Color &from, value_type a) : r(from.r), g(from.g), b(from.b), a(a)
{
}

Color::Color(int rgba) : a((rgba >> 24) & 0xFF), r((rgba >> 16) & 0xFF), g((rgba >> 8) & 0xFF), b(rgba & 0xFF)
{
}

Color::Color(int rgb, value_type alpha) : r((rgb >> 16) & 0xFF), g((rgb >> 8) & 0xFF), b(rgb & 0xFF), a(alpha)
{
}

Color::Color(const char *hex) : Color(HexToColor(hex))
{
}

Color::Color(const std::string &hex) : Color(hex.c_str())
{
}

Color::Color(value_type r, value_type g, value_type b)
{
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = SDL_ALPHA_OPAQUE;
}

Color::Color(value_type r, value_type g, value_type b, value_type a)
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

std::tuple<float, float, float> Color::toHSV() const
{
    std::tuple<float, float, float> conv = Color::RGBToHSV(r, g, b);
    return conv;
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
    return Color(Math::Min(static_cast<int>(r) + rhs.r, 0xFF), Math::Min(static_cast<int>(g) + rhs.g, 0xFF), Math::Min(static_cast<int>(b) + rhs.b, 0xFF), Math::Min(static_cast<int>(a) + rhs.a, 0xFF));
}

Color Color::operator-(const Color &rhs)
{
    return Color(Math::Max(static_cast<int>(r) - rhs.r, 0), Math::Max(static_cast<int>(g) - rhs.g, 0), Math::Max(static_cast<int>(b) - rhs.b, 0), Math::Max(static_cast<int>(a) - rhs.a, 0));
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
    std::uint32_t red = 0, green = 0, blue = 0, alpha = 255;

    if(hex == nullptr)
    {
        throw ronin_null_error();
    }

    if(hex[0] == '#')
        hex++;

    int len = strlen(hex);
    char *chars = static_cast<char *>(alloca(len));

    for(int i = 0; hex[i]; ++i)
        chars[i] = std::toupper(hex[i]);

    sscanf(chars, HexColorFormatRGBA, &red, &green, &blue, &alpha);

    return Color(red, green, blue, alpha);
}

Color Color::HexToColor(const std::string &hex)
{
    return HexToColor(hex.c_str());
}

Color Color::MakeTransparency(const Color &color, float alpha)
{
    return {color, static_cast<value_type>(Math::Clamp01(alpha) * 0xFF)};
}

std::string Color::ColorToHex(const Color &color, bool appendAlpha)
{
    char hex[18];

    if(appendAlpha)
        snprintf(hex, sizeof(hex), HexColorFormatRGBA, color.r, color.g, color.b, color.a);
    else
        snprintf(hex, sizeof(hex), HexColorFormatRGB, color.r, color.g, color.b);

    return {hex};
}

Color Color::HSVToRGB(float h, float s, float v)
{
    h = Math::Max(0.0f, Math::Min(360.0f, h));
    s = Math::Max(0.0f, Math::Min(1.0f, s));
    v = Math::Max(0.0f, Math::Min(1.0f, v));

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

    return Color(static_cast<value_type>((r + m) * 255), static_cast<value_type>((g + m) * 255), static_cast<value_type>((b + m) * 255));
}

std::tuple<float, float, float> Color::RGBToHSV(value_type r, value_type g, value_type b)
{
    float rf = static_cast<float>(r) / 255.0f;
    float gf = static_cast<float>(g) / 255.0f;
    float bf = static_cast<float>(b) / 255.0f;

    float cmax = Math::Max(Math::Max(rf, gf), bf);
    float cmin = Math::Min(Math::Min(rf, gf), bf);
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

    return {h, s, v};
}

Color Color::HSVToRGBHalf(std::int16_t h, std::int16_t s, std::int16_t v)
{
    return HSVToRGB(Math::HalfToFloat(h), Math::HalfToFloat(s), Math::HalfToFloat(v));
}

std::tuple<std::int16_t, std::int16_t, std::int16_t> Color::RGBToHSVHalf(value_type r, value_type g, value_type b)
{
    std::tuple<float, float, float> hsv = RGBToHSV(r, g, b);
    return {Math::FloatToHalf(std::get<0>(hsv)), Math::FloatToHalf(std::get<1>(hsv)), Math::FloatToHalf(std::get<2>(hsv))};
}

Color Color::FromRGBA(int rgba)
{
    return {rgba};
}

Color Color::FromARGB(int argb)
{
    return {
        /*r*/ static_cast<value_type>((argb >> 8) & 0xFF),
        /*g*/ static_cast<value_type>((argb >> 16) & 0xFF),
        /*b*/ static_cast<value_type>((argb >> 24) & 0xFF),
        /*a*/ static_cast<value_type>(argb & 0xFF)};
}

Color Color::FromHSV(float h, float s, float v)
{
    return Color::HSVToRGB(h, s, v);
}

Color Color::Lerp(Color start, Color end, float t)
{
    t = Math::Clamp01(t);
    start.r = (start.r + (end.r - start.r) * t);
    start.g = (start.g + (end.g - start.g) * t);
    start.b = (start.b + (end.b - start.b) * t);
    start.a = (start.a + (end.a - start.a) * t);
    return start;
}

std::vector<Color> Color::GetBasicColors()
{
    return {Color::red, Color::green, Color::blue, Color::silver, Color::gray, Color::maroon, Color::purple, Color::fuchsia, Color::lime, Color::olive, Color::yellow, Color::navy, Color::teal, Color::aqua, Color::black, Color::white};
}

std::vector<Color> Color::GetExtendedColors()
{
    return {
        Color::aliceblue,
        Color::antiquewhite,
        Color::aquamarine,
        Color::azure,
        Color::beige,
        Color::bisque,
        Color::blanchedalmond,
        Color::blueviolet,
        Color::brown,
        Color::burlywood,
        Color::cadetblue,
        Color::chartreuse,
        Color::chocolate,
        Color::coral,
        Color::cornflowerblue,
        Color::cornsilk,
        Color::crimson,
        Color::cyan,
        Color::darkblue,
        Color::darkcyan,
        Color::darkgoldenrod,
        Color::darkgray,
        Color::darkgreen,
        Color::darkgrey,
        Color::darkkhaki,
        Color::darkmagenta,
        Color::darkolivegreen,
        Color::darkorange,
        Color::darkorchid,
        Color::darkred,
        Color::darksalmon,
        Color::darkseagreen,
        Color::darkslateblue,
        Color::darkslategray,
        Color::darkslategrey,
        Color::darkturquoise,
        Color::darkviolet,
        Color::deeppink,
        Color::deepskyblue,
        Color::dimgray,
        Color::dimgrey,
        Color::dodgerblue,
        Color::firebrick,
        Color::floralwhite,
        Color::forestgreen,
        Color::gainsboro,
        Color::ghostwhite,
        Color::gold,
        Color::goldenrod,
        Color::greenyellow,
        Color::grey,
        Color::honeydew,
        Color::hotpink,
        Color::indianred,
        Color::indigo,
        Color::ivory,
        Color::khaki,
        Color::lavender,
        Color::lavenderblush,
        Color::lawngreen,
        Color::lemonchiffon,
        Color::lightblue,
        Color::lightcoral,
        Color::lightcyan,
        Color::lightgoldenrodyellow,
        Color::lightgray,
        Color::lightgreen,
        Color::lightgrey,
        Color::lightpink,
        Color::lightsalmon,
        Color::lightseagreen,
        Color::lightskyblue,
        Color::lightslategray,
        Color::lightslategrey,
        Color::lightsteelblue,
        Color::lightyellow,
        Color::limegreen,
        Color::linen,
        Color::magenta,
        Color::mediumaquamarine,
        Color::mediumblue,
        Color::mediumorchid,
        Color::mediumpurple,
        Color::mediumseagreen,
        Color::mediumslateblue,
        Color::mediumspringgreen,
        Color::mediumturquoise,
        Color::mediumvioletred,
        Color::midnightblue,
        Color::mintcream,
        Color::mistyrose,
        Color::moccasin,
        Color::navajowhite,
        Color::oldlace,
        Color::olivedrab,
        Color::orange,
        Color::orangered,
        Color::orchid,
        Color::palegoldenrod,
        Color::palegreen,
        Color::paleturquoise,
        Color::palevioletred,
        Color::papayawhip,
        Color::peachpuff,
        Color::peru,
        Color::pink,
        Color::plum,
        Color::powderblue,
        Color::rosybrown,
        Color::royalblue,
        Color::saddlebrown,
        Color::salmon,
        Color::sandybrown,
        Color::seagreen,
        Color::seashell,
        Color::sienna,
        Color::skyblue,
        Color::slateblue,
        Color::slategray,
        Color::slategrey,
        Color::snow,
        Color::springgreen,
        Color::steelblue,
        Color::tan,
        Color::thistle,
        Color::tomato,
        Color::turquoise,
        Color::violet,
        Color::wheat,
        Color::whitesmoke,
        Color::yellowgreen};
}

// NOTE: Optimized thats
// TODO: Optimize to static members (release an reinitialize)
// Basic Colors
COLOR_IMPL_EXT Color::transparent(0, 0, 0, 0);
COLOR_IMPL_EXT Color::black(0, 0, 0);
COLOR_IMPL_EXT Color::silver(192, 192, 192);
COLOR_IMPL_EXT Color::gray(128, 128, 128);
COLOR_IMPL_EXT Color::white(255, 255, 255);
COLOR_IMPL_EXT Color::maroon(128, 0, 0);
COLOR_IMPL_EXT Color::red(255, 0, 0);
COLOR_IMPL_EXT Color::purple(128, 0, 128);
COLOR_IMPL_EXT Color::fuchsia(255, 0, 255);
COLOR_IMPL_EXT Color::green(0, 128, 0);
COLOR_IMPL_EXT Color::lime(0, 255, 0);
COLOR_IMPL_EXT Color::olive(128, 128, 0);
COLOR_IMPL_EXT Color::yellow(255, 255, 0);
COLOR_IMPL_EXT Color::navy(0, 0, 128);
COLOR_IMPL_EXT Color::blue(0, 0, 255);
COLOR_IMPL_EXT Color::teal(0, 128, 128);
COLOR_IMPL_EXT Color::aqua(0, 255, 255);

// Extended Colors
COLOR_IMPL_EXT Color::aliceblue(240, 248, 255);
COLOR_IMPL_EXT Color::antiquewhite(250, 235, 215);
COLOR_IMPL_EXT Color::aquamarine(127, 255, 212);
COLOR_IMPL_EXT Color::azure(240, 255, 255);
COLOR_IMPL_EXT Color::beige(245, 245, 220);
COLOR_IMPL_EXT Color::bisque(255, 228, 196);
COLOR_IMPL_EXT Color::blanchedalmond(255, 235, 205);
COLOR_IMPL_EXT Color::blueviolet(138, 43, 226);
COLOR_IMPL_EXT Color::brown(165, 42, 42);
COLOR_IMPL_EXT Color::burlywood(222, 184, 135);
COLOR_IMPL_EXT Color::cadetblue(95, 158, 160);
COLOR_IMPL_EXT Color::chartreuse(127, 255, 0);
COLOR_IMPL_EXT Color::chocolate(210, 105, 30);
COLOR_IMPL_EXT Color::coral(255, 127, 80);
COLOR_IMPL_EXT Color::cornflowerblue(100, 149, 237);
COLOR_IMPL_EXT Color::cornsilk(255, 248, 220);
COLOR_IMPL_EXT Color::crimson(220, 20, 60);
COLOR_IMPL_EXT Color::cyan(0, 255, 255);
COLOR_IMPL_EXT Color::darkblue(0, 0, 139);
COLOR_IMPL_EXT Color::darkcyan(0, 139, 139);
COLOR_IMPL_EXT Color::darkgoldenrod(184, 134, 11);
COLOR_IMPL_EXT Color::darkgray(169, 169, 169);
COLOR_IMPL_EXT Color::darkgreen(0, 100, 0);
COLOR_IMPL_EXT Color::darkgrey(169, 169, 169);
COLOR_IMPL_EXT Color::darkkhaki(189, 183, 107);
COLOR_IMPL_EXT Color::darkmagenta(139, 0, 139);
COLOR_IMPL_EXT Color::darkolivegreen(85, 107, 47);
COLOR_IMPL_EXT Color::darkorange(255, 140, 0);
COLOR_IMPL_EXT Color::darkorchid(153, 50, 204);
COLOR_IMPL_EXT Color::darkred(139, 0, 0);
COLOR_IMPL_EXT Color::darksalmon(233, 150, 122);
COLOR_IMPL_EXT Color::darkseagreen(143, 188, 143);
COLOR_IMPL_EXT Color::darkslateblue(72, 61, 139);
COLOR_IMPL_EXT Color::darkslategray(47, 79, 79);
COLOR_IMPL_EXT Color::darkslategrey(47, 79, 79);
COLOR_IMPL_EXT Color::darkturquoise(0, 206, 209);
COLOR_IMPL_EXT Color::darkviolet(148, 0, 211);
COLOR_IMPL_EXT Color::deeppink(255, 20, 147);
COLOR_IMPL_EXT Color::deepskyblue(0, 191, 255);
COLOR_IMPL_EXT Color::dimgray(105, 105, 105);
COLOR_IMPL_EXT Color::dimgrey(105, 105, 105);
COLOR_IMPL_EXT Color::dodgerblue(30, 144, 255);
COLOR_IMPL_EXT Color::firebrick(178, 34, 34);
COLOR_IMPL_EXT Color::floralwhite(255, 250, 240);
COLOR_IMPL_EXT Color::forestgreen(34, 139, 34);
COLOR_IMPL_EXT Color::gainsboro(220, 220, 220);
COLOR_IMPL_EXT Color::ghostwhite(248, 248, 255);
COLOR_IMPL_EXT Color::gold(255, 215, 0);
COLOR_IMPL_EXT Color::goldenrod(218, 165, 32);
COLOR_IMPL_EXT Color::greenyellow(173, 255, 47);
COLOR_IMPL_EXT Color::grey(128, 128, 128);
COLOR_IMPL_EXT Color::honeydew(240, 255, 240);
COLOR_IMPL_EXT Color::hotpink(255, 105, 180);
COLOR_IMPL_EXT Color::indianred(205, 92, 92);
COLOR_IMPL_EXT Color::indigo(75, 0, 130);
COLOR_IMPL_EXT Color::ivory(255, 255, 240);
COLOR_IMPL_EXT Color::khaki(240, 230, 140);
COLOR_IMPL_EXT Color::lavender(230, 230, 250);
COLOR_IMPL_EXT Color::lavenderblush(255, 240, 245);
COLOR_IMPL_EXT Color::lawngreen(124, 252, 0);
COLOR_IMPL_EXT Color::lemonchiffon(255, 250, 205);
COLOR_IMPL_EXT Color::lightblue(173, 216, 230);
COLOR_IMPL_EXT Color::lightcoral(240, 128, 128);
COLOR_IMPL_EXT Color::lightcyan(224, 255, 255);
COLOR_IMPL_EXT Color::lightgoldenrodyellow(250, 250, 210);
COLOR_IMPL_EXT Color::lightgray(211, 211, 211);
COLOR_IMPL_EXT Color::lightgreen(144, 238, 144);
COLOR_IMPL_EXT Color::lightgrey(211, 211, 211);
COLOR_IMPL_EXT Color::lightpink(255, 182, 193);
COLOR_IMPL_EXT Color::lightsalmon(255, 160, 122);
COLOR_IMPL_EXT Color::lightseagreen(32, 178, 170);
COLOR_IMPL_EXT Color::lightskyblue(135, 206, 250);
COLOR_IMPL_EXT Color::lightslategray(119, 136, 153);
COLOR_IMPL_EXT Color::lightslategrey(119, 136, 153);
COLOR_IMPL_EXT Color::lightsteelblue(176, 196, 222);
COLOR_IMPL_EXT Color::lightyellow(255, 255, 224);
COLOR_IMPL_EXT Color::limegreen(50, 205, 50);
COLOR_IMPL_EXT Color::linen(250, 240, 230);
COLOR_IMPL_EXT Color::magenta(255, 0, 255);
COLOR_IMPL_EXT Color::mediumaquamarine(102, 205, 170);
COLOR_IMPL_EXT Color::mediumblue(0, 0, 205);
COLOR_IMPL_EXT Color::mediumorchid(186, 85, 211);
COLOR_IMPL_EXT Color::mediumpurple(147, 112, 219);
COLOR_IMPL_EXT Color::mediumseagreen(60, 179, 113);
COLOR_IMPL_EXT Color::mediumslateblue(123, 104, 238);
COLOR_IMPL_EXT Color::mediumspringgreen(0, 250, 154);
COLOR_IMPL_EXT Color::mediumturquoise(72, 209, 204);
COLOR_IMPL_EXT Color::mediumvioletred(199, 21, 133);
COLOR_IMPL_EXT Color::midnightblue(25, 25, 112);
COLOR_IMPL_EXT Color::mintcream(245, 255, 250);
COLOR_IMPL_EXT Color::mistyrose(255, 228, 225);
COLOR_IMPL_EXT Color::moccasin(255, 228, 181);
COLOR_IMPL_EXT Color::navajowhite(255, 222, 173);
COLOR_IMPL_EXT Color::oldlace(253, 245, 230);
COLOR_IMPL_EXT Color::olivedrab(107, 142, 35);
COLOR_IMPL_EXT Color::orange(255, 165, 0);
COLOR_IMPL_EXT Color::orangered(255, 69, 0);
COLOR_IMPL_EXT Color::orchid(218, 112, 214);
COLOR_IMPL_EXT Color::palegoldenrod(238, 232, 170);
COLOR_IMPL_EXT Color::palegreen(152, 251, 152);
COLOR_IMPL_EXT Color::paleturquoise(175, 238, 238);
COLOR_IMPL_EXT Color::palevioletred(219, 112, 147);
COLOR_IMPL_EXT Color::papayawhip(255, 239, 213);
COLOR_IMPL_EXT Color::peachpuff(255, 218, 185);
COLOR_IMPL_EXT Color::peru(205, 133, 63);
COLOR_IMPL_EXT Color::pink(255, 192, 203);
COLOR_IMPL_EXT Color::plum(221, 160, 221);
COLOR_IMPL_EXT Color::powderblue(176, 224, 230);
COLOR_IMPL_EXT Color::rosybrown(188, 143, 143);
COLOR_IMPL_EXT Color::royalblue(65, 105, 225);
COLOR_IMPL_EXT Color::saddlebrown(139, 69, 19);
COLOR_IMPL_EXT Color::salmon(250, 128, 114);
COLOR_IMPL_EXT Color::sandybrown(244, 164, 96);
COLOR_IMPL_EXT Color::seagreen(46, 139, 87);
COLOR_IMPL_EXT Color::seashell(255, 245, 238);
COLOR_IMPL_EXT Color::sienna(160, 82, 45);
COLOR_IMPL_EXT Color::skyblue(135, 206, 235);
COLOR_IMPL_EXT Color::slateblue(106, 90, 205);
COLOR_IMPL_EXT Color::slategray(112, 128, 144);
COLOR_IMPL_EXT Color::slategrey(112, 128, 144);
COLOR_IMPL_EXT Color::snow(255, 250, 250);
COLOR_IMPL_EXT Color::springgreen(0, 255, 127);
COLOR_IMPL_EXT Color::steelblue(70, 130, 180);
COLOR_IMPL_EXT Color::tan(210, 180, 140);
COLOR_IMPL_EXT Color::thistle(216, 191, 216);
COLOR_IMPL_EXT Color::tomato(255, 99, 71);
COLOR_IMPL_EXT Color::turquoise(64, 224, 208);
COLOR_IMPL_EXT Color::violet(238, 130, 238);
COLOR_IMPL_EXT Color::wheat(245, 222, 179);
COLOR_IMPL_EXT Color::whitesmoke(245, 245, 245);
COLOR_IMPL_EXT Color::yellowgreen(154, 205, 50);
