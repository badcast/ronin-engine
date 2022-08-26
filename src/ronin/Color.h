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

    // Basic Collors
    static const Color transparent;
    static const Color black;
    static const Color silver;
    static const Color gray;
    static const Color white;
    static const Color maroon;
    static const Color red;
    static const Color purple;
    static const Color fuchsia;
    static const Color green;
    static const Color lime;
    static const Color olive;
    static const Color yellow;
    static const Color navy;
    static const Color blue;
    static const Color teal;
    static const Color aqua;

    // Extendedcolors
    static const Color aliceblue;
    static const Color antiquewhite;
    static const Color aquamarine;
    static const Color azure;
    static const Color beige;
    static const Color bisque;
    static const Color blanchedalmond;
    static const Color blueviolet;
    static const Color brown;
    static const Color burlywood;
    static const Color cadetblue;
    static const Color chartreuse;
    static const Color chocolate;
    static const Color coral;
    static const Color cornflowerblue;
    static const Color cornsilk;
    static const Color crimson;
    static const Color cyan;
    static const Color darkblue;
    static const Color darkcyan;
    static const Color darkgoldenrod;
    static const Color darkgray;
    static const Color darkgreen;
    static const Color darkgrey;
    static const Color darkkhaki;
    static const Color darkmagenta;
    static const Color darkolivegreen;
    static const Color darkorange;
    static const Color darkorchid;
    static const Color darkred;
    static const Color darksalmon;
    static const Color darkseagreen;
    static const Color darkslateblue;
    static const Color darkslategray;
    static const Color darkslategrey;
    static const Color darkturquoise;
    static const Color darkviolet;
    static const Color deeppink;
    static const Color deepskyblue;
    static const Color dimgray;
    static const Color dimgrey;
    static const Color dodgerblue;
    static const Color firebrick;
    static const Color floralwhite;
    static const Color forestgreen;
    static const Color gainsboro;
    static const Color ghostwhite;
    static const Color gold;
    static const Color goldenrod;
    static const Color greenyellow;
    static const Color grey;
    static const Color honeydew;
    static const Color hotpink;
    static const Color indianred;
    static const Color indigo;
    static const Color ivory;
    static const Color khaki;
    static const Color lavender;
    static const Color lavenderblush;
    static const Color lawngreen;
    static const Color lemonchiffon;
    static const Color lightblue;
    static const Color lightcoral;
    static const Color lightcyan;
    static const Color lightgoldenrodyellow;
    static const Color lightgray;
    static const Color lightgreen;
    static const Color lightgrey;
    static const Color lightpink;
    static const Color lightsalmon;
    static const Color lightseagreen;
    static const Color lightskyblue;
    static const Color lightslategray;
    static const Color lightslategrey;
    static const Color lightsteelblue;
    static const Color lightyellow;
    static const Color limegreen;
    static const Color linen;
    static const Color magenta;
    static const Color mediumaquamarine;
    static const Color mediumblue;
    static const Color mediumorchid;
    static const Color mediumpurple;
    static const Color mediumseagreen;
    static const Color mediumslateblue;
    static const Color mediumspringgreen;
    static const Color mediumturquoise;
    static const Color mediumvioletred;
    static const Color midnightblue;
    static const Color mintcream;
    static const Color mistyrose;
    static const Color moccasin;
    static const Color navajowhite;
    static const Color oldlace;
    static const Color olivedrab;
    static const Color orange;
    static const Color orangered;
    static const Color orchid;
    static const Color palegoldenrod;
    static const Color palegreen;
    static const Color paleturquoise;
    static const Color palevioletred;
    static const Color papayawhip;
    static const Color peachpuff;
    static const Color peru;
    static const Color pink;
    static const Color plum;
    static const Color powderblue;
    static const Color rosybrown;
    static const Color royalblue;
    static const Color saddlebrown;
    static const Color salmon;
    static const Color sandybrown;
    static const Color seagreen;
    static const Color seashell;
    static const Color sienna;
    static const Color skyblue;
    static const Color slateblue;
    static const Color slategray;
    static const Color slategrey;
    static const Color snow;
    static const Color springgreen;
    static const Color steelblue;
    static const Color tan;
    static const Color thistle;
    static const Color tomato;
    static const Color turquoise;
    static const Color violet;
    static const Color wheat;
    static const Color whitesmoke;
    static const Color yellowgreen;
};
// NOTE: Optimized thats
// TODO: Optimize to static members (release an reinitialize)
// Basic Collors
inline const Color Color::transparent(0);
inline const Color Color::black(0, 0, 0);
inline const Color Color::silver(192, 192, 192);
inline const Color Color::gray(128, 128, 128);
inline const Color Color::white(255, 255, 255);
inline const Color Color::maroon(128, 0, 0);
inline const Color Color::red(255, 0, 0);
inline const Color Color::purple(128, 0, 128);
inline const Color Color::fuchsia(255, 0, 255);
inline const Color Color::green(0, 128, 0);
inline const Color Color::lime(0, 255, 0);
inline const Color Color::olive(128, 128, 0);
inline const Color Color::yellow(255, 255, 0);
inline const Color Color::navy(0, 0, 128);
inline const Color Color::blue(0, 0, 255);
inline const Color Color::teal(0, 128, 128);
inline const Color Color::aqua(0, 255, 255);

// Extendedcolors
const Color Color::aliceblue(240, 248, 255);
const Color Color::antiquewhite(250, 235, 215);
const Color Color::aquamarine(127, 255, 212);
const Color Color::azure(240, 255, 255);
const Color Color::beige(245, 245, 220);
const Color Color::bisque(255, 228, 196);
const Color Color::blanchedalmond(255, 235, 205);
const Color Color::blueviolet(138, 43, 226);
const Color Color::brown(165, 42, 42);
const Color Color::burlywood(222, 184, 135);
const Color Color::cadetblue(95, 158, 160);
const Color Color::chartreuse(127, 255, 0);
const Color Color::chocolate(210, 105, 30);
const Color Color::coral(255, 127, 80);
const Color Color::cornflowerblue(100, 149, 237);
const Color Color::cornsilk(255, 248, 220);
const Color Color::crimson(220, 20, 60);
const Color Color::cyan(0, 255, 255);
const Color Color::darkblue(0, 0, 139);
const Color Color::darkcyan(0, 139, 139);
const Color Color::darkgoldenrod(184, 134, 11);
const Color Color::darkgray(169, 169, 169);
const Color Color::darkgreen(0, 100, 0);
const Color Color::darkgrey(169, 169, 169);
const Color Color::darkkhaki(189, 183, 107);
const Color Color::darkmagenta(139, 0, 139);
const Color Color::darkolivegreen(85, 107, 47);
const Color Color::darkorange(255, 140, 0);
const Color Color::darkorchid(153, 50, 204);
const Color Color::darkred(139, 0, 0);
const Color Color::darksalmon(233, 150, 122);
const Color Color::darkseagreen(143, 188, 143);
const Color Color::darkslateblue(72, 61, 139);
const Color Color::darkslategray(47, 79, 79);
const Color Color::darkslategrey(47, 79, 79);
const Color Color::darkturquoise(0, 206, 209);
const Color Color::darkviolet(148, 0, 211);
const Color Color::deeppink(255, 20, 147);
const Color Color::deepskyblue(0, 191, 255);
const Color Color::dimgray(105, 105, 105);
const Color Color::dimgrey(105, 105, 105);
const Color Color::dodgerblue(30, 144, 255);
const Color Color::firebrick(178, 34, 34);
const Color Color::floralwhite(255, 250, 240);
const Color Color::forestgreen(34, 139, 34);
const Color Color::gainsboro(220, 220, 220);
const Color Color::ghostwhite(248, 248, 255);
const Color Color::gold(255, 215, 0);
const Color Color::goldenrod(218, 165, 32);
const Color Color::greenyellow(173, 255, 47);
const Color Color::grey(128, 128, 128);
const Color Color::honeydew(240, 255, 240);
const Color Color::hotpink(255, 105, 180);
const Color Color::indianred(205, 92, 92);
const Color Color::indigo(75, 0, 130);
const Color Color::ivory(255, 255, 240);
const Color Color::khaki(240, 230, 140);
const Color Color::lavender(230, 230, 250);
const Color Color::lavenderblush(255, 240, 245);
const Color Color::lawngreen(124, 252, 0);
const Color Color::lemonchiffon(255, 250, 205);
const Color Color::lightblue(173, 216, 230);
const Color Color::lightcoral(240, 128, 128);
const Color Color::lightcyan(224, 255, 255);
const Color Color::lightgoldenrodyellow(250, 250, 210);
const Color Color::lightgray(211, 211, 211);
const Color Color::lightgreen(144, 238, 144);
const Color Color::lightgrey(211, 211, 211);
const Color Color::lightpink(255, 182, 193);
const Color Color::lightsalmon(255, 160, 122);
const Color Color::lightseagreen(32, 178, 170);
const Color Color::lightskyblue(135, 206, 250);
const Color Color::lightslategray(119, 136, 153);
const Color Color::lightslategrey(119, 136, 153);
const Color Color::lightsteelblue(176, 196, 222);
const Color Color::lightyellow(255, 255, 224);
const Color Color::limegreen(50, 205, 50);
const Color Color::linen(250, 240, 230);
const Color Color::magenta(255, 0, 255);
const Color Color::mediumaquamarine(102, 205, 170);
const Color Color::mediumblue(0, 0, 205);
const Color Color::mediumorchid(186, 85, 211);
const Color Color::mediumpurple(147, 112, 219);
const Color Color::mediumseagreen(60, 179, 113);
const Color Color::mediumslateblue(123, 104, 238);
const Color Color::mediumspringgreen(0, 250, 154);
const Color Color::mediumturquoise(72, 209, 204);
const Color Color::mediumvioletred(199, 21, 133);
const Color Color::midnightblue(25, 25, 112);
const Color Color::mintcream(245, 255, 250);
const Color Color::mistyrose(255, 228, 225);
const Color Color::moccasin(255, 228, 181);
const Color Color::navajowhite(255, 222, 173);
const Color Color::oldlace(253, 245, 230);
const Color Color::olivedrab(107, 142, 35);
const Color Color::orange(255, 165, 0);
const Color Color::orangered(255, 69, 0);
const Color Color::orchid(218, 112, 214);
const Color Color::palegoldenrod(238, 232, 170);
const Color Color::palegreen(152, 251, 152);
const Color Color::paleturquoise(175, 238, 238);
const Color Color::palevioletred(219, 112, 147);
const Color Color::papayawhip(255, 239, 213);
const Color Color::peachpuff(255, 218, 185);
const Color Color::peru(205, 133, 63);
const Color Color::pink(255, 192, 203);
const Color Color::plum(221, 160, 221);
const Color Color::powderblue(176, 224, 230);
const Color Color::rosybrown(188, 143, 143);
const Color Color::royalblue(65, 105, 225);
const Color Color::saddlebrown(139, 69, 19);
const Color Color::salmon(250, 128, 114);
const Color Color::sandybrown(244, 164, 96);
const Color Color::seagreen(46, 139, 87);
const Color Color::seashell(255, 245, 238);
const Color Color::sienna(160, 82, 45);
const Color Color::skyblue(135, 206, 235);
const Color Color::slateblue(106, 90, 205);
const Color Color::slategray(112, 128, 144);
const Color Color::slategrey(112, 128, 144);
const Color Color::snow(255, 250, 250);
const Color Color::springgreen(0, 255, 127);
const Color Color::steelblue(70, 130, 180);
const Color Color::tan(210, 180, 140);
const Color Color::thistle(216, 191, 216);
const Color Color::tomato(255, 99, 71);
const Color Color::turquoise(64, 224, 208);
const Color Color::violet(238, 130, 238);
const Color Color::wheat(245, 222, 179);
const Color Color::whitesmoke(245, 245, 245);
const Color Color::yellowgreen(154, 205, 50);

}  // namespace RoninEngine::Runtime
