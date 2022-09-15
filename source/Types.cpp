#include "framework.h"

namespace RoninEngine {
template <>
const xRect<float> xRect<float>::zero={};
template <>
const xRect<float> xRect<float>::one = {1, 1, 1, 1};

template <>
const xRect<int> xRect<int>::zero={};
template <>
const xRect<int> xRect<int>::one = {1, 1, 1, 1};

double Get_Angle(Vec2 lhs, Vec2 rhs) {
    double YEnd = rhs.y;
    double YStart = lhs.y;
    double XEnd = rhs.x;
    double XStart = lhs.x;

    double a = SDL_atan2(YEnd - YStart, XEnd - XStart);
    if (a < 0) a += 2 * M_PI;  // angle is now in radians

    a -= (M_PI / 2);  // shift by 90deg
    // restore value in range 0-2pi instead of -pi/2-3pi/2
    if (a < 0) a += 2 * M_PI;
    if (a < 0) a += 2 * M_PI;
    a = SDL_abs((M_PI * 2) - a);  // invert rotation
    a = a * 180 / M_PI;           // convert to deg

    return a;
}

}  // namespace RoninEngine
