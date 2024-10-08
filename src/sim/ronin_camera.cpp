#include "ronin.h"

namespace RoninEngine::Runtime
{

    Camera::Camera(const std::string &name) : backclear(true), backcolor(0x111111, 255), Component(DESCRIBE_AS_ONLY_NAME(Camera))
    {
        DESCRIBE_AS_MAIN(Camera);

        // using this camera as main
        World::GetCurrentWorld()->irs->cameraResources.emplace_front(RoninMemory::alloc_self(res));
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
        if(_world)
            _world->irs->event_camera_changed(this, CameraEvent::CAM_TARGET);
    }

    Vec2 Camera::ScreenToWorldPoint(Vec2 screenPoint)
    {
        Vec2 scale;

        if(_world->irs->mainCamera)
        {
            scale = _world->irs->mainCamera->res->scale;
        }
        else
        {
            scale = Vec2::one;
        }

        scale = Vec2::Scale(scale, _world->irs->metricPixelsPerPoint);

        screenPoint.x = -(gscope.activeResolution.width / 2.f - screenPoint.x) / scale.x;
        screenPoint.y = (gscope.activeResolution.height / 2.f - screenPoint.y) / scale.y;

        // Difference at Main Camera
        if(_world->irs->mainCamera)
            screenPoint += _world->irs->mainCamera->transform()->_position;

        return screenPoint;
    }

    Vec2 Camera::WorldToScreenPoint(Vec2 worldPoint)
    {
        Vec2 scale;
        // Difference at Main Camera
        if(_world->irs->mainCamera)
        {
            worldPoint = _world->irs->mainCamera->transform()->_position - worldPoint;
            scale = _world->irs->mainCamera->res->scale;
        }
        else
        {
            worldPoint = Vec2::zero - worldPoint;
            scale = Vec2::one;
        }

        scale = Vec2::Scale(scale, _world->irs->metricPixelsPerPoint);

        // Horizontal position
        worldPoint.x = gscope.activeResolution.width / 2.f - worldPoint.x * scale.x;
        // Vertical position
        worldPoint.y = gscope.activeResolution.height / 2.f + worldPoint.y * scale.y;

        return worldPoint;
    }

    Vec2 Camera::ViewportToWorldPoint(Vec2 viewportPoint)
    {
        Vec2 scale;

        if(_world->irs->mainCamera)
            scale = _world->irs->mainCamera->res->scale;
        else
            scale = Vec2::one;

        scale = Vec2::Scale(scale, _world->irs->metricPixelsPerPoint);

        // Horizontal position
        viewportPoint.x = (gscope.activeResolution.width / 2.f - gscope.activeResolution.width * viewportPoint.x) * -1.f / scale.x;
        // Vertical position
        viewportPoint.y = (gscope.activeResolution.height / 2.f - gscope.activeResolution.height * (1.0f - viewportPoint.y)) / scale.y;
        // Offset of Main Camera
        if(_world->irs->mainCamera)
            viewportPoint += _world->irs->mainCamera->transform()->_position;
        return viewportPoint;
    }

    Vec2 Camera::WorldToViewportPoint(Vec2 worldPoint)
    {
        Vec2 scale;

        // Difference at Main Camera
        if(_world->irs->mainCamera)
        {
            worldPoint = mainCamera()->transform()->position() - worldPoint;
            scale = _world->irs->mainCamera->res->scale;
        }
        else
        {
            worldPoint = Vec2::zero - worldPoint;
            scale = Vec2::one;
        }

        scale = Vec2::Scale(scale, _world->irs->metricPixelsPerPoint);

        // Horizontal position
        worldPoint.x = (gscope.activeResolution.width / 2.0f - worldPoint.x * scale.x) / gscope.activeResolution.width;
        // Vertical position
        worldPoint.y = 1.0f - (gscope.activeResolution.height / 2.0f + worldPoint.y * scale.y) / gscope.activeResolution.height;
        return worldPoint;
    }

    Vec2 Camera::ViewportToScreenPoint(Vec2 viewportPoint)
    {
        Vec2 scr;
        if(gscope.activeWindow)
        {
            scr.x = gscope.activeResolution.width;
            scr.y = gscope.activeResolution.height;
        }
        else
        {
            scr.x = 1.0f;
            scr.y = 1.0f;
        }

        viewportPoint = Vec2::Scale(viewportPoint, scr);

        return viewportPoint;
    }

    Vec2 Camera::ScreenToViewportPoint(Vec2 screenPoint)
    {
        Vec2 scr;
        if(gscope.activeWindow)
        {
            scr.x = gscope.activeResolution.width;
            scr.y = gscope.activeResolution.height;
        }
        else
        {
            scr.x = 1.0f;
            scr.y = 1.0f;
        }

        screenPoint.x /= scr.x;
        screenPoint.y /= scr.y;

        return screenPoint;
    }

    Vec2 Camera::ScreenToViewportPointClamp(Vec2 screenPoint)
    {
        screenPoint = ScreenToViewportPoint(screenPoint);
        screenPoint.x = Math::Clamp01(screenPoint.x);
        screenPoint.y = Math::Clamp01(screenPoint.y);
        return screenPoint;
    }

    Vec2 Camera::WorldToViewportPointClamp(Vec2 worldPoint)
    {
        worldPoint = WorldToViewportPoint(worldPoint);
        worldPoint.x = Math::Clamp01(worldPoint.x);
        worldPoint.y = Math::Clamp01(worldPoint.y);
        return worldPoint;
    }

    Camera *Camera::mainCamera()
    {
        if(_world)
            return _world->irs->mainCamera;
        return nullptr;
    }

} // namespace RoninEngine::Runtime
