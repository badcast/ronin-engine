#include "ronin.h"

namespace RoninEngine::Runtime
{

    Camera::Camera() : Camera(DESCRIBE_AS_MAIN_OFF(Camera))
    {
    }

    Camera::Camera(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Camera)), res(nullptr), backclear(true), backcolor(0x111111, 255)
    {
        DESCRIBE_AS_MAIN(Camera);

        RoninMemory::alloc_self(res);
        res->scale = Vec2::one;
        res->culled = 0;
        if(World *w = World::GetCurrentWorld(); w && w->irs)
        {
            w->irs->cameraResources.emplace_front(res);
        }
    }

    Camera::Camera(const Camera &other) : Component(other.m_name), res(nullptr), backclear(other.backclear), backcolor(other.backcolor)
    {
        DESCRIBE_AS_MAIN(Camera);

        RoninMemory::alloc_self(res);
        res->scale = other.res ? other.res->scale : Vec2::one;
        res->offsetScaling = other.res ? other.res->offsetScaling : Vec2::zero;
        res->culled = 0;
        if(World *w = World::GetCurrentWorld(); w && w->irs)
        {
            w->irs->cameraResources.emplace_front(res);
        }
    }

    Camera::~Camera()
    {
        if(res)
        {
            if(currentWorld && currentWorld->irs)
            {
                if(currentWorld->irs->mainCamera.ptr_ == this)
                    currentWorld->irs->mainCamera = nullptr;
                currentWorld->irs->cameraResources.remove(res);
            }
            RoninMemory::free(res);
            res = nullptr;
        }
    }

    Vec2 Camera::GetScale()
    {
        return res ? res->scale : Vec2::one;
    }

    void Camera::SetScale(Vec2 scale)
    {
        if(res)
        {
            // Safeguard against absolute zero to prevent division by zero in projection
            if(std::abs(scale.x) < 1e-6f) scale.x = (scale.x < 0.0f ? -1e-6f : 1e-6f);
            if(std::abs(scale.y) < 1e-6f) scale.y = (scale.y < 0.0f ? -1e-6f : 1e-6f);
            res->scale = scale;
        }
    }

    void Camera::SetScale(float scale)
    {
        SetScale(Vec2(scale, scale));
    }

    bool Camera::isFocused()
    {
        return mainCamera().get() == this;
    }

    void Camera::Focus()
    {
        if(currentWorld && currentWorld->irs)
            currentWorld->irs->event_camera_changed(this->GetRef<Camera>(), CameraEvent::CAM_TARGET);
    }

    Vec2 Camera::ScreenToWorldPoint(Vec2 screenPoint)
    {
        if(!currentWorld || !currentWorld->irs)
            return screenPoint;

        Vec2 scale = (currentWorld->irs->mainCamera && currentWorld->irs->mainCamera->res)
                         ? currentWorld->irs->mainCamera->res->scale
                         : Vec2::one;

        scale = Vec2::Scale(scale, currentWorld->irs->metricPixelsPerPoint);
        if(std::abs(scale.x) < 1e-6f) scale.x = (scale.x < 0.0f ? -1e-6f : 1e-6f);
        if(std::abs(scale.y) < 1e-6f) scale.y = (scale.y < 0.0f ? -1e-6f : 1e-6f);

        screenPoint.x = -(gscope.activeResolution.width / 2.f - screenPoint.x) / scale.x;
        screenPoint.y = (gscope.activeResolution.height / 2.f - screenPoint.y) / scale.y;

        // Difference at Main Camera
        if(currentWorld->irs->mainCamera && currentWorld->irs->mainCamera->transform())
            screenPoint += currentWorld->irs->mainCamera->transform()->_position;

        return screenPoint;
    }

    Vec2 Camera::WorldToScreenPoint(Vec2 worldPoint)
    {
        if(!currentWorld || !currentWorld->irs)
            return worldPoint;

        Vec2 scale;
        // Difference at Main Camera
        if(currentWorld->irs->mainCamera && currentWorld->irs->mainCamera->transform() && currentWorld->irs->mainCamera->res)
        {
            worldPoint = currentWorld->irs->mainCamera->transform()->_position - worldPoint;
            scale = currentWorld->irs->mainCamera->res->scale;
        }
        else
        {
            worldPoint = Vec2::zero - worldPoint;
            scale = Vec2::one;
        }

        scale = Vec2::Scale(scale, currentWorld->irs->metricPixelsPerPoint);

        // Horizontal position
        worldPoint.x = gscope.activeResolution.width / 2.f - worldPoint.x * scale.x;
        // Vertical position
        worldPoint.y = gscope.activeResolution.height / 2.f + worldPoint.y * scale.y;

        return worldPoint;
    }

    Vec2 Camera::ViewportToWorldPoint(Vec2 viewportPoint)
    {
        if(!currentWorld || !currentWorld->irs)
            return viewportPoint;

        Vec2 scale = (currentWorld->irs->mainCamera && currentWorld->irs->mainCamera->res)
                         ? currentWorld->irs->mainCamera->res->scale
                         : Vec2::one;

        scale = Vec2::Scale(scale, currentWorld->irs->metricPixelsPerPoint);
        if(std::abs(scale.x) < 1e-6f) scale.x = (scale.x < 0.0f ? -1e-6f : 1e-6f);
        if(std::abs(scale.y) < 1e-6f) scale.y = (scale.y < 0.0f ? -1e-6f : 1e-6f);

        // Horizontal position
        viewportPoint.x = (gscope.activeResolution.width / 2.f - gscope.activeResolution.width * viewportPoint.x) * -1.f / scale.x;
        // Vertical position
        viewportPoint.y = (gscope.activeResolution.height / 2.f - gscope.activeResolution.height * (1.0f - viewportPoint.y)) / scale.y;
        // Offset of Main Camera
        if(currentWorld->irs->mainCamera && currentWorld->irs->mainCamera->transform())
            viewportPoint += currentWorld->irs->mainCamera->transform()->_position;
        return viewportPoint;
    }

    Vec2 Camera::WorldToViewportPoint(Vec2 worldPoint)
    {
        if(!currentWorld || !currentWorld->irs)
            return worldPoint;

        Vec2 scale;

        // Difference at Main Camera
        if(currentWorld->irs->mainCamera && currentWorld->irs->mainCamera->transform() && currentWorld->irs->mainCamera->res)
        {
            worldPoint = mainCamera()->transform()->position() - worldPoint;
            scale = currentWorld->irs->mainCamera->res->scale;
        }
        else
        {
            worldPoint = Vec2::zero - worldPoint;
            scale = Vec2::one;
        }

        scale = Vec2::Scale(scale, currentWorld->irs->metricPixelsPerPoint);

        float w = gscope.activeResolution.width > 0 ? static_cast<float>(gscope.activeResolution.width) : 1.0f;
        float h = gscope.activeResolution.height > 0 ? static_cast<float>(gscope.activeResolution.height) : 1.0f;

        // Horizontal position
        worldPoint.x = (w / 2.0f - worldPoint.x * scale.x) / w;
        // Vertical position
        worldPoint.y = 1.0f - (h / 2.0f + worldPoint.y * scale.y) / h;
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
        if(gscope.activeWindow && gscope.activeResolution.width > 0 && gscope.activeResolution.height > 0)
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

    CameraRef Camera::mainCamera()
    {
        if(currentWorld && currentWorld->irs)
            return currentWorld->irs->mainCamera;
        return CameraRef{nullptr};
    }

} // namespace RoninEngine::Runtime
