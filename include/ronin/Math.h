#pragma once

#include "begin.h"
namespace RoninEngine::Runtime
{

    class RONIN_API Random
    {
    public:
        static void srand(std::uint32_t seed);

        static int range(int min, int max);

        static float range(float min, float max);

        static float value();

        static Vec2 random_vector();

        static float random_angle_deg();

        static float random_angle_rad();
    };

    class RONIN_API Math
    {
    public:
        static constexpr long double long_Pi = 3.14159265358979323846264338327950288;
        static constexpr float Pi = static_cast<float>(long_Pi); // calc PI formula l / d (Length circle div diameter)
        static constexpr float Pi2 = Pi * 2;
        static constexpr float infinity = std::numeric_limits<float>::infinity();
        static constexpr float negative_infinity = -infinity;
        static constexpr float deg2rad = 0.01745329;
        static constexpr float rad2deg = 180 / Pi;

        static float cos(float x);

        static float sin(float x);

        static float tan(float x);

        static float acos(float x);

        static float atan(float x);

        static float atan2(float y, float x);

        static float clamp01(float val);

        template <typename InT, typename OutT>
        static OutT map(const InT x, const InT in_min, const InT in_max, const OutT out_min, const OutT out_max)
        {
            return static_cast<OutT>((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
        }

        template <typename T>
        static T map(const T x, const T in_min, const T in_max, const T out_min, const T out_max)
        {
            return map<T, T>(x, in_min, in_max, out_min, out_max);
        }

        template <typename T>
        static T outside(T x, T minRange, T maxRange)
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
        static T clamp(const T &val, const T &min, const T &max)
        {
            return T(val > max ? max : val < min ? min : val);
        }
        template <typename T>
        static const T &max(const T &x, const T &y)
        {
            return x > y ? x : y;
        }

        template <typename T>
        static const T &min(const T &x, const T &y)
        {
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

        static float delta_angle(float current, float target);

        static float gamma(float value, float absmax, float gamma);

        static float inverse_lerp(float a, float b, float value);

        static float lerp(float a, float b, float t);

        static float lerp_angle(float a, float b, float t);

        static float lerp_unclamped(float a, float b, float t);

        static bool line_intersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 &result);

        static float move_towards(float current, float target, float maxDelta);

        static bool line_segment_intersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 &result);

        static float smooth_damp(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed);

        static float smooth_damp(float current, float target, float &currentVelocity, float smoothTime);

        static float smooth_damp(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

        // Celsius to Fahrenheit
        static float cel2far(float celsius);
        // Fahrenheit to Celsius
        static float far2cel(float fahrenheit);

        static float smooth_damp_angle(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed);

        static float smooth_damp_angle(float current, float target, float &currentVelocity, float smoothTime);

        static float smooth_damp_angle(
            float current, float target, float &currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

        static float smooth_step(float from, float to, float t);

        static std::uint64_t num_lastof(std::uint64_t number, int level);

        static std::string num_beautify(std::uint64_t num, char seperate = ' ', int digits = 3);

        static std::uint16_t float_to_half(float value);

        static float half_to_float(std::uint16_t value);

        static float square(float x);
        static float square_rectangle(float a, float b);
        static float square_rectangle(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4);
        static float square_circle(float radius);
        static float square_triangle(float base, float height);
        static float square_triangle(Vec2 p1, Vec2 p2, Vec2 p3);
        static float square_mesh(std::list<Vec2> &vec);
    };
} // namespace RoninEngine::Runtime
