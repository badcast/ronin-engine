#include "ronin.h"

using namespace RoninEngine;
using namespace RoninEngine::Exception;
using namespace RoninEngine::Runtime;

constexpr auto periodic_fahren = 5.0 / 9;

// get random value
inline int get_internal_rand()
{
    int result = rand();
#ifdef WIN32
    // WIN32 RAND_MAX = 0x7fff - ISSUE
    result |= (result << 16);
#endif
    return result;
}

void Random::srand(int seed)
{
    std::srand(static_cast<uint32_t>(seed));
}

int Random::range(int min, int max)
{
    int result = Math::number(value() * (max - min)) + min;
    return result;
}

float Random::range(float min, float max)
{
    float result = value() * (max - min) + min;
    return result;
}

float Random::value()
{
    float result = get_internal_rand() * static_cast<float>(4.6566128752458E-10);
    return result;
}

Vec2 Random::random_vector()
{
    return Vec2(Random::value() * 2 - 1, Random::value() * 2 - 1);
}

float Random::random_angle_deg()
{
    return value() * 360;
}

float Random::random_angle_rad()
{
    return value() * Math::Pi2;
}

float Math::cos(float x)
{
    return SDL_cosf(x);
}

float Math::sin(float x)
{
    return SDL_sinf(x);
}

float Math::tan(float x)
{
    return SDL_tanf(x);
}

float Math::acos(float x)
{
    return SDL_acosf(x);
}

float Math::atan(float x)
{
    return SDL_atanf(x);
}

float Math::atan2(float y, float x)
{
    return SDL_atan2f(y, x);
}

float Math::clamp01(float val)
{
    return clamp(val, 0.f, 1.f);
}

int Math::abs(int x)
{
    return std::abs(x);
}

float Math::abs(float x)
{
    return std::abs(x);
}

// negative absolute value
float Math::nabs(float x)
{
    return abs(x) * ~0;
}

// negative absolute value
int Math::nabs(int x)
{
    return abs(x) * ~0;
}

float Math::signf(float x)
{
    return x < 0.f ? -1.f : 1.f;
}

int Math::sign(int x)
{
    return x < 0 ? -1 : 1;
}

float Math::round(float x)
{
    return SDL_roundf(x);
}

float Math::ceil(float x)
{
    return SDL_ceilf(x);
}

float Math::floor(float x)
{
    return SDL_floorf(x);
}

float Math::frac(float x)
{
    return x - number(x);
}

int Math::number(float x)
{
    return static_cast<int>(x);
}

float Math::exp(float x)
{
    return SDL_expf(x);
}

float Math::pow2(float x)
{
    return SDL_powf(x, 2);
}

int Math::pow2(int x)
{
    return SDL_pow(x, 2);
}

int Math::pow(int x, int y)
{
    return SDL_pow(x, y);
}

float Math::pow(float x, float y)
{
    return SDL_powf(x, y);
}

float Math::sqrt(float x)
{
    return SDL_sqrtf(x);
}

float Math::repeat(float t, float length)
{
    return clamp(t - floor(t / length) * length, 0.f, length);
}

float Math::delta_angle(float current, float target)
{
    float num = repeat(target - current, 360);
    if(num > 180)
    {
        num -= 360;
    }
    return num;
}

float Math::gamma(float value, float absmax, float gamma)
{
    bool flag = false;
    if(value < 0)
    {
        flag = true;
    }
    float num = abs(value);
    float result;
    if(num > absmax)
    {
        result = ((!flag) ? num : (-num));
    }
    else
    {
        float num2 = pow(num / absmax, gamma) * absmax;
        result = ((!flag) ? num2 : (-num2));
    }
    return result;
}

float Math::inverse_lerp(float a, float b, float value)
{
    float result;
    if(a != b)
    {
        result = clamp01((value - a) / (b - a));
    }
    else
    {
        result = 0;
    }
    return result;
}

float Math::lerp(float a, float b, float t)
{
    return a + (b - a) * clamp01(t);
}

float Math::lerp_angle(float a, float b, float t)
{
    float num = repeat(b - a, 360);
    if(num > 180)
    {
        num -= 360;
    }
    return a + num * clamp01(t);
}

float Math::lerp_unclamped(float a, float b, float t)
{
    return a + (b - a) * t;
}

bool Math::line_intersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 &result)
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

float Math::move_towards(float current, float target, float maxDelta)
{
    float result;
    if(abs(target - current) <= maxDelta)
    {
        result = target;
    }
    else
    {
        result = current + sign(target - current) * maxDelta;
    }
    return result;
}

bool Math::line_segment_intersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 &result)
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

float Math::smooth_damp(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed)
{
    return Math::smooth_damp(current, target, currentVelocity, smoothTime, maxSpeed, TimeEngine::deltaTime());
}

float Math::smooth_damp(float current, float target, float &currentVelocity, float smoothTime)
{
    float maxSpeed = infinity;
    return Math::smooth_damp(current, target, currentVelocity, smoothTime, maxSpeed, TimeEngine::deltaTime());
}

float Math::smooth_damp(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
{
    smoothTime = max(0.f, max(1.f, smoothTime));
    float num = 2 / smoothTime;
    float num2 = num * deltaTime;
    float num3 = 1 / (1 + num2 + 0, 48 * num2 * num2 + 0, 235 * num2 * num2 * num2);
    float num4 = current - target;
    float num5 = target;
    float num6 = maxSpeed * smoothTime;
    num4 = clamp(num4, -num6, num6);
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

float Math::cel2far(float celsius)
{
    return celsius * 1.8f + 32;
}

float Math::far2cel(float fahrenheit)
{
    return (fahrenheit - 32) * periodic_fahren;
}

float Math::smooth_damp_angle(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed)
{
    return smooth_damp_angle(current, target, currentVelocity, smoothTime, maxSpeed, TimeEngine::deltaTime());
}

float Math::smooth_damp_angle(float current, float target, float &currentVelocity, float smoothTime)
{
    float maxSpeed = infinity;
    return smooth_damp_angle(current, target, currentVelocity, smoothTime, maxSpeed, TimeEngine::deltaTime());
}

float Math::smooth_damp_angle(float current, float target, float &currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
{
    target = current + delta_angle(current, target);
    return smooth_damp(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
}

float Math::smooth_step(float from, float to, float t)
{
    t = clamp01(t);
    t = -2 * t * t * t + 3 * t * t;
    return to * t + from * (1 - t);
}

std::uint64_t Math::num_lastof(std::uint64_t number, int level)
{
    std::uint64_t x = std::pow(10, level);
    auto d = std::ldiv(number, x);
    if(number < x / 10)
        d.rem = 0;
    return d.rem;
}

std::string Math::num_beautify(std::uint64_t num, char seperate, int digits)
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

uint16_t Math::float_to_half(float value)
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

float Math::half_to_float(uint16_t value)
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

float Math::square_triangle(float base, float height)
{
    return base * height / 2;
}
float Math::square(float x)
{
    return x * x;
}
float Math::square_rectangle(float a, float b)
{
    return a * b;
}
float Math::square_circle(float radius)
{
    /*
     *      get the square circle
     *      S = PI * r^2
     *      S = PI * (d/2)^2
     *      S = (PI * d^2) / 4
     *
     */
    return Math::Pi * Math::pow2(radius);
}
float Math::square_mesh(std::list<Vec2> &vecs)
{
    /*
     *    get the square a customize figure
     *    S = square
     *    S1 = cell square
     *    n = closed cell
     *    r = partially closed cell
     *    S = S1 * (n + 1/2 * r)
     *
     */

    float S = -1;

    // TODO: написать алгоритм измерение площади произвольным фигурам
    throw ronin_implementation_error();
    return S;
}
