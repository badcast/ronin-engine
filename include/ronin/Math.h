#pragma once

#include "begin.h"
namespace RoninEngine::Runtime
{

    class RONIN_API Random
    {
    public:
        static void SRand(std::uint32_t seed);
        static int Range(int min, int max);
        static float Range(float min, float max);
        static float Value();
        static Vec2 RandomVector();
        static float AngleDegress();
        static float AngleRadian();
        static Color color();
    };

    class RONIN_API Math
    {
    public:
        static constexpr long double longPi = 3.14159265358979323846264338327950288;
        static constexpr float Pi = static_cast<float>(longPi); // calc PI formula l / d (Length circle div diameter)
        static constexpr float Pi2 = Pi * 2;
        static constexpr float infinity = std::numeric_limits<float>::infinity();
        static constexpr float negativeInfinity = -infinity;
        static constexpr float deg2rad = 0.01745329;
        static constexpr float rad2deg = 180 / Pi;
        static constexpr float epsilon = 9.999999E-11; // 1e-10f

        static float Cos(float x);

        static float Sin(float x);

        static float Tan(float x);

        static float Acos(float x);

        static float Atan(float x);

        static float Atan2(float y, float x);

        template <typename InT, typename OutT>
        static OutT Map(const InT x, const InT in_min, const InT in_max, const OutT out_min, const OutT out_max)
        {
            return static_cast<OutT>((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
        }

        template <typename T>
        static T Map(const T x, const T in_min, const T in_max, const T out_min, const T out_max)
        {
            return Map<T, T>(x, in_min, in_max, out_min, out_max);
        }

        template <typename T>
        static T Outside(T x, T minRange, T maxRange)
        {
            if(minRange > maxRange)
                std::swap(minRange, maxRange);
            if(x >= minRange && x <= maxRange)
            {
                x = (minRange + maxRange) / 2 <= x ? maxRange : minRange;
            }

            return x;
        }

        template <typename T>
        static T OutsideNonZero(T x)
        {
            return Outside(x, static_cast<T>(1), static_cast<T>(-1));
        }

        template <typename T>
        static T Clamp(const T &val, const T &min, const T &max)
        {
            return T(val > max ? max : val < min ? min : val);
        }

        static float Clamp01(float val)
        {
            return Clamp(val, 0.f, 1.f);
        }

        template <typename T>
        static const T &Max(const T &x, const T &y)
        {
            return x > y ? x : y;
        }

        template <typename T>
        static const T &Min(const T &x, const T &y)
        {
            return x < y ? x : y;
        }

        static float Abs(float x);

        static int Abs(int x);

        // negative absolute value
        static float NAbs(float x);

        static int NAbs(int x);

        static float Signf(float x);

        static int Sign(int x);

        static float Round(float x);

        static float Ceil(float x);

        static float Floor(float x);

        static float Frac(float x);

        static int Number(float x);

        static float Exp(float x);

        static float Pow2(float x);

        static int Pow2(int x);

        static int Pow(int x, int y);

        static float Pow(float x, float y);

        static float Sqrt(float x);

        static float Repeat(float t, float length);

        static float DeltaAngle(float current, float target);

        static float Gamma(float value, float absmax, float gamma);

        static float InverseLerp(float a, float b, float value);

        static float Lerp(float a, float b, float t);

        static float LerpAngle(float a, float b, float t);

        static float LerpUnclamped(float a, float b, float t);

        static bool LineIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 &result);

        static float MoveTowards(float current, float target, float maxDelta);

        static bool LineSegmentIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 &result);

        static float SmoothDamp(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed);

        static float SmoothDamp(float current, float target, float &currentVelocity, float smoothTime);

        static float SmoothDamp(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

        // Celsius to Fahrenheit
        static float Cel2Far(float celsius);
        // Fahrenheit to Celsius
        static float Far2Cel(float fahrenheit);

        static float SmoothDampAngle(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed);

        static float SmoothDampAngle(float current, float target, float &currentVelocity, float smoothTime);

        static float SmoothDampAngle(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

        static float SmoothStep(float from, float to, float t);

        static std::uint64_t NumLastOf(std::uint64_t number, int level);

        static std::string NumBeautify(std::uint64_t num, char seperate = ' ', int digits = 3);

        static std::uint16_t FloatToHalf(float value);

        static float HalfToFloat(std::uint16_t value);

        static float Square(float x);
        static float SquareRectangle(float a, float b);
        static float SquareRectangle(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4);
        static float SquareCircle(float radius);
        static float SquareTriangle(float base, float height);
        static float SquareTriangle(Vec2 p1, Vec2 p2, Vec2 p3);
    };
} // namespace RoninEngine::Runtime
