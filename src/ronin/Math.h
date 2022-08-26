#pragma once

#include "dependency.h"

namespace RoninEngine::Runtime {

class Random {
   public:
    static void srand(int seed);

    static int range(int min, int max);

    static float range(float min, float max);

    static float value();

    static Vec2 randomVector();

    static float randomAngle();
};

class Mathf {
   public:
    static constexpr long double longPI = 3.14159265358979323846264338327950288;
    static constexpr float PI = static_cast<float>(longPI);  // calc PI formula l / d (Length circle div diameter)
    static constexpr float Infinity = std::numeric_limits<float>::infinity();
    static constexpr float NegativeInfinity = -Infinity;
    static constexpr float Deg2Rad = 0.01745329;
    static constexpr float Rad2Deg = 180 / PI;
    static constexpr int RLevelDigits = 10000;

    static float cos(float x);

    static float sin(float x);

    static float tan(float x);

    static float acos(float x);

    static float atan(float x);

    static float atan2(float y, float x);

    static float Clamp01(float val);

    template <typename T>
    static T map(const T& x, const T& in_min, const T& in_max, const T& out_min, const T& out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    template <typename T>
    static T outside(T x, T minRange, T maxRange) {
        if (minRange > maxRange) std::swap(minRange, maxRange);
        if (x >= minRange && x <= maxRange) {
            x = (minRange + maxRange) / 2 <= x ? maxRange : minRange;
        }

        return x;
    }

    static float Clamp(float val, const float min, const float max);

    template <typename T>
    static const T& max(const T& x, const T& y) {
        return x > y ? x : y;
    }

    template <typename T>
    static const T& min(const T& x, const T& y) {
        return x < y ? x : y;
    }

    static float abs(float x);

    static int abs(int x);

    // negative absolute value
    static float nabs(float x);

    static int nabs(int x);

    static float signf(float x);

    static int sign(int x);

    static float round(float x);

    static float ceil(float x);

    static float floor(float x);

    static float frac(float x);

    static int number(float x);

    static float exp(float x);

    static float pow2(float x);

    static int pow2(int x);

    static int pow(int x, int y);

    static float pow(float x, float y);

    static float sqrt(float x);

    static float repeat(float t, float length);

    static float DeltaAngle(float current, float target);

    static float Gamma(float value, float absmax, float gamma);

    static float InverseLerp(float a, float b, float value);

    static float Lerp(float a, float b, float t);

    static float LerpAngle(float a, float b, float t);

    static float LerpUnclamped(float a, float b, float t);

    static bool LineIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2& result);

    static float MoveTowards(float current, float target, float maxDelta);

    static bool LineSegmentIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2& result);

    static float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed);

    static float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime);

    static float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed,
                            float deltaTime);

    // Celsius to Fahrenheit
    static float cel2far(float celsius);
    // Fahre nheit to Celsius
    static float far2cel(float fahrenheit);

    static float SmoothDampAngle(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed);

    static float SmoothDampAngle(float current, float target, float& currentVelocity, float smoothTime);

    static float SmoothDampAngle(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed,
                                 float deltaTime);

    static float SmoothStep(float from, float to, float t);
};
}  // namespace RoninEngine::Runtime
