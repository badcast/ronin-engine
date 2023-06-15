#include "ronin.h"

namespace RoninEngine::Runtime
{

    Camera::Camera(const std::string& name)
        : Component(DESCRIBE_AS_ONLY_NAME(Camera))
    {
        DESCRIBE_AS_MAIN(Camera);

        // using this camera as main
        RoninMemory::alloc_self(camera_resources);
        camera_resources->targetClear = true;

        // set focusing
        focus();
    }
    Camera::~Camera() { }

    bool Camera::is_focused() { return main_camera() == this; }

    void Camera::focus() { World::self()->internal_resources->event_camera_changed(this, CameraEvent::CAM_TARGET); }

    const Vec2 Camera::screen_to_world(Vec2 screenPoint)
    {
        Resolution res = Application::get_resolution();
        Vec2 offset;
        if (main_camera())
            offset = main_camera()->transform()->position();
        Vec2 scale;
        SDL_RenderGetScale(Application::get_renderer(), &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        screenPoint.x = ((res.width / 2.f - screenPoint.x) * -1) / scale.x;
        screenPoint.y = (res.height / 2.f - screenPoint.y) / scale.y;
        screenPoint += offset;
        return screenPoint;
    }
    const Vec2 Camera::world_to_screen(Vec2 worldPoint)
    {
        Resolution res = Application::get_resolution();
        Vec2 scale;
        Vec2 offset;
        if (main_camera())
            offset = main_camera()->transform()->position();
        SDL_RenderGetScale(Application::get_renderer(), &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        // Horizontal position
        worldPoint.x = (res.width / 2.f - (offset.x - worldPoint.x) * scale.x);
        // Vertical position
        worldPoint.y = (res.height / 2.f + (offset.y - worldPoint.y) * scale.y);
        return worldPoint;
    }

    const Vec2 Camera::viewport_to_world(Vec2 viewportPoint)
    {
        Resolution res = Application::get_resolution();
        Vec2 scale, offset;
        if (main_camera())
            offset = main_camera()->transform()->position();
        SDL_RenderGetScale(Application::get_renderer(), &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        // Horizontal position
        viewportPoint.x = (res.width / 2.f - res.width * viewportPoint.x) * -1 / scale.x;
        // Vertical position
        viewportPoint.y = (res.height / 2.f - res.height * viewportPoint.y) / scale.y;
        viewportPoint += offset;
        return viewportPoint;
    }

    const Vec2 Camera::world_to_viewport(Vec2 worldPoint)
    {
        Resolution res = Application::get_resolution();
        Vec2 scale;
        Vec2 offset;
        if (main_camera())
            offset = main_camera()->transform()->position();
        SDL_RenderGetScale(Application::get_renderer(), &scale.x, &scale.y);
        scale *= pixelsPerPoint;
        // Horizontal position
        worldPoint.x = (res.width / 2.0f - (offset.x - worldPoint.x) * scale.x) / res.width;
        // Vertical position
        worldPoint.y = (res.height / 2.0f + (offset.y - worldPoint.y) * scale.y) / res.height;
        return worldPoint;
    }

    const Vec2 Camera::world_to_viewport_clamp(Vec2 worldPoint)
    {
        worldPoint = world_to_viewport(worldPoint);
        worldPoint.x = Math::clamp01(worldPoint.x);
        worldPoint.y = Math::clamp01(worldPoint.y);
        return worldPoint;
    }
    Camera* Camera::main_camera() { return World::self()->internal_resources->main_camera; }

} // namespace RoninEngine::Runtime
