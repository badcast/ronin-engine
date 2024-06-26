#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

constexpr double periodic_fahren = 5.0 / 9;

float Math::Cos(float x)
{
    return SDL_cosf(x);
}

float Math::Sin(float x)
{
    return SDL_sinf(x);
}

float Math::Tan(float x)
{
    return SDL_tanf(x);
}

float Math::Acos(float x)
{
    return SDL_acosf(x);
}

float Math::Atan(float x)
{
    return SDL_atanf(x);
}

float Math::Atan2(float y, float x)
{
    return SDL_atan2f(y, x);
}

int Math::Abs(int x)
{
    return SDL_abs(x);
}

float Math::Abs(float x)
{
    return SDL_fabs(x);
}

// negative absolute value
float Math::NAbs(float x)
{
    return Abs(x) * ~0;
}

// negative absolute value
int Math::NAbs(int x)
{
    return Abs(x) * ~0;
}

float Math::Signf(float x)
{
    return x < 0.f ? -1.f : 1.f;
}

int Math::Sign(int x)
{
    return x < 0 ? -1 : 1;
}

float Math::Round(float x)
{
    return SDL_roundf(x);
}

float Math::Ceil(float x)
{
    return SDL_ceilf(x);
}

float Math::Floor(float x)
{
    return SDL_floorf(x);
}

float Math::Frac(float x)
{
    return x - Number(x);
}

int Math::Number(float x)
{
    return static_cast<int>(x);
}

float Math::Exp(float x)
{
    return SDL_expf(x);
}

float Math::Pow2(float x)
{
    return SDL_powf(x, 2);
}

int Math::Pow2(int x)
{
    return SDL_pow(x, 2);
}

int Math::Pow(int x, int y)
{
    return SDL_pow(x, y);
}

float Math::Pow(float x, float y)
{
    return SDL_powf(x, y);
}

float Math::Sqrt(float x)
{
    return SDL_sqrtf(x);
}

float Math::Repeat(float t, float length)
{
    return Clamp(t - Floor(t / length) * length, 0.f, length);
}

float Math::DeltaAngle(float current, float target)
{
    float num = Repeat(target - current, 360);
    if(num > 180)
    {
        num -= 360;
    }
    return num;
}

float Math::Gamma(float value, float absmax, float gamma)
{
    bool flag = false;
    if(value < 0)
    {
        flag = true;
    }
    float num = Abs(value);
    float result;
    if(num > absmax)
    {
        result = ((!flag) ? num : (-num));
    }
    else
    {
        float num2 = Pow(num / absmax, gamma) * absmax;
        result = ((!flag) ? num2 : (-num2));
    }
    return result;
}

float Math::InverseLerp(float a, float b, float value)
{
    float result;
    if(a != b)
    {
        result = Clamp01((value - a) / (b - a));
    }
    else
    {
        result = 0;
    }
    return result;
}

float Math::Lerp(float a, float b, float t)
{
    return a + (b - a) * Clamp01(t);
}

float Math::LerpAngle(float a, float b, float t)
{
    float num = Repeat(b - a, 360);
    if(num > 180)
    {
        num -= 360;
    }
    return a + num * Clamp01(t);
}

float Math::LerpUnclamped(float a, float b, float t)
{
    return a + (b - a) * t;
}

bool Math::LineIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 &result)
{
    float num = p2.x - p1.x;
    float num2 = p2.y - p1.y;
    float num3 = p4.x - p3.x;
    float num4 = p4.y - p3.y;
    float num5 = num * num4 - num2 * num3;
    bool result2;
    if(num5 == 0)
    {
        result2 = false;
    }
    else
    {
        float num6 = p3.x - p1.x;
        float num7 = p3.y - p1.y;
        float num8 = (num6 * num4 - num7 * num3) / num5;
        result = Vec2(p1.x + num8 * num, p1.y + num8 * num2);
        result2 = true;
    }
    return result2;
}

float Math::MoveTowards(float current, float target, float maxDelta)
{
    float result;
    if(Abs(target - current) <= maxDelta)
    {
        result = target;
    }
    else
    {
        result = current + Sign(target - current) * maxDelta;
    }
    return result;
}

bool Math::LineSegmentIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 &result)
{
    float num = p2.x - p1.x;
    float num2 = p2.y - p1.y;
    float num3 = p4.x - p3.x;
    float num4 = p4.y - p3.y;
    float num5 = num * num4 - num2 * num3;
    bool result2;
    if(num5 == 0)
    {
        result2 = false;
    }
    else
    {
        float num6 = p3.x - p1.x;
        float num7 = p3.y - p1.y;
        float num8 = (num6 * num4 - num7 * num3) / num5;
        if(num8 < 0 || num8 > 1)
        {
            result2 = false;
        }
        else
        {
            float num9 = (num6 * num2 - num7 * num) / num5;
            if(num9 < 0 || num9 > 1)
            {
                result2 = false;
            }
            else
            {
                result = Vec2(p1.x + num8 * num, p1.y + num8 * num2);
                result2 = true;
            }
        }
    }
    return result2;
}

float Math::SmoothDamp(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed)
{
    return Math::SmoothDamp(current, target, currentVelocity, smoothTime, maxSpeed, Time::deltaTime());
}

float Math::SmoothDamp(float current, float target, float &currentVelocity, float smoothTime)
{
    float maxSpeed = infinity;
    return Math::SmoothDamp(current, target, currentVelocity, smoothTime, maxSpeed, Time::deltaTime());
}

float Math::SmoothDamp(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
{
    smoothTime = Max(0.f, Min(1.f, smoothTime));
    float num = 2 / smoothTime;
    float num2 = num * deltaTime;
    float num3 = 1 / (1 + num2 + 0, 48 * num2 * num2 + 0, 235 * num2 * num2 * num2);
    float num4 = current - target;
    float num5 = target;
    float num6 = maxSpeed * smoothTime;
    num4 = Clamp(num4, -num6, num6);
    target = current - num4;
    float num7 = (currentVelocity + num * num4) * deltaTime;
    currentVelocity = (currentVelocity - num * num7) * num3;
    float num8 = target + (num4 + num7) * num3;
    if(num5 - current > 0 == num8 > num5)
    {
        num8 = num5;
        currentVelocity = (num8 - num5) / deltaTime;
    }
    return num8;
}

float Math::Cel2Far(float celsius)
{
    return celsius * 1.8f + 32;
}

float Math::Far2Cel(float fahrenheit)
{
    return (fahrenheit - 32) * periodic_fahren;
}

float Math::SmoothDampAngle(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed)
{
    return SmoothDampAngle(current, target, currentVelocity, smoothTime, maxSpeed, Time::deltaTime());
}

float Math::SmoothDampAngle(float current, float target, float &currentVelocity, float smoothTime)
{
    float maxSpeed = infinity;
    return SmoothDampAngle(current, target, currentVelocity, smoothTime, maxSpeed, Time::deltaTime());
}

float Math::SmoothDampAngle(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
{
    target = current + DeltaAngle(current, target);
    return SmoothDamp(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
}

float Math::SmoothStep(float from, float to, float t)
{
    t = Clamp01(t);
    t = -2 * t * t * t + 3 * t * t;
    return to * t + from * (1 - t);
}

std::uint64_t Math::NumLastOf(std::uint64_t number, int level)
{
    std::uint64_t x = std::pow(10, level);
    auto d = std::ldiv(number, x);
    if(number < x / 10)
        d.rem = 0;
    return d.rem;
}

std::string Math::NumBeautify(std::uint64_t num, char seperate, int digits)
{
    constexpr int radix = 10;
    int level = 1;
    std::string result {};
    std::div_t dv;

    while((num = (dv = std::div(num, radix)).quot))
    {
        result.insert(0, std::to_string(dv.rem));

        if(level % digits == 0)
            result.insert(0, 1, seperate);
        ++level;
    }
    result.insert(0, std::to_string(dv.rem));
    return result;
}

uint16_t Math::FloatToHalf(float value)
{
    uint16_t sign = (std::copysign(1.0f, value) > 0.0f ? 0x0000 : 0x8000);

    // Infinity
    if(std::isinf(value))
    {
        return sign | 0x7C00;
    }
    // NaN
    else if(std::isnan(value))
    {
        return sign | 0x7FFF;
    }

    int exponent;

    float significand = std::fabs(std::frexp(value, &exponent));

    // Exponent bias
    exponent += 15;

    // Crosses upper boundary, clamp to infinity
    if(exponent > 31)
    {
        return sign | 0x7C00;
    }
    // Crosses lower boundary, clamp to zero
    else if(exponent <= 0)
    {
        return sign | 0x0000;
    }
    // Zero
    else if(significand < 0.25f)
    {
        return sign | 0x0000;
    }

    // Normal value
    uint16_t mantissa = static_cast<uint16_t>(std::ldexp(2 * significand - 1, 10));

    uint16_t bits = sign | mantissa | ((exponent - 1) << 10);

    return bits;
}

float Math::HalfToFloat(uint16_t value)
{
    int exponent = (value >> 10) & 0x001F;
    int mantissa = (value >> 0) & 0x03FF;

    float result;

    // Zero
    if((exponent == 0) && (mantissa == 0))
    {
        result = 0.0f;
    }
    // Subnormal
    else if((exponent == 0) && (mantissa != 0))
    {
        result = std::ldexp(static_cast<float>(mantissa), -24);
    }
    // Infinity
    else if((exponent == 31) && (mantissa == 0))
    {
        result = std::numeric_limits<float>::infinity();
    }
    // NaN
    else if((exponent == 31) && (mantissa != 0))
    {
        result = std::nanf("");
    }
    // Normal number
    else
    {
        result = std::ldexp(static_cast<float>(mantissa | 0x0400), exponent - 25);
    }

    float sign = ((value & 0x8000) == 0 ? 1.0f : -1.0f);

    return std::copysignf(result, sign);
}

float Math::SquareTriangle(float base, float height)
{
    return base * height / 2;
}

float Math::Square(float x)
{
    return x * x;
}

float Math::SquareRectangle(float a, float b)
{
    return a * b;
}

float Math::SquareCircle(float radius)
{
    /*
     *      get the square circle
     *      S = PI * r^2
     *      S = PI * (d/2)^2
     *      S = (PI * d^2) / 4
     *
     */
    return Math::Pi * Math::Pow2(radius);
}
