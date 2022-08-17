#pragma once

#include "dependency.h"

namespace RoninEngine {
namespace Runtime {

struct Vec2Int {
   public:
    int x;
    int y;
    Vec2Int() : x(0), y(0) {}
    Vec2Int(int _x, int _y) : x(_x), y(_y) {}

    bool operator==(const Vec2Int& rhs) { return this->x == rhs.x && this->y == rhs.y; }

    bool operator!=(const Vec2Int& rhs) { return !this->operator==(rhs); }

    Vec2Int operator+(const Vec2Int& rhs) { return {this->x + rhs.x, this->y + rhs.y}; }

    Vec2Int operator-(const Vec2Int& rhs) { return {this->x - rhs.x, this->y + rhs.y}; }

    Vec2Int operator*(const int& rhs) { return {this->x * rhs, this->y * rhs}; }

    Vec2Int operator/(const int& rhs) { return {this->x / rhs, this->y / rhs}; }

    Vec2Int& operator+=(const Vec2Int& rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }

    Vec2Int& operator-=(const Vec2Int& rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        return *this;
    }

    Vec2Int& operator*=(const int& rhs) {
        this->x *= rhs;
        this->y *= rhs;
        return *this;
    }

    Vec2Int& operator/=(const int& rhs) {
        this->x /= rhs;
        this->y /= rhs;
        return *this;
    }
};

struct Vec2 {
    float x;
    float y;

    Vec2();
    explicit Vec2(const Vec2Int& rhs);
    Vec2(const float& x, const float& y);
    // extern Vec2(float && x, float && y);
    Vec2(const Vec2&) = default;
    ~Vec2() = default;

    float magnitude() const;
    float sqrMagnitude() const;
    Vec2 normalized();
    void Normalize();

    static const Vec2 one;
    static const Vec2 half;
    static const Vec2 minusOne;
    static const Vec2 zero;
    static const Vec2 down;
    static const Vec2 left;
    static const Vec2 right;
    static const Vec2 up;
    static const Vec2 infinity;
    static const Vec2 negativeInfinity;

    static const Vec2 Abs(const Vec2& value);
    static const Vec2 NAbs(const Vec2& value);
    static Vec2 Slerp(const Vec2& a, const Vec2& b, float t);
    static Vec2 SlerpUnclamped(const Vec2& a, const Vec2& b, float t);
    static Vec2 Lerp(const Vec2& a, const Vec2& b, float t);
    static Vec2 LerpUnclamped(const Vec2& a, const Vec2& b, float t);
    static Vec2 Max(const Vec2& lhs, const Vec2& rhs);
    static Vec2 Min(const Vec2& lhs, const Vec2& rhs);
    static Vec2 MoveTowards(const Vec2& current, const Vec2& target, float maxDistanceDelta);
    static Vec2 Reflect(const Vec2& inDirection, const Vec2& inNormal);
    static Vec2 Scale(const Vec2& a, const Vec2& b);
    static float Distance(const Vec2& lhs, const Vec2& rhs);
    static float DistanceSqr(const Vec2& lhs, const Vec2& rhs);
    static float Angle(Vec2 from, Vec2 to);
    static float SignedAngle(Vec2 from, Vec2 to);
    static Vec2 ClampMagnitude(Vec2 vector, float maxLength);
    static Vec2 SmoothDamp(Vec2 current, Vec2 target, Vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
    static float Dot(const Vec2& lhs, const Vec2& rhs);
    static float SqrMagnitude(const Vec2& lhs);
    static bool AreaHasIntersection(const SDL_Rect* A, const SDL_Rect* B);
    static bool AreaIntersectRect(const SDL_FRect& A, const SDL_FRect& B, SDL_FRect& result);
    static void Area_UnionRect(const SDL_Rect* A, const SDL_Rect* B, SDL_Rect* result);
    static Vec2 Round(Vec2 lhs);
    static Runtime::Vec2Int RoundToInt(const Vec2& lhs);
    //	static bool AreaEnclosePoints(const Rectf_t* points, int count, const
    // Rectf_t* clip, Rectf_t* result);
    static bool AreaPointInRect(const Vec2& p, const SDL_FRect& r);
    static bool InArea(const Vec2& p, const SDL_FRect& r);
    static const Vec2 Rotate(Vec2 vec, Vec2 normal, float angleRadian);
    static const Vec2 Rotate(Vec2 position,float angleRadian);
    static const Vec2 RotateClockwise(Vec2 position,float angleRadian);
    static const Vec2 RotateAround(Vec2 center, Vec2 localPosition, float angleRadian);

    static const Vec2 Perpendicular(Vec2 inDirection);

    Vec2& operator+=(const Vec2& rhs);
    Vec2& operator-=(const Vec2& rhs);
    Vec2& operator*=(const float& d);
    Vec2& operator/=(const float& d);
    Vec2& operator=(const Vec2& rhs);
};

//NOTE: Я не считаю это лучшим способом инициализировать переменные статических членов.
//TODO: Перевести строки ниже на функий get constexpr для предотвращения пре-инициализаций, пре-проверки
inline const Vec2 Vec2::one(1, 1);
inline const Vec2 Vec2::half(0.5f, 0.5f);
inline const Vec2 Vec2::minusOne(-1, -1);
inline const Vec2 Vec2::zero(0, 0);
inline const Vec2 Vec2::down(0, -1);
inline const Vec2 Vec2::left(-1, 0);
inline const Vec2 Vec2::right(1, 0);
inline const Vec2 Vec2::up(0, 1);
inline const Vec2 Vec2::infinity(1.f / 0.f, 1.f / 0.f);
inline const Vec2 Vec2::negativeInfinity(-1.f / 0.f, -1.f / 0.f);

Vec2Int operator+(const Vec2Int& lhs, const Vec2Int& rhs);
Vec2Int operator-(const Vec2Int& lhs, const Vec2Int& rhs);
bool operator==(const Vec2Int& lhs, const Vec2Int& rhs);
bool operator!=(const Vec2Int& lhs, const Vec2Int& rhs);
Vec2Int operator*(const float& d, const Vec2Int& rhs);
Vec2Int operator/(const float& d, const Vec2Int& rhs);
Vec2Int operator*(const Vec2Int& rhs, const float& d);
Vec2Int operator/(const Vec2Int& rhs, const float& d);

bool operator==(const Vec2Int& lhs, const Vec2& rhs);
bool operator!=(const Vec2Int& lhs, const Vec2& rhs);

Vec2 operator+(const Vec2& lhs, const Vec2& rhs);
Vec2 operator-(const Vec2& lhs, const Vec2& rhs);
Vec2 operator+(const Vec2& lhs, const Vec2Int& rhs);
Vec2 operator-(const Vec2& lhs, const Vec2Int& rhs);
bool operator==(const Vec2& lhs, const Vec2& rhs);
bool operator!=(const Vec2& lhs, const Vec2& rhs);
Vec2 operator*(const float& d, const Vec2& rhs);
Vec2 operator/(const float& d, const Vec2& rhs);
Vec2 operator*(const Vec2& rhs, const float& d);
Vec2 operator/(const Vec2& rhs, const float& d);

}  // namespace Runtime
}  // namespace RoninEngine

namespace std {
// this for Hashtable function
template <>
struct hash<RoninEngine::Runtime::Vec2> {
    std::size_t operator()(const RoninEngine::Runtime::Vec2& val) const noexcept {
        std::int64_t fake = (*reinterpret_cast<std::int64_t*>(&const_cast<RoninEngine::Runtime::Vec2&>(val)));
        return std::hash<std::int64_t>{}(fake);
    }
};

template <>
struct hash<RoninEngine::Runtime::Vec2Int> {
    std::size_t operator()(const RoninEngine::Runtime::Vec2Int& val) const noexcept {
        std::int64_t fake = (*reinterpret_cast<std::int64_t*>(&const_cast<RoninEngine::Runtime::Vec2Int&>(val)));
        return std::hash<std::int64_t>{}(fake);
    }
};

}  // namespace std
