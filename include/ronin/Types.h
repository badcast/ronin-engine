#pragma once

#include "begin.h"

namespace RoninEngine
{

    namespace Runtime
    {
        enum Align { Left, Right, Center };

        template <typename T>
        class RRect
        {
        public:
            T x, y, w, h;
            template <typename V>
            RRect(const V& xy, const V& wh)
                : x(xy.x)
                , y(xy.y)
                , w(wh.x)
                , h(wh.y)
            {
            }
            RRect() { x = y = w = h = 0; }
            RRect(const T& x, const T& y, const T& w, const T& h)
                : x(x)
                , y(y)
                , w(w)
                , h(h)
            {
            }

            bool empty() const { return x == 0 && y == 0 && w == 0 && h == 0; }

            RRect<T> operator+(const RRect<T>& rhs) { return { this->x + rhs.x, this->y + rhs.y, this->w + rhs.w, this->h + rhs.h }; }

            RRect<T> operator-(const RRect<T>& rhs) { return { this->x - rhs.x, this->y + rhs.y, this->w - rhs.w, this->h - rhs.h }; }

            RRect<T> operator*(const T& rhs) { return { this->x * rhs, this->y * rhs, this->w * rhs, this->h * rhs }; }

            RRect<T> operator/(const T& rhs) { return { this->x / rhs, this->y / rhs, this->w / rhs, this->h / rhs }; }

            RRect<T>& operator+=(const RRect<T>& rhs)
            {
                this->x += rhs.x;
                this->y += rhs.y;
                this->w += rhs.w;
                this->h += rhs.h;
                return *this;
            }

            RRect<T>& operator-=(const RRect<T>& rhs)
            {
                this->x -= rhs.x;
                this->y -= rhs.y;
                this->w -= rhs.w;
                this->h -= rhs.h;
                return *this;
            }

            RRect<T>& operator*=(const T& rhs)
            {
                this->x *= rhs;
                this->y *= rhs;
                return *this;
            }

            RRect<T>& operator/=(const T& rhs)
            {
                this->x /= rhs;
                this->y /= rhs;
                return *this;
            }

            constexpr auto getXY() const
            {
                if constexpr (std::is_same<T, int>::value) {
                    return Vec2Int(x, y);
                } else if constexpr (std::is_same<T, float>::value) {
                    return Vec2(x, y);
                }
            }
            constexpr auto getWH() const
            {
                if constexpr (std::is_same<T, int>::value) {
                    return Vec2Int(w, h);
                } else if constexpr (std::is_same<T, float>::value) {
                    return Vec2(w, h);
                }
            }
            static const RRect<T> zero;
            static const RRect<T> one;
        };

        struct IComponents {
            virtual Transform* transform() = 0;
            virtual SpriteRenderer* get_sprite_renderer() = 0;
            virtual Camera2D* get_camera2D() = 0;
            virtual Terrain2D* get_terrain2D() = 0;
        };
    } // namespace Runtime

} // namespace RoninEngine
