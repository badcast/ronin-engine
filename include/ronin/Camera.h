#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Camera : public Component
    {
    private:
        bool targetClear;
        std::map<int, std::set<Renderer*>> renders;
        std::set<Renderer*> prev;
        std::set<Light*> _lightsOutResults;
        virtual void render(SDL_Renderer* renderer, Rect rect, GameObject* root) = 0;

    public:
        bool enabled;
        std::uint32_t distanceEvcall;

        Camera();
        Camera(const std::string& name);

        virtual ~Camera();

        bool is_focused();
        void focus();

        std::tuple<std::map<int, std::set<Renderer*>>*, std::set<Light*>*> matrix_select();

        static const Vec2 screen_to_world(Vec2 screenPoint);
        static const Vec2 world_to_screen(Vec2 worldPoint);
        static const Vec2 viewport_to_world(Vec2 viewportPoint);
        static const Vec2 world_to_viewport(Vec2 worldPoint);
        static const Vec2 world_to_viewport_clamp(Vec2 worldPoint);
        static Camera* main_camera();
    };

} // namespace RoninEngine::Runtime
