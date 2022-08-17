#include "Mathf.h"

#include "framework.h"

namespace RoninEngine {
// get random value
int getRVal() {
    int value;
    char* p = reinterpret_cast<char*>(&value);
    p[0] = (char)rand() % 0xff;
    p[1] = (char)rand() % 0xff;
    p[2] = (char)rand() % 0xff;
    p[3] = (char)rand() % 0xff;
    return Mathf::abs(value);
}

void Random::srand(int seed) { std::srand(static_cast<uint32_t>(seed)); }

int Random::range(int min, int max) {
    int result = Mathf::number(value() * (max - min)) + min;
    return result;
}

float Random::range(float min, float max) {
    float result = value() * (max - min) + min;
    return result;
}

float Random::value() {
    float result = getRVal() * static_cast<float>(4.6566128752458E-10);
    return result;
}

Vec2 Random::randomVector() { return Vec2(Random::value() * 2 - 1, Random::value() * 2 - 1); }

float Random::randomAngle() { return value() * 360; }

bool Mathf::Approximately(float a, float b) {
    return Mathf::abs(b - a) < Mathf::max(1E-06f * Mathf::max(Mathf::abs(a), Mathf::abs(b)), Epsilon * 8);
}

float Mathf::cos(float x) { return ::cosf(x); }

float Mathf::sin(float x) { return ::sinf(x); }

float Mathf::tan(float x) { return ::tanf(x); }

float Mathf::acos(float x) { return ::acosf(x); }

float Mathf::atan(float x) { return ::atanf(x); }

float Mathf::atan2(float y, float x) { return ::atan2f(y, x); }

float Mathf::Clamp01(float val) { return Clamp(val, 0, 1.f); }

float Mathf::Clamp(float val, const float min, const float max) { return val > max ? max : val < min ? min : val; }

int Mathf::abs(int x) { return x * (x < 0 ? -1 : 1); }

float Mathf::abs(float x) { return x * (x < 0 ? -1 : 1); }

// negative absolute value
float Mathf::nabs(float x) { return abs(x) * ~0; }

// negative absolute value
int Mathf::nabs(int x) { return abs(x) * ~0; }

float Mathf::signf(float x) { return x < 0.f ? -1.f : 1.f; }

int Mathf::sign(int x) { return x < 0 ? -1 : 1; }

float Mathf::round(float x) { return ::SDL_roundf(x); }

float Mathf::ceil(float x) { return SDL_ceilf(x); }

float Mathf::floor(float x) { return SDL_floorf(x); }

float Mathf::frac(float x) { return x - number(x); }

int Mathf::number(float x) { return static_cast<int>(x); }

float Mathf::exp(float x) { return ::expf(x); }

float Mathf::pow2(float x) { return ::powf(x, 2); }

int Mathf::pow2(int x) { return ::pow(x, 2); }

int Mathf::pow(int x, int y) { return ::pow(x, y); }

float Mathf::pow(float x, float y) { return ::powf(x, y); }

float Mathf::sqrt(float x) { return ::SDL_sqrtf(x); }

float Mathf::repeat(float t, float length) { return Clamp(t - floor(t / length) * length, 0, length); }

float Mathf::DeltaAngle(float current, float target) {
    float num = repeat(target - current, 360);
    if (num > 180) {
        num -= 360;
    }
    return num;
}

float Mathf::Gamma(float value, float absmax, float gamma) {
    bool flag = false;
    if (value < 0) {
        flag = true;
    }
    float num = abs(value);
    float result;
    if (num > absmax) {
        result = ((!flag) ? num : (-num));
    } else {
        float num2 = pow(num / absmax, gamma) * absmax;
        result = ((!flag) ? num2 : (-num2));
    }
    return result;
}

float Mathf::InverseLerp(float a, float b, float value) {
    float result;
    if (a != b) {
        result = Clamp01((value - a) / (b - a));
    } else {
        result = 0;
    }
    return result;
}

float Mathf::Lerp(float a, float b, float t) { return a + (b - a) * Clamp01(t); }

float Mathf::LerpAngle(float a, float b, float t) {
    float num = repeat(b - a, 360);
    if (num > 180) {
        num -= 360;
    }
    return a + num * Clamp01(t);
}

float Mathf::LerpUnclamped(float a, float b, float t) { return a + (b - a) * t; }

bool Mathf::LineIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2& result) {
    float num = p2.x - p1.x;
    float num2 = p2.y - p1.y;
    float num3 = p4.x - p3.x;
    float num4 = p4.y - p3.y;
    float num5 = num * num4 - num2 * num3;
    bool result2;
    if (num5 == 0) {
        result2 = false;
    } else {
        float num6 = p3.x - p1.x;
        float num7 = p3.y - p1.y;
        float num8 = (num6 * num4 - num7 * num3) / num5;
        result = Vec2(p1.x + num8 * num, p1.y + num8 * num2);
        result2 = true;
    }
    return result2;
}

float Mathf::MoveTowards(float current, float target, float maxDelta) {
    float result;
    if (abs(target - current) <= maxDelta) {
        result = target;
    } else {
        result = current + sign(target - current) * maxDelta;
    }
    return result;
}

bool Mathf::LineSegmentIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2& result) {
    float num = p2.x - p1.x;
    float num2 = p2.y - p1.y;
    float num3 = p4.x - p3.x;
    float num4 = p4.y - p3.y;
    float num5 = num * num4 - num2 * num3;
    bool result2;
    if (num5 == 0) {
        result2 = false;
    } else {
        float num6 = p3.x - p1.x;
        float num7 = p3.y - p1.y;
        float num8 = (num6 * num4 - num7 * num3) / num5;
        if (num8 < 0 || num8 > 1) {
            result2 = false;
        } else {
            float num9 = (num6 * num2 - num7 * num) / num5;
            if (num9 < 0 || num9 > 1) {
                result2 = false;
            } else {
                result = Vec2(p1.x + num8 * num, p1.y + num8 * num2);
                result2 = true;
            }
        }
    }
    return result2;
}

float Mathf::SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed) {
    float deltaTime = RoninEngine::Time::deltaTime();
    return Mathf::SmoothDamp(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
}

float Mathf::SmoothDamp(float current, float target, float& currentVelocity, float smoothTime) {
    float deltaTime = RoninEngine::Time::deltaTime();
    float maxSpeed = Infinity;
    return Mathf::SmoothDamp(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
}

float Mathf::SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed,
                        float deltaTime) {
    smoothTime = max(0.f, max(1.f, smoothTime));
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
    if (num5 - current > 0 == num8 > num5) {
        num8 = num5;
        currentVelocity = (num8 - num5) / deltaTime;
    }
    return num8;
}

float Mathf::SmoothDampAngle(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed) {
    float deltaTime = Time::deltaTime();
    return SmoothDampAngle(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
}

float Mathf::SmoothDampAngle(float current, float target, float& currentVelocity, float smoothTime) {
    float deltaTime = Time::deltaTime();
    float maxSpeed = Infinity;
    return SmoothDampAngle(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
}

float Mathf::SmoothDampAngle(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed,
                             float deltaTime) {
    target = current + DeltaAngle(current, target);
    return SmoothDamp(current, target, currentVelocity, smoothTime, maxSpeed, deltaTime);
}

float Mathf::SmoothStep(float from, float to, float t) {
    t = Clamp01(t);
    t = -2 * t * t * t + 3 * t * t;
    return to * t + from * (1 - t);
}

}  // namespace RoninEngine
