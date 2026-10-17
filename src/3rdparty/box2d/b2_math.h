/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include <cmath>
#include <cfloat>
#include <algorithm>

namespace box2d
{
    constexpr float k_pi = 3.14159265358979323846f;

    struct Vec2
    {
        Vec2() : x(0.0f), y(0.0f) {}
        Vec2(float x_, float y_) : x(x_), y(y_) {}

        void Set(float x_, float y_) { x = x_; y = y_; }

        Vec2 operator-() const { return Vec2(-x, -y); }

        void operator+=(const Vec2 &v)
        {
            x += v.x;
            y += v.y;
        }

        void operator-=(const Vec2 &v)
        {
            x -= v.x;
            y -= v.y;
        }

        void operator*=(float a)
        {
            x *= a;
            y *= a;
        }

        float Length() const
        {
            return std::sqrt(x * x + y * y);
        }

        float x, y;
    };

    struct Mat22
    {
        Mat22() {}
        Mat22(float angle)
        {
            float c = std::cos(angle), s = std::sin(angle);
            col1.Set(c, s);
            col2.Set(-s, c);
        }

        Mat22(const Vec2 &col1_, const Vec2 &col2_) : col1(col1_), col2(col2_) {}

        Mat22 Transpose() const
        {
            return Mat22(Vec2(col1.x, col2.x), Vec2(col1.y, col2.y));
        }

        Mat22 Invert() const
        {
            float a = col1.x, b = col2.x, c = col1.y, d = col2.y;
            float det = a * d - b * c;
            det = 1.0f / det;
            return Mat22(Vec2(det * d, -det * c), Vec2(-det * b, det * a));
        }

        Vec2 col1, col2;
    };

    inline float Dot(const Vec2 &a, const Vec2 &b)
    {
        return a.x * b.x + a.y * b.y;
    }

    inline float Cross(const Vec2 &a, const Vec2 &b)
    {
        return a.x * b.y - a.y * b.x;
    }

    inline Vec2 Cross(const Vec2 &a, float s)
    {
        return Vec2(s * a.y, -s * a.x);
    }

    inline Vec2 Cross(float s, const Vec2 &a)
    {
        return Vec2(-s * a.y, s * a.x);
    }

    inline Vec2 operator*(const Mat22 &A, const Vec2 &v)
    {
        return Vec2(A.col1.x * v.x + A.col2.x * v.y, A.col1.y * v.x + A.col2.y * v.y);
    }

    inline Vec2 operator+(const Vec2 &a, const Vec2 &b)
    {
        return Vec2(a.x + b.x, a.y + b.y);
    }

    inline Vec2 operator-(const Vec2 &a, const Vec2 &b)
    {
        return Vec2(a.x - b.x, a.y - b.y);
    }

    inline Vec2 operator*(float s, const Vec2 &a)
    {
        return Vec2(s * a.x, s * a.y);
    }

    inline Vec2 operator*(const Vec2 &a, float s)
    {
        return Vec2(s * a.x, s * a.y);
    }

    inline Mat22 operator+(const Mat22 &A, const Mat22 &B)
    {
        return Mat22(A.col1 + B.col1, A.col2 + B.col2);
    }

    inline Mat22 operator*(const Mat22 &A, const Mat22 &B)
    {
        return Mat22(A * B.col1, A * B.col2);
    }

    inline float Abs(float a)
    {
        return a > 0.0f ? a : -a;
    }

    inline Vec2 Abs(const Vec2 &a)
    {
        return Vec2(Abs(a.x), Abs(a.y));
    }

    inline Mat22 Abs(const Mat22 &A)
    {
        return Mat22(Abs(A.col1), Abs(A.col2));
    }

    inline float Sign(float x)
    {
        return x < 0.0f ? -1.0f : 1.0f;
    }

    inline float Min(float a, float b)
    {
        return a < b ? a : b;
    }

    inline float Max(float a, float b)
    {
        return a > b ? a : b;
    }

    inline float Clamp(float a, float low, float high)
    {
        return Max(low, Min(a, high));
    }
}
