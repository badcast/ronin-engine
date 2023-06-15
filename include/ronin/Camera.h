#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Camera : public Component
    {
    protected:
        struct CameraResource* camera_resources;
        virtual void render(SDL_Renderer* renderer, Rect rect, GameObject* root) = 0;

    public:
        Camera(const std::string& name);

        virtual ~Camera();

        bool is_focused();
        void focus();

        static const Vec2 screen_to_world(Vec2 screenPoint);
        static const Vec2 world_to_screen(Vec2 worldPoint);
        static const Vec2 viewport_to_world(Vec2 viewportPoint);
        static const Vec2 world_to_viewport(Vec2 worldPoint);
        static const Vec2 world_to_viewport_clamp(Vec2 worldPoint);
        static Camera* main_camera();
    };

} // namespace RoninEngine::Runtime
