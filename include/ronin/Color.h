#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{

#define COLOR_IMPL static const

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
        static std::tuple<float, float, float> RGBToHSV(value_type r, value_type g, value_type b);

        /**
         * @brief Converts HSV (Hue, Saturation, Value) color representation to RGB (Red, Green, Blue) with half float.
         *
         * @param h Hue value in the range (Computed from Math::FloatToHalf).
         * @param s Saturation value in the range (Computed from Math::FloatToHalf).
         * @param v Value (brightness) value in the range (Computed from Math::FloatToHalf).
         *
         * @see Math::FloatToHalf
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
        static Color HSVToRGBHalf(std::int16_t h, std::int16_t s, std::int16_t v);

        /**
         * @brief Convert RGB color components to HSV (Hue, Saturation, Value) color space with half float.
         *
         * @param r Red color component (0-255)
         * @param g Green color component (0-255)
         * @param b Blue color component (0-255)
         * @return Color in HSV color space (h,s,v)
         */
        static std::tuple<std::int16_t, std::int16_t, std::int16_t> RGBToHSVHalf(value_type r, value_type g, value_type b);

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

        /**
         * @brief Entries the basic colors.
         * @return List of the Colors
         */
        static std::vector<Color> GetBasicColors();

        /**
         * @brief Entries the extended colors.
         * @return List of the Colors
         */
        static std::vector<Color> GetExtendedColors();

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

#undef COLOR_IMPL

} // namespace RoninEngine::Runtime
