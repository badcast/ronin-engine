#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{

#define COLOR_IMPL API_EXPORT static const
#define COLOR_IMPL_EXT API_EXPORT inline const Color

    struct RONIN_API Color
    {
        using value_type = std::uint8_t;

        /**
         * @brief Red Color component
         */
        value_type r;
        /**
         * @brief Green Color component
         */
        value_type g;
        /**
         * @brief Blue Color component
         */
        value_type b;
        /**
         * @brief Alpha Transparent Color component
         */
        value_type a;

        Color();

        Color(const Color &);

        Color(Color &&);

        Color(const Color &from, value_type a);

        Color(int rgba);

        Color(int rgb, value_type a);

        Color(const char *hex);

        Color(const std::string &hex);

        Color(const native_color_t &color);

        Color(value_type r, value_type g, value_type b);

        Color(value_type r, value_type g, value_type b, value_type a);

        /**
         * @brief Converts the color to its hexadecimal representation.
         * @return The color in hexadecimal format (e.g., "#RRGGBBAA").
         */
        std::string toHex() const;

        /**
         * @brief Converts the color to its integer RGBA representation.
         * @return The color in RGBA.
         */
        int toRGBA() const;

        /**
         * @brief Converts the color to its integer ARGB representation.
         * @return The color in ARGB.
         */
        int toARGB() const;

        /**
         * @brief Converts the color to its HSV representation.
         * @return Color in HSV color space (h,s,v)
         */
        std::tuple<float, float, float> toHSV() const;

        Color operator=(const Color &rhs);
        Color operator=(const int &rhs);
        Color operator=(const native_color_t &rhs);

        Color operator+(const Color &rhs);
        Color operator-(const Color &rhs);

        bool operator==(const Color &rhs);
        bool operator!=(const Color &rhs);

        operator int() const;
        operator native_color_t() const;

        /**
         * @brief Make Color to its MakeTransparency
         * @param color The color to be make
         * @param alpha Value from 0.0 to 1.0 (Clamp)
         * @return The color maked of transparency
         */
        static Color MakeTransparency(const Color &color, float alpha);

        /**
         * @brief Converts the color to its hexadecimal representation.
         *
         * @param color The color to be converted.
         * @param appendAlpha If true, put the color alpha component, false otherwise.
         * @return The color in hexadecimal format (e.g., "#RRGGBBAA").
         */
        static std::string ColorToHex(const Color &color, bool appendAlpha = true);

        /**
         * @brief Converts a hexadecimal color value to a Color object.
         *
         * @param hex The hexadecimal color value (e.g., "#RRGGBB", "RRGGBB" or "#RRGGBBAA").
         * @return The Color object representing the converted color.
         */
        static Color HexToColor(const char *hex);

        /**
         * @brief Converts a hexadecimal color value to a Color object.
         *
         * @param hex The hexadecimal color value (e.g., "#RRGGBB", "RRGGBB" or "#RRGGBBAA").
         * @return The Color object representing the converted color.
         */
        static Color HexToColor(const std::string &hex);

        /**
         * @brief Converts HSV (Hue, Saturation, Value) color representation to RGB (Red, Green, Blue).
         *
         * @param h Hue value in the range [0, 360].
         * @param s Saturation value in the range [0, 1].
         * @param v Value (brightness) value in the range [0, 1].
         *
         * @return Color object in RGB representation.
         *
         * This function takes HSV color values and converts them to RGB color representation.
         * The input hue value should be in the range of [0, 360], where 0 corresponds to red,
         * 120 to green, and 240 to blue. The saturation value should be in the range of [0, 1],
         * where 0 represents grayscale and 1 represents full saturation. The value (brightness)
         * value should also be in the range of [0, 1], where 0 is black and 1 is the maximum brightness.
         *
         * The function returns a Color object with the corresponding RGB values after the conversion.
         */
        static Color HSVToRGB(float h, float s, float v);

        /**
         * @brief Convert RGB color components to HSV (Hue, Saturation, Value) color space.
         *
         * @param r Red color component (0-255)
         * @param g Green color component (0-255)
         * @param b Blue color component (0-255)
         * @return Color in HSV color space (h,s,v)
         */
        static std::tuple<float, float, float> RGBToHSV(std::uint8_t r, value_type g, value_type b);

        /**
         * @brief Create a Color object from an integer value in the RGBA format.
         *
         * @param rgba The integer value representing the color in the RGBA format.
         * @return The Color object corresponding to the given RGBA value.
         */
        static Color FromRGBA(int rgba);

        /**
         * @brief Create a Color object from an integer value in the ARGB format.
         *
         * @param argb The integer value representing the color in the ARGB format.
         * @return The Color object corresponding to the given ARGB value.
         */
        static Color FromARGB(int argb);

        /**
         * @brief  Converts HSV (Hue, Saturation, Value) color representation to RGB (Red, Green, Blue).
         * @param h Hue value in the range [0, 360].
         * @param s Saturation value in the range [0, 1].
         * @param v Value (brightness) value in the range [0, 1].
         * @return Color object in RGB representation.
         */
        static Color FromHSV(float h, float s, float v);

        /**
         * @brief Linearly interpolates between two Color objects.
         *
         * This function performs linear interpolation (Lerp) between two Color objects.
         * It takes two Color objects, `start` and `end`, and a parameter `t` that
         * determines how close the result is to `end`. The parameter `t` should be
         * in the range [0, 1]. When `t` is 0, the result is equal to `start`, and
         * when `t` is 1, the result is equal to `end`.
         *
         * @param start The starting Color.
         * @param end The ending Color.
         * @param t The interpolation parameter in the range [0, 1].
         *
         * @return The interpolated Color.
         *
         * Example usage:
         * @code
         * RoninEngine::Runtime::Color color1(255, 0, 0);  // Red color
         * RoninEngine::Runtime::Color color2(0, 0, 255);  // Blue color
         *
         * float t = 0.5f;  // Interpolate halfway between color1 and color2
         * RoninEngine::Runtime::Color lerpedColor = RoninEngine::Runtime::Color::Lerp(color1, color2, t);
         * @endcode
         */
        static Color Lerp(Color start, Color end, float t);

        // Basic Colors
        COLOR_IMPL Color transparent;
        COLOR_IMPL Color black;
        COLOR_IMPL Color silver;
        COLOR_IMPL Color gray;
        COLOR_IMPL Color white;
        COLOR_IMPL Color maroon;
        COLOR_IMPL Color red;
        COLOR_IMPL Color purple;
        COLOR_IMPL Color fuchsia;
        COLOR_IMPL Color green;
        COLOR_IMPL Color lime;
        COLOR_IMPL Color olive;
        COLOR_IMPL Color yellow;
        COLOR_IMPL Color navy;
        COLOR_IMPL Color blue;
        COLOR_IMPL Color teal;
        COLOR_IMPL Color aqua;

        // Extended Colors
        COLOR_IMPL Color aliceblue;
        COLOR_IMPL Color antiquewhite;
        COLOR_IMPL Color aquamarine;
        COLOR_IMPL Color azure;
        COLOR_IMPL Color beige;
        COLOR_IMPL Color bisque;
        COLOR_IMPL Color blanchedalmond;
        COLOR_IMPL Color blueviolet;
        COLOR_IMPL Color brown;
        COLOR_IMPL Color burlywood;
        COLOR_IMPL Color cadetblue;
        COLOR_IMPL Color chartreuse;
        COLOR_IMPL Color chocolate;
        COLOR_IMPL Color coral;
        COLOR_IMPL Color cornflowerblue;
        COLOR_IMPL Color cornsilk;
        COLOR_IMPL Color crimson;
        COLOR_IMPL Color cyan;
        COLOR_IMPL Color darkblue;
        COLOR_IMPL Color darkcyan;
        COLOR_IMPL Color darkgoldenrod;
        COLOR_IMPL Color darkgray;
        COLOR_IMPL Color darkgreen;
        COLOR_IMPL Color darkgrey;
        COLOR_IMPL Color darkkhaki;
        COLOR_IMPL Color darkmagenta;
        COLOR_IMPL Color darkolivegreen;
        COLOR_IMPL Color darkorange;
        COLOR_IMPL Color darkorchid;
        COLOR_IMPL Color darkred;
        COLOR_IMPL Color darksalmon;
        COLOR_IMPL Color darkseagreen;
        COLOR_IMPL Color darkslateblue;
        COLOR_IMPL Color darkslategray;
        COLOR_IMPL Color darkslategrey;
        COLOR_IMPL Color darkturquoise;
        COLOR_IMPL Color darkviolet;
        COLOR_IMPL Color deeppink;
        COLOR_IMPL Color deepskyblue;
        COLOR_IMPL Color dimgray;
        COLOR_IMPL Color dimgrey;
        COLOR_IMPL Color dodgerblue;
        COLOR_IMPL Color firebrick;
        COLOR_IMPL Color floralwhite;
        COLOR_IMPL Color forestgreen;
        COLOR_IMPL Color gainsboro;
        COLOR_IMPL Color ghostwhite;
        COLOR_IMPL Color gold;
        COLOR_IMPL Color goldenrod;
        COLOR_IMPL Color greenyellow;
        COLOR_IMPL Color grey;
        COLOR_IMPL Color honeydew;
        COLOR_IMPL Color hotpink;
        COLOR_IMPL Color indianred;
        COLOR_IMPL Color indigo;
        COLOR_IMPL Color ivory;
        COLOR_IMPL Color khaki;
        COLOR_IMPL Color lavender;
        COLOR_IMPL Color lavenderblush;
        COLOR_IMPL Color lawngreen;
        COLOR_IMPL Color lemonchiffon;
        COLOR_IMPL Color lightblue;
        COLOR_IMPL Color lightcoral;
        COLOR_IMPL Color lightcyan;
        COLOR_IMPL Color lightgoldenrodyellow;
        COLOR_IMPL Color lightgray;
        COLOR_IMPL Color lightgreen;
        COLOR_IMPL Color lightgrey;
        COLOR_IMPL Color lightpink;
        COLOR_IMPL Color lightsalmon;
        COLOR_IMPL Color lightseagreen;
        COLOR_IMPL Color lightskyblue;
        COLOR_IMPL Color lightslategray;
        COLOR_IMPL Color lightslategrey;
        COLOR_IMPL Color lightsteelblue;
        COLOR_IMPL Color lightyellow;
        COLOR_IMPL Color limegreen;
        COLOR_IMPL Color linen;
        COLOR_IMPL Color magenta;
        COLOR_IMPL Color mediumaquamarine;
        COLOR_IMPL Color mediumblue;
        COLOR_IMPL Color mediumorchid;
        COLOR_IMPL Color mediumpurple;
        COLOR_IMPL Color mediumseagreen;
        COLOR_IMPL Color mediumslateblue;
        COLOR_IMPL Color mediumspringgreen;
        COLOR_IMPL Color mediumturquoise;
        COLOR_IMPL Color mediumvioletred;
        COLOR_IMPL Color midnightblue;
        COLOR_IMPL Color mintcream;
        COLOR_IMPL Color mistyrose;
        COLOR_IMPL Color moccasin;
        COLOR_IMPL Color navajowhite;
        COLOR_IMPL Color oldlace;
        COLOR_IMPL Color olivedrab;
        COLOR_IMPL Color orange;
        COLOR_IMPL Color orangered;
        COLOR_IMPL Color orchid;
        COLOR_IMPL Color palegoldenrod;
        COLOR_IMPL Color palegreen;
        COLOR_IMPL Color paleturquoise;
        COLOR_IMPL Color palevioletred;
        COLOR_IMPL Color papayawhip;
        COLOR_IMPL Color peachpuff;
        COLOR_IMPL Color peru;
        COLOR_IMPL Color pink;
        COLOR_IMPL Color plum;
        COLOR_IMPL Color powderblue;
        COLOR_IMPL Color rosybrown;
        COLOR_IMPL Color royalblue;
        COLOR_IMPL Color saddlebrown;
        COLOR_IMPL Color salmon;
        COLOR_IMPL Color sandybrown;
        COLOR_IMPL Color seagreen;
        COLOR_IMPL Color seashell;
        COLOR_IMPL Color sienna;
        COLOR_IMPL Color skyblue;
        COLOR_IMPL Color slateblue;
        COLOR_IMPL Color slategray;
        COLOR_IMPL Color slategrey;
        COLOR_IMPL Color snow;
        COLOR_IMPL Color springgreen;
        COLOR_IMPL Color steelblue;
        COLOR_IMPL Color tan;
        COLOR_IMPL Color thistle;
        COLOR_IMPL Color tomato;
        COLOR_IMPL Color turquoise;
        COLOR_IMPL Color violet;
        COLOR_IMPL Color wheat;
        COLOR_IMPL Color whitesmoke;
        COLOR_IMPL Color yellowgreen;
    };

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

#undef COLOR_IMPL
#undef COLOR_IMPL_EXT
} // namespace RoninEngine::Runtime
