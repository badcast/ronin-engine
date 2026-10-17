#include "ronin.h"

using namespace RoninEngine::Exception;

namespace RoninEngine::Runtime
{
    Camera2D::Camera2D() : Camera2D(DESCRIBE_AS_MAIN_OFF(Camera2D))
    {
    }

    Camera2D::Camera2D(const std::string &name) : Camera(DESCRIBE_AS_ONLY_NAME(Camera2D)), distanceEvcall(1), visibleBorders(false), visibleGrids(false), visibleObjects(false), visibleNames(false)
    {
        DESCRIBE_AS_MAIN(Camera2D);
    }

    Camera2D::Camera2D(const Camera2D &other) : Camera(other), distanceEvcall(other.distanceEvcall), visibleBorders(other.visibleBorders), visibleGrids(other.visibleGrids), visibleObjects(other.visibleObjects), visibleNames(other.visibleNames)
    {
    }

    void Camera2D::SetZoomOut(float zoomLevel)
    {
        // Compute unconstrained scale without artificial hard limiters (previously clamped to [-200, 200] and [0.03f, 10.0f])
        float scaleVal = 1.0f - ((zoomLevel - 100.0f) / 100.0f);
        scaleVal = std::max(1e-5f, scaleVal);
        if(res)
            res->scale = Vec2::one * scaleVal;
    }

    float Camera2D::GetZoomOut()
    {
        return res ? (100.0f + (1.0f - res->scale.x) * 100.0f) : 100.0f;
    }
} // namespace RoninEngine::Runtime
