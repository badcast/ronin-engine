#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {

        struct RONIN_API Vec2Int {
        public:
            int x;
            int y;
            Vec2Int() = default;
            Vec2Int(const int& _x, const int& _y)
                : x(_x)
                , y(_y)
            {
            }
            Vec2Int(const Vec2Int&) = default;

            bool operator==(const Vec2Int& rhs) { return this->x == rhs.x && this->y == rhs.y; }

            bool operator!=(const Vec2Int& rhs) { return !this->operator==(rhs); }

            Vec2Int operator+(const Vec2Int& rhs) { return { this->x + rhs.x, this->y + rhs.y }; }

            Vec2Int operator-(const Vec2Int& rhs) { return { this->x - rhs.x, this->y + rhs.y }; }

            Vec2Int operator*(const int& rhs) { return { this->x * rhs, this->y * rhs }; }

            Vec2Int operator/(const int& rhs) { return { this->x / rhs, this->y / rhs }; }

            Vec2Int& operator+=(const Vec2Int& rhs)
            {
                this->x += rhs.x;
                this->y += rhs.y;
                return *this;
            }

            Vec2Int& operator-=(const Vec2Int& rhs)
            {
                this->x -= rhs.x;
                this->y -= rhs.y;
                return *this;
            }

            Vec2Int& operator*=(const int& rhs)
            {
                this->x *= rhs;
                this->y *= rhs;
                return *this;
            }

            Vec2Int& operator/=(const int& rhs)
            {
                this->x /= rhs;
                this->y /= rhs;
                return *this;
            }
        };

        struct RONIN_API Vec2 {
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

            RONIN_API static const Vec2 one;
            RONIN_API static const Vec2 half;
            RONIN_API static const Vec2 minusOne;
            RONIN_API static const Vec2 zero;
            RONIN_API static const Vec2 down;
            RONIN_API static const Vec2 left;
            RONIN_API static const Vec2 right;
            RONIN_API static const Vec2 up;
            RONIN_API static const Vec2 infinity;
            RONIN_API static const Vec2 negativeInfinity;

            RONIN_API static const Vec2 Abs(const Vec2& value);
            RONIN_API static const Vec2 NAbs(const Vec2& value);
            RONIN_API static Vec2 Slerp(const Vec2& a, const Vec2& b, float t);
            RONIN_API static Vec2 SlerpUnclamped(const Vec2& a, const Vec2& b, float t);
            RONIN_API static Vec2 Lerp(const Vec2& a, const Vec2& b, float t);
            RONIN_API static Vec2 LerpUnclamped(const Vec2& a, const Vec2& b, float t);
            RONIN_API static Vec2 Max(const Vec2& lhs, const Vec2& rhs);
            RONIN_API static Vec2 Min(const Vec2& lhs, const Vec2& rhs);
            RONIN_API static Vec2 MoveTowards(const Vec2& current, const Vec2& target, float maxDistanceDelta);
            RONIN_API static Vec2 Reflect(const Vec2& inDirection, const Vec2& inNormal);
            RONIN_API static Vec2 Scale(const Vec2& a, const Vec2& b);
            RONIN_API static float Distance(const Vec2& lhs, const Vec2& rhs);
            RONIN_API static float DistanceSqr(const Vec2& lhs, const Vec2& rhs);
            RONIN_API static float Angle(Vec2 from, Vec2 to);
            RONIN_API static float SignedAngle(Vec2 from, Vec2 to);
            RONIN_API static Vec2 ClampMagnitude(Vec2 vector, float maxLength);
            RONIN_API static Vec2 SmoothDamp(Vec2 current, Vec2 target, Vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
            RONIN_API static float Dot(const Vec2& lhs, const Vec2& rhs);
            RONIN_API static float SqrMagnitude(const Vec2& lhs);
            RONIN_API static Vec2 Round(Vec2 lhs);
            RONIN_API static Runtime::Vec2Int RoundToInt(const Vec2& lhs);
            //	static bool AreaEnclosePoints(const Rectf_t* points, int count, const
            // Rectf_t* clip, Rectf_t* result);
            RONIN_API static bool AreaPointInRect(const Vec2& p, const Rectf_t& r);
            RONIN_API static bool HasIntersection(const Rectf_t& lhs, const Rectf_t& rhs);
            RONIN_API static bool InArea(const Vec2& p, const Rectf_t& r);
            RONIN_API static const Vec2 Rotate(Vec2 vec, Vec2 normal, float angleRadian);
            RONIN_API static const Vec2 Rotate(Vec2 position, float angleRadian);
            RONIN_API static const Vec2 RotateClockwise(Vec2 position, float angleRadian);
            RONIN_API static const Vec2 RotateAround(Vec2 center, Vec2 localPosition, float angleRadian);

            RONIN_API static const Vec2 Perpendicular(Vec2 inDirection);

            RONIN_API Vec2& operator+=(const Vec2& rhs);
            RONIN_API Vec2& operator-=(const Vec2& rhs);
            RONIN_API Vec2& operator*=(const float& d);
            RONIN_API Vec2& operator/=(const float& d);
            RONIN_API Vec2& operator=(const Vec2& rhs);

            RONIN_API Vec2 operator-() const;
        };

        // NOTE: Я не считаю это лучшим способом инициализировать переменные статических членов.
        // TODO: Перевести строки ниже на функий get constexpr для предотвращения пре-инициализаций, пре-проверки
        API_EXPORT inline const Vec2 Vec2::one(1, 1);
        API_EXPORT inline const Vec2 Vec2::half(0.5f, 0.5f);
        API_EXPORT inline const Vec2 Vec2::minusOne(-1, -1);
        API_EXPORT inline const Vec2 Vec2::zero(0, 0);
        API_EXPORT inline const Vec2 Vec2::down(0, -1);
        API_EXPORT inline const Vec2 Vec2::left(-1, 0);
        API_EXPORT inline const Vec2 Vec2::right(1, 0);
        API_EXPORT inline const Vec2 Vec2::up(0, 1);
        API_EXPORT inline const Vec2 Vec2::infinity(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
        API_EXPORT inline const Vec2 Vec2::negativeInfinity(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

        RONIN_API Vec2Int operator+(const Vec2Int& lhs, const Vec2Int& rhs);
        RONIN_API Vec2Int operator-(const Vec2Int& lhs, const Vec2Int& rhs);
        RONIN_API bool operator==(const Vec2Int& lhs, const Vec2Int& rhs);
        RONIN_API bool operator!=(const Vec2Int& lhs, const Vec2Int& rhs);
        RONIN_API Vec2Int operator*(const float& d, const Vec2Int& rhs);
        RONIN_API Vec2Int operator/(const float& d, const Vec2Int& rhs);
        RONIN_API Vec2Int operator*(const Vec2Int& rhs, const float& d);
        RONIN_API Vec2Int operator/(const Vec2Int& rhs, const float& d);

        RONIN_API bool operator==(const Vec2Int& lhs, const Vec2& rhs);
        RONIN_API bool operator!=(const Vec2Int& lhs, const Vec2& rhs);

        RONIN_API Vec2 operator+(const Vec2& lhs, const Vec2& rhs);
        RONIN_API Vec2 operator-(const Vec2& lhs, const Vec2& rhs);
        RONIN_API Vec2 operator+(const Vec2& lhs, const Vec2Int& rhs);
        RONIN_API Vec2 operator-(const Vec2& lhs, const Vec2Int& rhs);
        RONIN_API bool operator==(const Vec2& lhs, const Vec2& rhs);
        RONIN_API bool operator!=(const Vec2& lhs, const Vec2& rhs);
        RONIN_API Vec2 operator*(const float& d, const Vec2& rhs);
        RONIN_API Vec2 operator/(const float& d, const Vec2& rhs);
        RONIN_API Vec2 operator*(const Vec2& rhs, const float& d);
        RONIN_API Vec2 operator/(const Vec2& rhs, const float& d);

    } // namespace Runtime
} // namespace RoninEngine

namespace std
{
    // this for Hashtable function
    template <>
    struct hash<RoninEngine::Runtime::Vec2> {
        std::size_t operator()(const RoninEngine::Runtime::Vec2& val) const noexcept
        {
            std::int64_t fake = (*reinterpret_cast<std::int64_t*>(&const_cast<RoninEngine::Runtime::Vec2&>(val)));
            return std::hash<std::int64_t> {}(fake);
        }
    };

    template <>
    struct hash<RoninEngine::Runtime::Vec2Int> {
        std::size_t operator()(const RoninEngine::Runtime::Vec2Int& val) const noexcept
        {
            std::int64_t fake = (*reinterpret_cast<std::int64_t*>(&const_cast<RoninEngine::Runtime::Vec2Int&>(val)));
            return std::hash<std::int64_t> {}(fake);
        }
    };

} // namespace std
