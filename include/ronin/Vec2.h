#pragma once

#include "begin.h"

namespace RoninEngine
{
    namespace Runtime
    {
        struct RONIN_API Vec2Int
        {
        public:
            int x;
            int y;

            Vec2Int() = default;
            Vec2Int(int x, int y);

            Vec2Int(const Vec2Int &) = default;
            Vec2Int(Vec2Int &&) = default;

            ~Vec2Int() = default;

            Vec2Int &operator+=(const Vec2Int &rhs);

            Vec2Int &operator-=(const Vec2Int &rhs);

            Vec2Int &operator*=(int rhs);

            Vec2Int &operator/=(int rhs);

            Vec2Int &operator=(const Vec2Int &rhs);

            Vec2Int &operator=(Vec2Int &&rhs);

            Vec2Int operator-() const;

            static const Vec2Int one;
            static const Vec2Int minusOne;
            static const Vec2Int zero;
            static const Vec2Int down;
            static const Vec2Int left;
            static const Vec2Int right;
            static const Vec2Int up;

            static bool HasIntersection(const Vec2Int &point, const Rect &rect);
            static bool HasIntersection(const Rect &lhs, const Rect &rhs);
        };

        struct RONIN_API Vec2
        {
            float x;
            float y;

            Vec2();
            explicit Vec2(const Vec2Int &rhs);
            explicit Vec2(Vec2Int &&rhs);
            Vec2(float x, float y);
            Vec2(const Vec2 &) = default;
            Vec2(Vec2 &&) = default;
            ~Vec2() = default;

            float magnitude() const;
            float sqrMagnitude() const;
            void normalize();

            Vec2 normalized() const;

            Vec2 &operator+=(const Vec2 &rhs);
            Vec2 &operator-=(const Vec2 &rhs);
            Vec2 &operator*=(const float &d);
            Vec2 &operator/=(const float &d);
            Vec2 &operator*=(float &&d);
            Vec2 &operator/=(float &&d);
            Vec2 &operator=(const Vec2 &rhs);

            Vec2 operator-() const;

            static const Vec2 one;
            static const Vec2 half;
            static const Vec2 minusOne;
            static const Vec2 zero;
            static const Vec2 down;
            static const Vec2 left;
            static const Vec2 right;
            static const Vec2 up;
            static const Vec2 up_right;
            static const Vec2 up_left;
            static const Vec2 down_right;
            static const Vec2 down_left;
            static const Vec2 infinity;
            static const Vec2 negativeInfinity;

            static const Vec2 Abs(const Vec2 &value);
            static const Vec2 NAbs(const Vec2 &value);
            static Vec2 Slerp(const Vec2 &lhs, const Vec2 &rhs, float t);
            static Vec2 SlerpUnclamped(const Vec2 &lhs, const Vec2 &rhs, float t);
            static Vec2 Lerp(const Vec2 &lhs, const Vec2 &rhs, float t);
            static Vec2 LerpUnclamped(const Vec2 &lhs, const Vec2 &rhs, float t);
            static Vec2 Max(const Vec2 &lhs, const Vec2 &rhs);
            static Vec2 Min(const Vec2 &lhs, const Vec2 &rhs);
            static Vec2 MoveTowards(const Vec2 &current, const Vec2 &target, float maxDistanceDelta);
            static Vec2 Reflect(const Vec2 &inDirection, const Vec2 &inNormal);
            static Vec2 Scale(const Vec2 &lhs, const Vec2 &rhs);
            static float Distance(const Vec2 &lhs, const Vec2 &rhs);
            static float DistanceSqr(const Vec2 &lhs, const Vec2 &rhs);
            static float Angle(Vec2 from, Vec2 to);
            static float SignedAngle(Vec2 from, Vec2 to);
            static Vec2 ClampMagnitude(Vec2 vector, float maxLength);
            static Vec2 SmoothDamp(const Vec2 &current, Vec2 target, Vec2 &currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
            static float Dot(const Vec2 &lhs, const Vec2 &rhs);
            static float SqrMagnitude(const Vec2 &lhs);
            static Vec2 Round(Vec2 lhs);
            static Vec2Int RoundToInt(const Vec2 &lhs);
            static bool HasIntersection(const Vec2 &p, const Rectf &rhs);
            static bool HasIntersection(const Rectf &lhs, const Rectf &rhs);
            static const Vec2 Rotate(const Vec2 &vec, Vec2 normal, float angleRadian);
            static const Vec2 Rotate(Vec2 position, float angleRadian);
            static const Vec2 RotateClockwise(Vec2 position, float angleRadian);
            static const Vec2 RotateAround(Vec2 center, Vec2 localPosition, float angleRadian);
            static const Vec2 Mirror(const Vec2 &position);
            static const Vec2 Perpendicular(Vec2 inDirection);
            static bool LookRotation(Vec2 from, Transform *to, float angleRadian);
        };

        RONIN_API bool operator==(const Vec2 &lhs, const Vec2 &rhs);
        RONIN_API bool operator!=(const Vec2 &lhs, const Vec2 &rhs);

        RONIN_API Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs);
        RONIN_API Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs);
        RONIN_API Vec2 operator*(const float d, const Vec2 &rhs);
        RONIN_API Vec2 operator*(const Vec2 &rhs, const float d);
        RONIN_API Vec2 operator/(const Vec2 &rhs, const float d);

        RONIN_API bool operator==(const Vec2Int &lhs, const Vec2 &rhs);
        RONIN_API bool operator!=(const Vec2Int &lhs, const Vec2 &rhs);

        RONIN_API Vec2Int operator+(const Vec2Int &lhs, const Vec2Int &rhs);
        RONIN_API Vec2Int operator-(const Vec2Int &lhs, const Vec2Int &rhs);
        RONIN_API Vec2 operator+(const Vec2 &lhs, const Vec2Int &rhs);
        RONIN_API Vec2 operator-(const Vec2 &lhs, const Vec2Int &rhs);
        RONIN_API Vec2 operator+(const Vec2Int &lhs, const Vec2 &rhs);
        RONIN_API Vec2 operator-(const Vec2Int &lhs, const Vec2 &rhs);
        RONIN_API bool operator==(const Vec2Int &lhs, const Vec2Int &rhs);
        RONIN_API bool operator!=(const Vec2Int &lhs, const Vec2Int &rhs);
        RONIN_API Vec2Int operator*(const int d, const Vec2Int &rhs);
        RONIN_API Vec2Int operator*(const Vec2Int &rhs, const int d);
        RONIN_API Vec2Int operator/(const Vec2Int &rhs, const int d);
        RONIN_API Vec2 operator*(const float d, const Vec2Int &rhs);
        RONIN_API Vec2 operator*(const Vec2Int &rhs, const float d);
        RONIN_API Vec2 operator/(const Vec2Int &rhs, const float d);
    } // namespace Runtime
} // namespace RoninEngine
