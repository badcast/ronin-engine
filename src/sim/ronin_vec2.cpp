#include "ronin.h"

namespace RoninEngine::Runtime
{

    Vec2Int::Vec2Int(int _x, int _y) : x(_x), y(_y)
    {
    }

    Vec2Int &Vec2Int::operator+=(const Vec2Int &rhs)
    {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }

    Vec2Int &Vec2Int::operator-=(const Vec2Int &rhs)
    {
        this->x -= rhs.x;
        this->y -= rhs.y;
        return *this;
    }

    Vec2Int &Vec2Int::operator*=(int rhs)
    {
        this->x *= rhs;
        this->y *= rhs;
        return *this;
    }

    Vec2Int &Vec2Int::operator/=(int rhs)
    {
        this->x /= rhs;
        this->y /= rhs;
        return *this;
    }

    Vec2Int &Vec2Int::operator=(const Vec2Int &rhs)
    {
        this->x = rhs.x;
        this->y = rhs.y;
        return *this;
    }

    Vec2Int Vec2Int::operator-() const
    {
        return *this * -1;
    }

    bool Vec2Int::HasIntersection(const Vec2Int &point, const Rect &rect)
    {
        return ((point.x >= rect.x) && (point.x < (rect.x + rect.w)) && (point.y >= rect.y) && (point.y < (rect.y + rect.h)));
    }

    bool Vec2Int::HasIntersection(const Rect &lhs, const Rect &rhs)
    {
        return SDL_HasIntersection(reinterpret_cast<const SDL_Rect *>(&lhs), reinterpret_cast<const SDL_Rect *>(&rhs));
    }

    Vec2Int &Vec2Int::operator=(Vec2Int &&rhs)
    {
        this->x = rhs.x;
        this->y = rhs.y;
        return *this;
    }

    Vec2::Vec2() : x(0), y(0)
    {
    }

    Vec2::Vec2(const Vec2Int &rhs) : x(rhs.x), y(rhs.y)
    {
    }

    Vec2::Vec2(Vec2Int &&rhs) : x(rhs.x), y(rhs.y)
    {
    }

    Vec2::Vec2(float _x, float _y) : x(_x), y(_y)
    {
    }

    float Vec2::magnitude() const
    {
        return Math::Sqrt(x * x + y * y);
    }

    float Vec2::sqrMagnitude() const
    {
        return x * x + y * y;
    }

    Vec2 Vec2::normalized() const
    {
        Vec2 result = *this;
        result.normalize();
        return result;
    }

    void Vec2::normalize()
    {
        float mag = magnitude();
        if(mag > 1E-05)
        {
            *this /= mag;
        }
        else
        {
            *this = Vec2::zero;
        }
    }

    const Vec2 Vec2::Abs(const Vec2 &value)
    {
        return {Math::Abs(value.x), Math::Abs(value.y)};
    }

    const Vec2 Vec2::NAbs(const Vec2 &value)
    {
        return {Math::NAbs(value.x), Math::NAbs(value.y)};
    }

    Vec2 Vec2::Slerp(const Vec2 &lhs, const Vec2 &rhs, float t)
    {
        t = Math::Clamp01(t);

        Vec2 a_norm = lhs.normalized();
        Vec2 b_norm = rhs.normalized();

        float dot = Dot(a_norm, b_norm);
        float theta = std::acos(dot);

        if(std::abs(theta) < 0.001f)
        {
            return Lerp(lhs, rhs, t);
        }

        float sin_theta = std::sin(theta);
        float a_coeff = std::sin((1 - t) * theta) / sin_theta;
        float b_coeff = std::sin(t * theta) / sin_theta;

        return a_norm * a_coeff + b_norm * b_coeff;
    }

    Vec2 Vec2::SlerpUnclamped(const Vec2 &lhs, const Vec2 &rhs, float t)
    {
        Vec2 a_norm = lhs.normalized();
        Vec2 b_norm = rhs.normalized();

        float dot = Dot(a_norm, b_norm);
        float theta = std::acos(dot);

        float sin_theta = std::sin(theta);
        float a_coeff = std::sin((1 - t) * theta) / sin_theta;
        float b_coeff = std::sin(t * theta) / sin_theta;

        return a_norm * a_coeff + b_norm * b_coeff;
    }

    Vec2 Vec2::Lerp(const Vec2 &lhs, const Vec2 &rhs, float t)
    {
        t = Math::Clamp01(t);
        return {lhs.x + (rhs.x - lhs.x) * t, lhs.y + (rhs.y - lhs.y) * t};
    }

    Vec2 Vec2::LerpUnclamped(const Vec2 &lhs, const Vec2 &rhs, float t)
    {
        return {lhs.x + (rhs.x - lhs.x) * t, lhs.y + (rhs.y - lhs.y) * t};
    }

    Vec2 Vec2::Max(const Vec2 &lhs, const Vec2 &rhs)
    {
        return {Math::Max(lhs.x, rhs.x), Math::Max(lhs.y, rhs.y)};
    }

    Vec2 Vec2::Min(const Vec2 &lhs, const Vec2 &rhs)
    {
        return {Math::Min(lhs.x, rhs.x), Math::Min(lhs.y, rhs.y)};
    }

    Vec2 Vec2::MoveTowards(const Vec2 &current, const Vec2 &target, float maxDistanceDelta)
    {
        Vec2 a = target - current;
        float mag = a.magnitude();
        Vec2 result;
        if(mag <= maxDistanceDelta || mag == 0)
        {
            result = target;
        }
        else
        {
            result = current + a / mag * maxDistanceDelta;
        }
        return result;
    }

    Vec2 Vec2::Reflect(const Vec2 &inDirection, const Vec2 &inNormal)
    {
        return -2 * Dot(inNormal, inDirection) * inNormal + inDirection;
    }

    Vec2 Vec2::Scale(const Vec2 &lhs, const Vec2 &rhs)
    {
        return {lhs.x * rhs.x, lhs.y * rhs.y};
    }

    float Vec2::Distance(const Vec2 &lhs, const Vec2 &rhs)
    {
        float num = lhs.x - rhs.x;
        float num2 = lhs.y - rhs.y;
        return Math::Sqrt(num * num + num2 * num2);
    }

    float Vec2::DistanceSqr(const Vec2 &lhs, const Vec2 &rhs)
    {
        float num = lhs.x - rhs.x;
        float num2 = lhs.y - rhs.y;
        return num * num + num2 * num2;
    }

    float Vec2::Angle(Vec2 from, Vec2 to)
    {
        from.normalize();
        to.normalize();
        return Math::Acos(Math::Clamp(Vec2::Dot(from, to), -1.f, 1.f));
    }

    float Vec2::SignedAngle(Vec2 from, Vec2 to)
    {
        float num = Angle(from, to);
        from.normalize();
        to.normalize();
        float num2 = Math::Sign(from.x * to.y - from.y * to.x);

        return num * num2;
    }

    Vec2 Vec2::ClampMagnitude(Vec2 vector, float maxLength)
    {
        Vec2 result;
        if(vector.sqrMagnitude() > maxLength * maxLength)
        {
            result = vector.normalized() * maxLength;
        }
        else
        {
            result = vector;
        }
        return result;
    }

    Vec2 Vec2::SmoothDamp(const Vec2 &current, Vec2 target, Vec2 &currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
    {
        smoothTime = Math::Max(0.f, Math::Max(0001.f, smoothTime));
        float n0 = 2 / smoothTime;
        float n1 = n0 * deltaTime;
        float d = 1 / (1 + n1 + 0, 48 * n1 * n1 + 0, 235 * n1 * n1 * n1);
        Vec2 vector = current - target;
        Vec2 vector2 = target;
        float maxLength = maxSpeed * smoothTime;
        vector = ClampMagnitude(vector, maxLength);
        target = current - vector;
        Vec2 vector3 = (currentVelocity + n0 * vector) * deltaTime;
        currentVelocity = (currentVelocity - n0 * vector3) * d;
        Vec2 vector4 = target + (vector + vector3) * d;
        if(Vec2::Dot(vector2 - current, vector4 - vector2) > 0)
        {
            vector4 = vector2;
            currentVelocity = (vector4 - vector2) / deltaTime;
        }
        return vector4;
    }

    float Vec2::Dot(const Vec2 &lhs, const Vec2 &rhs)
    {
        return lhs.x * rhs.x + lhs.y * rhs.y;
    }

    float Vec2::SqrMagnitude(const Vec2 &lhs)
    {
        return lhs.x * lhs.x + lhs.y * lhs.y;
    }

    Vec2 Vec2::Round(Vec2 lhs)
    {
        lhs.x = (float) static_cast<int>(lhs.x);
        lhs.y = (float) static_cast<int>(lhs.y);
        return lhs;
    }

    Vec2Int Vec2::RoundToInt(const Vec2 &lhs)
    {
        return {static_cast<int>(lhs.x), static_cast<int>(lhs.y)};
    }

    bool Vec2::HasIntersection(const Vec2 &p, const Rectf &r)
    {
        return ((p.x >= r.x) && (p.x < (r.x + r.w)) && (p.y >= r.y) && (p.y < (r.y + r.h)));
    }

    bool Vec2::HasIntersection(const Rectf &lhs, const Rectf &rhs)
    {
        return SDL_HasIntersectionF(reinterpret_cast<const SDL_FRect *>(&lhs), reinterpret_cast<const SDL_FRect *>(&rhs));
    }

    const Vec2 Vec2::Rotate(const Vec2 &vec, Vec2 normal, float angleRadian)
    {
        normal = Vec2::RotateClockwise(normal, angleRadian);
        normal.x *= vec.x;
        normal.y *= vec.y;
        return normal;
    }

    const Vec2 Vec2::Rotate(Vec2 position, float angleRadian)
    {
        float Sin = Math::Sin(angleRadian);
        float Cos = Math::Cos(angleRadian);
        position.x = position.x * Cos - position.y * Sin;
        position.y = position.x * Sin + position.y * Cos;
        return position;
    }

    const Vec2 Vec2::RotateAround(Vec2 center, Vec2 localPosition, float angleRadian)
    {
        float Sin = Math::Sin(angleRadian);
        float Cos = Math::Cos(angleRadian);
        center.x = localPosition.x * Cos - localPosition.y * Sin + center.x;
        center.y = localPosition.x * Sin + localPosition.y * Cos + center.y;
        return center;
    }

    const Vec2 Vec2::RotateClockwise(Vec2 position, float angleRadian)
    {
        float Cos = Math::Cos(angleRadian);
        float Sin = Math::Sin(angleRadian);
        position.x = position.x * Cos + position.y * Sin;
        position.y = -position.x * Sin + position.y * Cos;
        return position;
    }

    const Vec2 Vec2::Mirror(const Vec2 &position)
    {
        return Vec2::Rotate(position, Math::Pi);
    }

    const Vec2 Vec2::Perpendicular(Vec2 inDirection)
    {
        return {0.f - inDirection.y, inDirection.x};
    }

    bool Vec2::LookRotation(Vec2 from, Transform *to, float angleRadian)
    {
        float angle = Vec2::Angle(to->_position - from, to->_position - to->forward());
        return angle <= angleRadian;
    }

    Vec2 &Vec2::operator+=(const Vec2 &rhs)
    {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }
    Vec2 &Vec2::operator-=(const Vec2 &rhs)
    {
        this->x -= rhs.x;
        this->y -= rhs.y;
        return *this;
    }

    Vec2 &Vec2::operator*=(const float &d)
    {
        this->x *= d;
        this->y *= d;
        return *this;
    }

    Vec2 &Vec2::operator/=(const float &d)
    {
        this->x /= d;
        this->y /= d;
        return *this;
    }

    Vec2 &Vec2::operator*=(float &&d)
    {
        this->x *= d;
        this->y *= d;
        return *this;
    }

    Vec2 &Vec2::operator/=(float &&d)
    {
        this->x /= d;
        this->y /= d;
        return *this;
    }

    Vec2 &Vec2::operator=(const Vec2 &rhs)
    {
        this->x = rhs.x;
        this->y = rhs.y;
        return *this;
    }

    Vec2 Vec2::operator-() const
    {
        return *this * -1.f;
    }

    /*Vec2 operators*/

    RONIN_API bool operator==(const Vec2Int &lhs, const Vec2 &rhs)
    {
        // return rhs.x == lhs.x && rhs.y == lhs.y;
        return Vec2 {rhs} == rhs;
    }

    RONIN_API bool operator!=(const Vec2Int &lhs, const Vec2 &rhs)
    {
        return !(lhs == rhs);
    }

    RONIN_API Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs)
    {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    RONIN_API Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs)
    {
        return {lhs.x - rhs.x, lhs.y - rhs.y};
    }

    RONIN_API bool operator==(const Vec2 &lhs, const Vec2 &rhs)
    {
        return (lhs - rhs).sqrMagnitude() < Math::epsilon;
    }

    RONIN_API bool operator!=(const Vec2 &lhs, const Vec2 &rhs)
    {
        return !(lhs == rhs);
    }

    RONIN_API Vec2 operator*(const float d, const Vec2 &rhs)
    {
        return {d * rhs.x, d * rhs.y};
    }

    RONIN_API Vec2 operator*(const Vec2 &rhs, const float d)
    {
        return {rhs.x * d, rhs.y * d};
    }

    RONIN_API Vec2 operator/(const Vec2 &rhs, const float d)
    {
        return {rhs.x / d, rhs.y / d};
    }

    /*Vec2Int operators*/

    RONIN_API bool operator==(const Vec2Int &lhs, const Vec2Int &rhs)
    {
        return memcmp(&lhs, &rhs, sizeof rhs) == 0;
    }

    RONIN_API bool operator!=(const Vec2Int &lhs, const Vec2Int &rhs)
    {
        return memcmp(&lhs, &rhs, sizeof rhs) != 0;
    }

    RONIN_API Vec2Int operator+(const Vec2Int &lhs, const Vec2Int &rhs)
    {
        return Vec2Int {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    RONIN_API Vec2Int operator-(const Vec2Int &lhs, const Vec2Int &rhs)
    {
        return Vec2Int {lhs.x - rhs.x, lhs.y - rhs.y};
    }

    RONIN_API Vec2 operator+(const Vec2 &lhs, const Vec2Int &rhs)
    {
        return Vec2 {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    RONIN_API Vec2 operator-(const Vec2 &lhs, const Vec2Int &rhs)
    {
        return Vec2 {lhs.x - rhs.x, lhs.y - rhs.y};
    }

    RONIN_API Vec2 operator+(const Vec2Int &lhs, const Vec2 &rhs)
    {
        return Vec2 {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    RONIN_API Vec2 operator-(const Vec2Int &lhs, const Vec2 &rhs)
    {
        return Vec2 {lhs.x - rhs.x, lhs.y - rhs.y};
    }

    RONIN_API Vec2Int operator*(const int d, const Vec2Int &rhs)
    {
        return Vec2Int {d * rhs.x, d * rhs.y};
    }

    RONIN_API Vec2Int operator*(const Vec2Int &rhs, const int d)
    {
        return Vec2Int {rhs.x * d, rhs.y * d};
    }

    RONIN_API Vec2Int operator/(const Vec2Int &rhs, const int d)
    {
        return Vec2Int {rhs.x / d, rhs.y / d};
    }

    RONIN_API Vec2 operator*(const float d, const Vec2Int &rhs)
    {
        return Vec2 {rhs.x * d, rhs.y * d};
    }

    RONIN_API Vec2 operator*(const Vec2Int &rhs, const float d)
    {
        return Vec2 {std::move(rhs)} * d;
    }

    RONIN_API Vec2 operator/(const Vec2Int &rhs, const float d)
    {
        return Vec2 {std::move(rhs)} / d;
    }

    /*Vector 2 definition*/

    const Vec2 Vec2::one(1.f, 1.f);
    const Vec2 Vec2::half(0.5f, 0.5f);
    const Vec2 Vec2::minusOne(-1, -1);
    const Vec2 Vec2::zero(0.f, 0.f);
    const Vec2 Vec2::down(0.f, -1.f);
    const Vec2 Vec2::left(-1.f, 0.f);
    const Vec2 Vec2::right(1.f, 0.f);
    const Vec2 Vec2::up(0.f, 1.f);
    const Vec2 Vec2::up_right(1.f, 1.f);
    const Vec2 Vec2::up_left(-1.f, 1.f);
    const Vec2 Vec2::down_right(1.f, -1.f);
    const Vec2 Vec2::down_left(-1.f, -1.f);
    const Vec2 Vec2::infinity(Math::infinity, Math::infinity);
    const Vec2 Vec2::negativeInfinity(Math::negativeInfinity, Math::negativeInfinity);

    /*Vector 2 Integer definition*/

    const Vec2Int Vec2Int::one(1, 1);
    const Vec2Int Vec2Int::minusOne(-1, -1);
    const Vec2Int Vec2Int::zero(0, 0);
    const Vec2Int Vec2Int::down(0, -1);
    const Vec2Int Vec2Int::left(-1, 0);
    const Vec2Int Vec2Int::right(1, 0);
    const Vec2Int Vec2Int::up(0, 1);

} // namespace RoninEngine::Runtime
