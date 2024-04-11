#include "ronin.h"

namespace RoninEngine::Runtime
{

    Camera::Camera(const std::string &name) : backclear(true), backcolor(0x111111, 255), Component(DESCRIBE_AS_ONLY_NAME(Camera))
    {
        DESCRIBE_AS_MAIN(Camera);

        // using this camera as main
        World::self()->irs->camera_resources.emplace_front(RoninMemory::alloc_self(camera_resource));
        camera_resource->culled = 0;
        // set focusing
        Focus();
    }
    Camera::~Camera()
    {
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

        SDL_RenderGetScale(env.renderer, &scale.x, &scale.y);

        scale *= pixelsPerPoint;

        screenPoint.x = -(env.activeResolution.width / 2.f - screenPoint.x) / scale.x;
        screenPoint.y = (env.activeResolution.height / 2.f - screenPoint.y) / scale.y;

        // Difference at Main Camera
        if(mainCamera())
            screenPoint += mainCamera()->transform()->_position;

        return screenPoint;
    }

    const Vec2 Camera::WorldToScreenPoint(Vec2 worldPoint)
    {
        Vec2 scale;
        // Difference at Main Camera
        if(mainCamera())
            worldPoint = mainCamera()->transform()->_position - worldPoint;
        else
            worldPoint = Vec2::zero - worldPoint;

        SDL_RenderGetScale(env.renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;

        // Horizontal position
        worldPoint.x = env.activeResolution.width / 2.f - worldPoint.x * scale.x;
        // Vertical position
        worldPoint.y = env.activeResolution.height / 2.f + worldPoint.y * scale.y;
        return worldPoint;
    }

    const Vec2 Camera::ViewportToWorldPoint(Vec2 viewportPoint)
    {
        Vec2 scale;
        SDL_RenderGetScale(env.renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        // Horizontal position
        viewportPoint.x = (env.activeResolution.width / 2.f - env.activeResolution.width * viewportPoint.x) * -1.f / scale.x;
        // Vertical position
        viewportPoint.y = (env.activeResolution.height / 2.f - env.activeResolution.height * viewportPoint.y) / scale.y;
        // Difference at Main Camera
        if(mainCamera())
            viewportPoint += mainCamera()->transform()->_position;
        return viewportPoint;
    }

    const Vec2 Camera::WorldToViewportPoint(Vec2 worldPoint)
    {
        Vec2 scale;

        // Difference at Main Camera
        if(mainCamera())
            worldPoint = mainCamera()->transform()->position() - worldPoint;
        else
            worldPoint = Vec2::zero - worldPoint;

        SDL_RenderGetScale(env.renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        // Horizontal position
        worldPoint.x = (env.activeResolution.width / 2.0f - worldPoint.x * scale.x) / env.activeResolution.width;
        // Vertical position
        worldPoint.y = (env.activeResolution.height / 2.0f + worldPoint.y * scale.y) / env.activeResolution.height;
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
            return switched_world->irs->main_camera;
        return nullptr;
    }

} // namespace RoninEngine::Runtime
