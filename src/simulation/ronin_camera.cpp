#include "ronin.h"

namespace RoninEngine::Runtime
{

    Camera::Camera(const std::string &name) : Component(DESCRIBE_AS_ONLY_NAME(Camera))
    {
        DESCRIBE_AS_MAIN(Camera);

        // using this camera as main
        World::self()->internal_resources->camera_resources.emplace_front(RoninMemory::alloc_self(camera_resources));
        camera_resources->targetClear = true;

        // set focusing
        focus();
    }
    Camera::~Camera()
    {
    }

    bool Camera::is_focused()
    {
        return main_camera() == this;
    }

    void Camera::focus()
    {
        World::self()->internal_resources->event_camera_changed(this, CameraEvent::CAM_TARGET);
    }

    const Vec2 Camera::ScreenToWorldPoint(Vec2 screenPoint)
    {
        Vec2 offset;
        if(main_camera())
            offset = main_camera()->transform()->position();
        Vec2 scale;
        SDL_RenderGetScale(renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        screenPoint.x = ((active_resolution.width / 2.f - screenPoint.x) * -1) / scale.x;
        screenPoint.y = (active_resolution.height / 2.f - screenPoint.y) / scale.y;
        screenPoint += offset;
        return screenPoint;
    }
    const Vec2 Camera::WorldToScreenPoint(Vec2 worldPoint)
    {
        Vec2 scale;
        Vec2 offset;
        if(main_camera())
            offset = main_camera()->transform()->position();
        SDL_RenderGetScale(RoninEngine::renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        // Horizontal position
        worldPoint.x = (active_resolution.width / 2.f - (offset.x - worldPoint.x) * scale.x);
        // Vertical position
        worldPoint.y = (active_resolution.height / 2.f + (offset.y - worldPoint.y) * scale.y);
        return worldPoint;
    }

    const Vec2 Camera::ViewportToWorldPoint(Vec2 viewportPoint)
    {
        Vec2 scale, offset;
        if(main_camera())
            offset = main_camera()->transform()->position();
        SDL_RenderGetScale(RoninEngine::renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        // Horizontal position
        viewportPoint.x = (active_resolution.width / 2.f - active_resolution.width * viewportPoint.x) * -1 / scale.x;
        // Vertical position
        viewportPoint.y = (active_resolution.height / 2.f - active_resolution.height * viewportPoint.y) / scale.y;
        viewportPoint += offset;
        return viewportPoint;
    }

    const Vec2 Camera::WorldToViewportPoint(Vec2 worldPoint)
    {
        Vec2 scale;
        Vec2 offset;
        if(main_camera())
            offset = main_camera()->transform()->position();
        SDL_RenderGetScale(RoninEngine::renderer, &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        // Horizontal position
        worldPoint.x = (active_resolution.width / 2.0f - (offset.x - worldPoint.x) * scale.x) / active_resolution.width;
        // Vertical position
        worldPoint.y = (active_resolution.height / 2.0f + (offset.y - worldPoint.y) * scale.y) / active_resolution.height;
        return worldPoint;
    }

    const Vec2 Camera::WorldToViewportPointClamp(Vec2 worldPoint)
    {
        worldPoint = WorldToViewportPoint(worldPoint);
        worldPoint.x = Math::Clamp01(worldPoint.x);
        worldPoint.y = Math::Clamp01(worldPoint.y);
        return worldPoint;
    }
    Camera *Camera::main_camera()
    {
        return World::self()->internal_resources->main_camera;
    }

} // namespace RoninEngine::Runtime
