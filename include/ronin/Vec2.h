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

            RONIN_API static const Vec2Int one;
            RONIN_API static const Vec2Int minusOne;
            RONIN_API static const Vec2Int zero;
            RONIN_API static const Vec2Int down;
            RONIN_API static const Vec2Int left;
            RONIN_API static const Vec2Int right;
            RONIN_API static const Vec2Int up;

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
            Vec2(const float x, const float y);
            // extern Vec2(float && x, float && y);
            Vec2(const Vec2&) = default;
            ~Vec2() = default;

            float magnitude() const;
            float sqr_magnitude() const;
            Vec2 normalized() const;
            void normalize();

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

            static const Vec2 abs(const Vec2& value);
            static const Vec2 nabs(const Vec2& value);
            static Vec2 slerp(Vec2 a, Vec2 b, float t);
            static Vec2 slerp_unclamped(Vec2 a, Vec2 b, float t);
            static Vec2 lerp(const Vec2& a, const Vec2& b, float t);
            static Vec2 lerp_unclamped(const Vec2& a, const Vec2& b, float t);
            static Vec2 max(const Vec2& lhs, const Vec2& rhs);
            static Vec2 min(const Vec2& lhs, const Vec2& rhs);
            static Vec2 move_towards(const Vec2& current, const Vec2& target, float maxDistanceDelta);
            static Vec2 reflect(const Vec2& inDirection, const Vec2& inNormal);
            static Vec2 scale(const Vec2& a, const Vec2& b);
            static float distance(const Vec2& lhs, const Vec2& rhs);
            static float distance_sqr(const Vec2& lhs, const Vec2& rhs);
            static float angle(Vec2 from, Vec2 to);
            static float signed_angle(Vec2 from, Vec2 to);
            static Vec2 clamp_magnitude(Vec2 vector, float maxLength);
            static Vec2 smooth_damp(Vec2 current, Vec2 target, Vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
            static float dot(const Vec2& lhs, const Vec2& rhs);
            static float sqr_magnitude(const Vec2& lhs);
            static Vec2 round(Vec2 lhs);
            static Vec2Int round_to_int(const Vec2& lhs);
            static bool has_intersection(const Vec2& p, const Rectf& rhs);
            static bool has_intersection(const Rectf& lhs, const Rectf& rhs);
            static bool in_area(const Vec2& p, const Rectf& rhs);
            static const Vec2 rotate(Vec2 vec, Vec2 normal, float angleRadian);
            static const Vec2 rotate(Vec2 position, float angleRadian);
            static const Vec2 rotate_clockwise(Vec2 position, float angleRadian);
            static const Vec2 rotate_around(Vec2 center, Vec2 localPosition, float angleRadian);

            static const Vec2 perpendicular(Vec2 inDirection);

            Vec2& operator+=(const Vec2& rhs);
            Vec2& operator-=(const Vec2& rhs);
            Vec2& operator*=(const float& d);
            Vec2& operator/=(const float& d);
            Vec2& operator=(const Vec2& rhs);

            Vec2 operator-() const;
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

        API_EXPORT inline const Vec2Int Vec2Int::one(1, 1);
        API_EXPORT inline const Vec2Int Vec2Int::minusOne(-1, -1);
        API_EXPORT inline const Vec2Int Vec2Int::zero(0, 0);
        API_EXPORT inline const Vec2Int Vec2Int::down(0, -1);
        API_EXPORT inline const Vec2Int Vec2Int::left(-1, 0);
        API_EXPORT inline const Vec2Int Vec2Int::right(1, 0);
        API_EXPORT inline const Vec2Int Vec2Int::up(0, 1);

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
