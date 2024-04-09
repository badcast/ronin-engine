#include "ronin.h"

namespace RoninEngine::Runtime
{

    Camera::Camera(const std::string &name) : backclear(true), backcolor(0x111111, 255), Component(DESCRIBE_AS_ONLY_NAME(Camera))
    {
        DESCRIBE_AS_MAIN(Camera);

        // using this camera as main
        World::self()->irs->cameraResources.emplace_front(RoninMemory::alloc_self(res));
        res->scale = Vec2::one;
        res->culled = 0;

        Focus();
    }
    Camera::~Camera()
    {
    }

    Vec2 Camera::GetScale()
    {
        return res->scale;
    }

    void Camera::SetScale(Vec2 scale)
    {
        res->scale = scale;
    }

    bool Camera::isFocused()
    {
        return mainCamera() == this;
    }

    void Camera::Focus()
    {
        if(switched_world)
            switched_world->irs->event_camera_changed(this, CameraEvent::CAM_TARGET);
    }

    const Vec2 Camera::ScreenToWorldPoint(Vec2 screenPoint)
    {
        Vec2 scale;

        if(switched_world->irs->mainCamera)
        {
            scale = switched_world->irs->mainCamera->res->scale;
        }
        else
        {
            scale = Vec2::one;
        }

        scale = Vec2::Scale(scale, switched_world->irs->metricPixelsPerPoint);

        screenPoint.x = -(gscope.activeResolution.width / 2.f - screenPoint.x) / scale.x;
        screenPoint.y = (gscope.activeResolution.height / 2.f - screenPoint.y) / scale.y;

        // Difference at Main Camera
        if(switched_world->irs->mainCamera)
            screenPoint += switched_world->irs->mainCamera->transform()->_position;

        return screenPoint;
    }

    const Vec2 Camera::WorldToScreenPoint(Vec2 worldPoint)
    {
        Vec2 scale;
        // Difference at Main Camera
        if(switched_world->irs->mainCamera)
        {
            worldPoint = switched_world->irs->mainCamera->transform()->_position - worldPoint;
            scale = switched_world->irs->mainCamera->res->scale;
        }
        else
        {
            worldPoint = Vec2::zero - worldPoint;
            scale = Vec2::one;
        }

        scale = Vec2::Scale(scale, switched_world->irs->metricPixelsPerPoint);

        // Horizontal position
        worldPoint.x = gscope.activeResolution.width / 2.f - worldPoint.x * scale.x;
        // Vertical position
        worldPoint.y = gscope.activeResolution.height / 2.f + worldPoint.y * scale.y;
        return worldPoint;
    }

    const Vec2 Camera::ViewportToWorldPoint(Vec2 viewportPoint)
    {
        Vec2 scale;

        if(switched_world->irs->mainCamera)
            scale = switched_world->irs->mainCamera->res->scale;
        else
            scale = Vec2::one;

        scale = Vec2::Scale(scale, switched_world->irs->metricPixelsPerPoint);

        // Horizontal position
        viewportPoint.x = (gscope.activeResolution.width / 2.f - gscope.activeResolution.width * viewportPoint.x) * -1.f / scale.x;
        // Vertical position
        viewportPoint.y = (gscope.activeResolution.height / 2.f - gscope.activeResolution.height * viewportPoint.y) / scale.y;
        // Offset of Main Camera
        if(switched_world->irs->mainCamera)
            viewportPoint += switched_world->irs->mainCamera->transform()->_position;
        return viewportPoint;
    }

    const Vec2 Camera::WorldToViewportPoint(Vec2 worldPoint)
    {
        Vec2 scale;

        // Difference at Main Camera
        if(switched_world->irs->mainCamera)
        {
            worldPoint = mainCamera()->transform()->position() - worldPoint;
            scale = switched_world->irs->mainCamera->res->scale;
        }
        else
        {
            worldPoint = Vec2::zero - worldPoint;
            scale = Vec2::one;
        }

        scale = Vec2::Scale(scale, switched_world->irs->metricPixelsPerPoint);

        // Horizontal position
        worldPoint.x = (gscope.activeResolution.width / 2.0f - worldPoint.x * scale.x) / gscope.activeResolution.width;
        // Vertical position
        worldPoint.y = (gscope.activeResolution.height / 2.0f + worldPoint.y * scale.y) / gscope.activeResolution.height;
        return worldPoint;
    }

    const Vec2 Camera::WorldToViewportPointClamp(Vec2 worldPoint)
    {
        worldPoint = WorldToViewportPoint(worldPoint);
        worldPoint.x = Math::Clamp01(worldPoint.x);
        worldPoint.y = Math::Clamp01(worldPoint.y);
        return worldPoint;
    }

    Camera *Camera::mainCamera()
    {
        if(switched_world)
            return switched_world->irs->mainCamera;
        return nullptr;
    }

} // namespace RoninEngine::Runtime
