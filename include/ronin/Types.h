#pragma once

#include "begin.h"

namespace RoninEngine
{

    namespace Runtime
    {
        enum Align
        {
            Left,
            Right,
            Center
        };

        enum AudioState
        {
            Undefined = -1,
            Play,
            Pause,
            Stop
        };

        template <typename T>
        class RRect
        {
        public:
            T x, y, w, h;
            template <typename V>
            RRect(const V &xy, const V &wh) : x(xy.x), y(xy.y), w(wh.x), h(wh.y)
            {
            }
            RRect()
            {
                x = y = w = h = 0;
            }
            RRect(const T &x, const T &y, const T &w, const T &h) : x(x), y(y), w(w), h(h)
            {
            }

            template <typename oth_rect>
            RRect(const oth_rect &other)
            {
                static_assert(std::is_same<T, int>::value && std::is_same<oth_rect, native_rect_t>::value, "Rect not set from SDL_FRect");
                static_assert(
                    std::is_same<T, float>::value && std::is_same<oth_rect, native_frect_t>::value, "Rectf not set from SDL_Rect");
                this->x = other.x;
                this->y = other.y;
                this->w = other.w;
                this->h = other.h;
            }

            bool empty() const
            {
                return x == 0 && y == 0 && w == 0 && h == 0;
            }

            RRect<T> operator+(const RRect<T> &rhs)
            {
                return {this->x + rhs.x, this->y + rhs.y, this->w + rhs.w, this->h + rhs.h};
            }

            RRect<T> operator-(const RRect<T> &rhs)
            {
                return {this->x - rhs.x, this->y + rhs.y, this->w - rhs.w, this->h - rhs.h};
            }

            RRect<T> operator*(const T &rhs)
            {
                return {this->x * rhs, this->y * rhs, this->w * rhs, this->h * rhs};
            }

            RRect<T> operator/(const T &rhs)
            {
                return {this->x / rhs, this->y / rhs, this->w / rhs, this->h / rhs};
            }

            RRect<T> &operator+=(const RRect<T> &rhs)
            {
                this->x += rhs.x;
                this->y += rhs.y;
                this->w += rhs.w;
                this->h += rhs.h;
                return *this;
            }

            RRect<T> &operator-=(const RRect<T> &rhs)
            {
                this->x -= rhs.x;
                this->y -= rhs.y;
                this->w -= rhs.w;
                this->h -= rhs.h;
                return *this;
            }

            constexpr auto GetXY() const
            {
                if constexpr(std::is_same<T, int>::value)
                {
                    return Vec2Int(x, y);
                }
                else if constexpr(std::is_same<T, float>::value)
                {
                    return Vec2(x, y);
                }
            }
            constexpr auto GetWH() const
            {
                if constexpr(std::is_same<T, int>::value)
                {
                    return Vec2Int(w, h);
                }
                else if constexpr(std::is_same<T, float>::value)
                {
                    return Vec2(w, h);
                }
            }

            template <typename oth_rect>
            RRect<T> &operator=(const oth_rect &rhs)
            {
                if constexpr(std::is_same<T, int>::value)
                    static_assert(std::is_same<oth_rect, native_rect_t>::value, "Rect not set from SDL_FRect, require SDL_Rect");
                else if constexpr(std::is_same<T, float>::value)
                    static_assert(std::is_same<oth_rect, native_frect_t>::value, "Rectf not set from SDL_Rect, require SDL_FRect");
                this->x = rhs.x;
                this->y = rhs.y;
                this->w = rhs.w;
                this->h = rhs.h;
                return *this;
            }

            static const RRect<T> zero;
            static const RRect<T> one;
        };

        struct IComponents
        {
            virtual Transform *transform() const = 0;
            virtual SpriteRenderer *spriteRenderer() const = 0;
            virtual Camera2D *camera2D() const = 0;
            virtual Terrain2D *terrain2D() const = 0;
            virtual Component *AddComponent(Component *component) = 0;
            virtual bool RemoveComponent(Component *component) = 0;
        };

        template <typename T>
        using RoninBaseEvent = std::function<void(T *)>;

        template <typename T>
        struct IRoninBaseEvents
        {
            void AddOnDestroy(RoninBaseEvent<T> callback);
            void ClearOnDestroy();
        };
    } // namespace Runtime

} // namespace RoninEngine
