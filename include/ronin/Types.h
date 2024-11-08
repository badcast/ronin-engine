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
        class TRect
        {
        public:
            T x, y, w, h;

            template <typename V>
            TRect(const V &xy, const V &wh) : x(xy.x), y(xy.y), w(wh.x), h(wh.y)
            {
            }
            TRect()
            {
                x = y = w = h = 0;
            }
            TRect(const T &x, const T &y, const T &w, const T &h) : x(x), y(y), w(w), h(h)
            {
            }

            template <typename oth_rect>
            TRect(const oth_rect &other)
            {
                static_assert(std::is_same<T, int>::value && std::is_same<oth_rect, native_rect_t>::value, "Rect not set from SDL_FRect");
                static_assert(std::is_same<T, float>::value && std::is_same<oth_rect, native_frect_t>::value, "Rectf not set from SDL_Rect");
                this->x = other.x;
                this->y = other.y;
                this->w = other.w;
                this->h = other.h;
            }

            bool empty() const
            {
                return x == 0 && y == 0 && w == 0 && h == 0;
            }

            TRect<T> operator+(const TRect<T> &rhs)
            {
                return {this->x + rhs.x, this->y + rhs.y, this->w + rhs.w, this->h + rhs.h};
            }

            TRect<T> operator-(const TRect<T> &rhs)
            {
                return {this->x - rhs.x, this->y + rhs.y, this->w - rhs.w, this->h - rhs.h};
            }

            TRect<T> operator*(const T &rhs)
            {
                return {this->x * rhs, this->y * rhs, this->w * rhs, this->h * rhs};
            }

            TRect<T> operator/(const T &rhs)
            {
                return {this->x / rhs, this->y / rhs, this->w / rhs, this->h / rhs};
            }

            TRect<T> &operator+=(const TRect<T> &rhs)
            {
                this->x += rhs.x;
                this->y += rhs.y;
                this->w += rhs.w;
                this->h += rhs.h;
                return *this;
            }

            TRect<T> &operator-=(const TRect<T> &rhs)
            {
                this->x -= rhs.x;
                this->y -= rhs.y;
                this->w -= rhs.w;
                this->h -= rhs.h;
                return *this;
            }

            template <typename _Value>
            constexpr void setXY(const _Value &value)
            {
                this->x = static_cast<T>(value.x);
                this->y = static_cast<T>(value.y);
            }

            template <typename _Value>
            constexpr void setWH(const _Value &value)
            {
                this->w = static_cast<T>(value.x);
                this->h = static_cast<T>(value.y);
            }

            constexpr auto getXY() const
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
            constexpr auto getWH() const
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

            constexpr auto getFirst() const
            {
                return getXY();
            }

            constexpr auto getSecond() const
            {
                return getWH();
            }

            template <typename oth_rect>
            TRect<T> &operator=(const oth_rect &rhs)
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

            static const TRect<T> zero;
            static const TRect<T> one;
        };

        struct IComponents
        {
            virtual TransformRef transform() = 0;
            virtual SpriteRendererRef spriteRenderer() = 0;
            virtual Camera2DRef camera2D() = 0;
            virtual Terrain2DRef terrain2D() = 0;
            virtual ComponentRef AddComponent(ComponentRef component) = 0;
            virtual bool RemoveComponent(ComponentRef component) = 0;
        };

        template <typename T>
        using RoninBaseEvent = std::function<void(Ref<T>)>;

        template <typename T>
        struct IRoninBaseEvents
        {
            void AddOnDestroy(RoninBaseEvent<T> callback);
            void ClearOnDestroy();
        };
    } // namespace Runtime

} // namespace RoninEngine
