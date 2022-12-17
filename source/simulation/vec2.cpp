#include "Vec2.h"

#include "framework.h"

using namespace RoninEngine;

// const Vec2 Vec2::one = Vec2(1, 1);
// const Vec2 Vec2::half = Vec2::one / 2;
// const Vec2 Vec2::minusOne = Vec2(-1, -1);
// const Vec2 Vec2::zero = Vec2(0, 0);
// const Vec2 Vec2::down = Vec2(0, -1);
// const Vec2 Vec2::left = Vec2(-1, 0);
// const Vec2 Vec2::right = Vec2(1, 0);
// const Vec2 Vec2::up = Vec2(0, 1);
// const Vec2 Vec2::infinity = Vec2(Mathf::Infinity, Mathf::Infinity);
// const Vec2 Vec2::negativeInfinity = infinity * -1;

int countRun = 0;

Vec2::Vec2() : x(0), y(0) {}

Vec2::Vec2(const Vec2Int& rhs) : x(rhs.x), y(rhs.y) {}

Vec2::Vec2(const float& x, const float& y) {
    this->x = x;
    this->y = y;
    ++countRun;
}

float Vec2::magnitude() const { return Math::sqrt(x * x + y * y); }

float Vec2::sqrMagnitude() const { return x * x + y * y; }

Vec2 Vec2::normalized() {
    Vec2 result = *this;
    result.Normalize();
    return result;
}

void Vec2::Normalize() {
    float mag = magnitude();
    if (mag > 1E-05) {
        *this /= mag;
    } else {
        *this = Vec2::zero;
    }
}

const Vec2 Vec2::Abs(const Vec2& value) { return {Math::abs(value.x), Math::abs(value.y)}; }

const Vec2 Vec2::NAbs(const Vec2& value) { return {Math::nabs(value.x), Math::nabs(value.y)}; }

Vec2 Vec2::Slerp(const Vec2& a, const Vec2& b, float t) {
    t = Math::Clamp01(t);

    // get cosine of angle between disposition (-1 -> 1)
    float CosAlpha = Dot(a, b);
    // get angle (0 -> pi)
    float Alpha = Math::acos(CosAlpha);
    // get sine of angle between disposition (0 -> 1)
    float SinAlpha = Math::sin(Alpha);
    // this breaks down when SinAlpha = 0, i.e. Alpha = 0 or pi
    float t1 = Math::sin(1.f - t * Alpha) / SinAlpha;
    float t2 = Math::sin(t * Alpha) / SinAlpha;

    // interpolate src disposition
    return a * t1 + b * t2;
}
Vec2 Vec2::SlerpUnclamped(const Vec2& a, const Vec2& b, float t) {
    // get cosine of angle between disposition (-1 -> 1)
    float CosAlpha = Dot(a, b);
    // get angle (0 -> pi)
    float Alpha = Math::acos(CosAlpha);
    // get sine of angle between disposition (0 -> 1)
    float SinAlpha = Math::sin(Alpha);
    // this breaks down when SinAlpha = 0, i.e. Alpha = 0 or pi
    float t1 = Math::sin(1.f - t * Alpha) / SinAlpha;
    float t2 = Math::sin(t * Alpha) / SinAlpha;

    // interpolate src disposition
    return a * t1 + b * t2;
}
Vec2 Vec2::Lerp(const Vec2& a, const Vec2& b, float t) {
    t = Math::Clamp01(t);
    return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

Vec2 Vec2::LerpUnclamped(const Vec2& a, const Vec2& b, float t) { return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }

Vec2 Vec2::Max(const Vec2& lhs, const Vec2& rhs) { return Vec2(Math::max(lhs.x, rhs.x), Math::max(lhs.y, rhs.y)); }

Vec2 Vec2::Min(const Vec2& lhs, const Vec2& rhs) { return Vec2(Math::min(lhs.x, rhs.x), Math::min(lhs.y, rhs.y)); }

Vec2 Vec2::MoveTowards(const Vec2& current, const Vec2& target, float maxDistanceDelta) {
    Vec2 a = target - current;
    float mag = a.magnitude();
    Vec2 result;
    if (mag <= maxDistanceDelta || mag == 0) {
        result = target;
    } else {
        result = current + a / mag * maxDistanceDelta;
    }
    return result;
}

Vec2 Vec2::Reflect(const Vec2& inDirection, const Vec2& inNormal) {
    return -2 * Dot(inNormal, inDirection) * inNormal + inDirection;
}

Vec2 Vec2::Scale(const Vec2& a, const Vec2& b) { return Vec2(a.x * b.x, a.y * b.y); }

float Vec2::Distance(const Vec2& lhs, const Vec2& rhs) {
    float num = lhs.x - rhs.x;
    float num2 = lhs.y - rhs.y;
    return Math::sqrt(num * num + num2 * num2);
}

float Vec2::DistanceSqr(const Vec2& lhs, const Vec2& rhs) {
    float num = lhs.x - rhs.x;
    float num2 = lhs.y - rhs.y;
    return num * num + num2 * num2;
}

float Vec2::Angle(Vec2 from, Vec2 to) {
    from.Normalize();
    to.Normalize();
    return Math::acos(Math::Clamp(Vec2::Dot(from, to), -1.f, 1.f));
}

float Vec2::SignedAngle(Vec2 from, Vec2 to) {
    float num = Angle(from, to);
    from.Normalize();
    to.Normalize();
    float num2 = Math::sign(from.x * to.y - from.y * to.x);

    return num * num2;
}

Vec2 Vec2::ClampMagnitude(Vec2 vector, float maxLength) {
    Vec2 result;
    if (vector.sqrMagnitude() > maxLength * maxLength) {
        result = vector.normalized() * maxLength;
    } else {
        result = vector;
    }
    return result;
}

Vec2 Vec2::SmoothDamp(Vec2 current, Vec2 target, Vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime) {
    smoothTime = Math::max(0.f, Math::max(0001.f, smoothTime));
    float num = 2 / smoothTime;
    float num2 = num * deltaTime;
    float d = 1 / (1 + num2 + 0, 48 * num2 * num2 + 0, 235 * num2 * num2 * num2);
    Vec2 vector = current - target;
    Vec2 vector2 = target;
    float maxLength = maxSpeed * smoothTime;
    vector = ClampMagnitude(vector, maxLength);
    target = current - vector;
    Vec2 vector3 = (currentVelocity + num * vector) * deltaTime;
    currentVelocity = (currentVelocity - num * vector3) * d;
    Vec2 vector4 = target + (vector + vector3) * d;
    if (Vec2::Dot(vector2 - current, vector4 - vector2) > 0) {
        vector4 = vector2;
        currentVelocity = (vector4 - vector2) / deltaTime;
    }
    return vector4;
}

float Vec2::Dot(const Vec2& lhs, const Vec2& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }

float Vec2::SqrMagnitude(const Vec2& lhs) { return lhs.x * lhs.x + lhs.y * lhs.y; }

Vec2 Vec2::Round(Vec2 lhs) {
    lhs.x = (float)static_cast<int>(lhs.x);
    lhs.y = (float)static_cast<int>(lhs.y);
    return lhs;
}

Vec2Int Vec2::RoundToInt(const Vec2& lhs) {
    Vec2Int p;
    p.x = static_cast<int>(lhs.x);
    p.y = static_cast<int>(lhs.y);
    return p;
}

bool Vec2::AreaPointInRect(const Vec2& p, const SDL_FRect& r) {
    return ((p.x >= r.x) && (p.x < (r.x + r.w)) && (p.y >= r.y) && (p.y < (r.y + r.h)));
}

bool Vec2::InArea(const Vec2& p, const SDL_FRect& r) { return p.x >= r.x && p.x <= r.w && p.y >= r.h && p.y <= r.y; }

const Vec2 Vec2::Rotate(Vec2 vec, Vec2 normal, float angleRadian) {
    normal = Vec2::RotateClockwise(normal,angleRadian * Math::Deg2Rad);
    normal.x *= vec.x;
    normal.y *= vec.y;
    return normal;
}

const Vec2 Vec2::Rotate(Vec2 position,float angleRadian) {
    float Cos = Math::cos(angleRadian);
    float Sin = Math::sin(angleRadian);
    position.x = position.x * Cos - position.y * Sin;
    position.y = position.x * Sin + position.y * Cos;
    return position;
}

const Vec2 Vec2::RotateClockwise(Vec2 position,float angleRadian) {
    float Cos = Math::cos(angleRadian);
    float Sin = Math::sin(angleRadian);
    position.x = position.x * Cos + position.y * Sin;
    position.y = -position.x * Sin + position.y * Cos;
    return position;
}

const Vec2 Vec2::RotateAround(Vec2 center, Vec2 localPosition, float angleRadian) {
    float Cos = Math::cos(angleRadian);
    float Sin = Math::sin(angleRadian);
    center.x = center.x + localPosition.x * Sin;
    center.y = center.y + localPosition.y * Cos;
    return center;
}

const Vec2 Vec2::Perpendicular(Vec2 inDirection) { return Vec2(0.f - inDirection.y, inDirection.x); }

Vec2& Vec2::operator+=(const Vec2& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
}
Vec2& Vec2::operator-=(const Vec2& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this;
}

Vec2& Vec2::operator*=(const float& d) {
    this->x *= d;
    this->y *= d;
    return *this;
}

Vec2& Vec2::operator/=(const float& d) {
    this->x /= d;
    this->y /= d;
    return *this;
}

Vec2& Vec2::operator=(const Vec2& rhs) {
    this->x = rhs.x;
    this->y = rhs.y;
    return *this;
}

Vec2 Vec2::operator -() const
{
    return *this * -1;
}

Vec2 RoninEngine::Runtime::operator+(const Vec2& lhs, const Vec2& rhs) { return Vec2(lhs.x + rhs.x, lhs.y + rhs.y); }

Vec2 RoninEngine::Runtime::operator-(const Vec2& lhs, const Vec2& rhs) { return Vec2(lhs.x - rhs.x, lhs.y - rhs.y); }

Vec2 RoninEngine::Runtime::operator+(const Vec2& lhs, const Vec2Int& rhs) { return Vec2(lhs.x + rhs.x, lhs.y + rhs.y); }

Vec2 RoninEngine::Runtime::operator-(const Vec2& lhs, const Vec2Int& rhs) { return Vec2(lhs.x - rhs.x, lhs.y - rhs.y); }

bool RoninEngine::Runtime::operator==(const Vec2& lhs, const Vec2& rhs) { return (lhs - rhs).sqrMagnitude() < 9.999999E-11; }

bool RoninEngine::Runtime::operator!=(const Vec2& lhs, const Vec2& rhs) { return !(operator==(lhs, rhs)); }

Vec2 RoninEngine::Runtime::operator*(const float& d, const Vec2& rhs) { return Vec2(rhs.x * d, rhs.y * d); }

Vec2 RoninEngine::Runtime::operator/(const float& d, const Vec2& rhs) { return Vec2(rhs.x / d, rhs.y / d); }

Vec2 RoninEngine::Runtime::operator*(const Vec2& rhs, const float& d) { return d * rhs; }

Vec2 RoninEngine::Runtime::operator/(const Vec2& rhs, const float& d) { return d / rhs; }

bool RoninEngine::Runtime::operator==(const Vec2Int& lhs, const Vec2& rhs) { return rhs.x == lhs.x && rhs.y == lhs.y; }

bool RoninEngine::Runtime::operator!=(const Vec2Int& lhs, const Vec2& rhs) { return !operator==(lhs, rhs); }

Vec2Int RoninEngine::Runtime::operator+(const Vec2Int& lhs, const Vec2Int& rhs) {
    return Vec2Int(lhs.x + rhs.x, lhs.y + rhs.y);
}

Vec2Int RoninEngine::Runtime::operator-(const Vec2Int& lhs, const Vec2Int& rhs) {
    return Vec2Int(lhs.x - rhs.x, lhs.y - rhs.y);
}

bool RoninEngine::Runtime::operator==(const Vec2Int& lhs, const Vec2Int& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

bool RoninEngine::Runtime::operator!=(const Vec2Int& lhs, const Vec2Int& rhs) { return !(operator==(lhs, rhs)); }

Vec2Int RoninEngine::Runtime::operator*(const float& d, const Vec2Int& rhs) { return Vec2Int(rhs.x * d, rhs.y * d); }

Vec2Int RoninEngine::Runtime::operator/(const float& d, const Vec2Int& rhs) { return Vec2Int(rhs.x / d, rhs.y / d); }

Vec2Int RoninEngine::Runtime::operator*(const Vec2Int& rhs, const float& d) { return d * rhs; }

Vec2Int RoninEngine::Runtime::operator/(const Vec2Int& rhs, const float& d) { return d / rhs; }