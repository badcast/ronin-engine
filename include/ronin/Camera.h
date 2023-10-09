#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Camera : public Component
    {
    protected:
        struct CameraResource *camera_resource;

    public:
        Camera(const std::string &name);

        virtual ~Camera();

        bool isFocused();
        void Focus();

        static const Vec2 ScreenToWorldPoint(Vec2 screenPoint);
        static const Vec2 WorldToScreenPoint(Vec2 worldPoint);
        static const Vec2 ViewportToWorldPoint(Vec2 viewportPoint);
        static const Vec2 WorldToViewportPoint(Vec2 worldPoint);
        static const Vec2 WorldToViewportPointClamp(Vec2 worldPoint);
        static Camera *mainCamera();
    };

} // namespace RoninEngine::Runtime
