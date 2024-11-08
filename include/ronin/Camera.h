#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Camera : public Component
    {
    protected:
        struct CameraResource *res;

    public:
        bool backclear;
        Color backcolor;

        Camera(const std::string &name);

        virtual ~Camera();

        bool isFocused();
        void Focus();

        Vec2 GetScale();
        void SetScale(Vec2 scale);
        /**
         * @brief Convert Screen Pixels Point to World Point.
         *
         * This function converts screen pixel coordinates to world coordinates.
         *
         * @param screenPoint The screen pixel coordinates to convert.
         * @return The world coordinates corresponding to the screen pixel coordinates.
         */
        static Vec2 ScreenToWorldPoint(Vec2 screenPoint);

        /**
         * @brief Convert World Point to Screen pixels.
         *
         * This function converts world coordinates to screen pixel coordinates.
         *
         * @param worldPoint The world coordinates to convert.
         * @return The screen pixel coordinates corresponding to the world coordinates.
         */
        static Vec2 WorldToScreenPoint(Vec2 worldPoint);

        /**
         * @brief Convert Viewport to World Point.
         *
         * This function converts viewport coordinates to world coordinates.
         *
         * @param viewportPoint The viewport coordinates to convert.
         * @return The world coordinates converted from viewport coordinates.
         */
        static Vec2 ViewportToWorldPoint(Vec2 viewportPoint);

        /**
         * @brief Convert World Point to Viewport.
         *
         * This function converts world coordinates to viewport coordinates.
         *
         * @param worldPoint The world coordinates to convert.
         * @return The viewport coordinates corresponding to the world coordinates.
         */
        static Vec2 WorldToViewportPoint(Vec2 worldPoint);

        /**
         * @brief Convert World Point to Viewport with Clamping.
         *
         * This function converts world coordinates to viewport coordinates with clamping.
         *
         * @param worldPoint The world coordinates to convert.
         * @return The clamped viewport coordinates corresponding to the world coordinates.
         */
        static Vec2 WorldToViewportPointClamp(Vec2 worldPoint);
        /**
         * @brief Convert Viewport Point to Screen Pixels.
         *
         * This function converts viewport coordinates to screen pixel coordinates.
         *
         * @param viewportPoint The viewport coordinates to convert.
         * @return The screen pixel coordinates corresponding to the viewport coordinates.
         */
        static Vec2 ViewportToScreenPoint(Vec2 viewportPoint);

        /**
         * @brief Convert Screen Pixels to Viewport Point.
         *
         * This function converts screen pixel coordinates to viewport coordinates.
         *
         * @param screenPoint The screen pixel coordinates to convert.
         * @return The viewport coordinates corresponding to the screen pixel coordinates.
         */
        static Vec2 ScreenToViewportPoint(Vec2 screenPoint);

        /**
         * @brief Convert Screen Pixels to Viewport Point clamped.
         *
         * This function converts screen pixel coordinates to viewport coordinates.
         *
         * @param screenPoint The screen pixel coordinates to convert.
         * @return The viewport coordinates corresponding to the screen pixel coordinates with clamped.
         */
        static Vec2 ScreenToViewportPointClamp(Vec2 screenPoint);

        static CameraRef mainCamera();
    };

} // namespace RoninEngine::Runtime
