#include "ronin.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    Camera2D::Camera2D() : Camera2D(DESCRIBE_AS_MAIN_OFF(Camera2D))
    {
    }

    Camera2D::Camera2D(const std::string &name) : Camera(DESCRIBE_AS_ONLY_NAME(Camera2D)), visibleBorders(false), visibleGrids(false), visibleObjects(false)
    {
        DESCRIBE_AS_MAIN(Camera2D);
        distanceEvcall = 1;
    }

    Camera2D::Camera2D(const Camera2D &other) : Camera(other.m_name), visibleBorders(other.visibleBorders), visibleGrids(other.visibleGrids), visibleObjects(other.visibleObjects)
    {
    }

    void Camera2D::SetZoomOut(float zoomLevel)
    {
        zoomLevel = Math::Clamp<float>(zoomLevel - 100, -200, 200);
        zoomLevel = Math::Clamp<float>(1.0f - (zoomLevel / 100.0f), 0.03f, 10);
        res->scale = Vec2::one * zoomLevel;
    }

    float Camera2D::GetZoomOut()
    {
        return 100 + (1.0f - res->scale.x) * 100;
    }
} // namespace RoninEngine::Runtime
