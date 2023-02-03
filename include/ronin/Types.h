#pragma once

#include "begin.h"

namespace RoninEngine
{

    // This is control (GUI, UI) identifier type
    typedef std::uint8_t uid;

    namespace Runtime
    {
        enum Align { Left, Right, Center };

        template <typename T>
        class SHARK xRect
        {
        public:
            T x, y, w, h;

            xRect() { x = y = w = h = 0; }
            xRect(const T& x, const T& y, const T& w, const T& h)
            {
                this->x = x;
                this->y = y;
                this->w = w;
                this->h = h;
            }

            bool empty() const { return x == 0 && y == 0 && w == 0 && h == 0; }

            xRect<T> operator+(const xRect<T>& rhs) { return { this->x + rhs.x, this->y + rhs.y, this->w + rhs.w, this->h + rhs.h }; }

            xRect<T> operator-(const xRect<T>& rhs) { return { this->x - rhs.x, this->y + rhs.y, this->w - rhs.w, this->h - rhs.h }; }

            xRect<T> operator*(const T& rhs) { return { this->x * rhs, this->y * rhs, this->w * rhs, this->h * rhs }; }

            xRect<T> operator/(const T& rhs) { return { this->x / rhs, this->y / rhs, this->w / rhs, this->h / rhs }; }

            xRect<T>& operator+=(const xRect<T>& rhs)
            {
                this->x += rhs.x;
                this->y += rhs.y;
                this->w += rhs.w;
                this->h += rhs.h;
                return *this;
            }

            xRect<T>& operator-=(const xRect<T>& rhs)
            {
                this->x -= rhs.x;
                this->y -= rhs.y;
                this->w -= rhs.w;
                this->h -= rhs.h;
                return *this;
            }

            xRect<T>& operator*=(const T& rhs)
            {
                this->x *= rhs;
                this->y *= rhs;
                return *this;
            }

            xRect<T>& operator/=(const T& rhs)
            {
                this->x /= rhs;
                this->y /= rhs;
                return *this;
            }

            Vec2 getXY() const;

            Vec2 getWH() const;

            static const xRect<T> zero;
            static const xRect<T> one;
        };

        double Get_Angle(Vec2 lhs, Vec2 rhs);
    } // namespace Runtime
} // namespace RoninEngine
