#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Camera : public Component
    {
        friend RoninEngine::Level;

    protected:
        bool targetClear;
        std::map<int, std::set<Renderer*>> renders;
        std::set<Light*> __lightsOutResults;
        virtual void render(SDL_Renderer* renderer, Rect rect, GameObject* root) = 0;

    public:
        bool enabled;
        std::uint32_t distanceEvcall;

        Camera();
        Camera(const std::string& name);

        virtual ~Camera();

        bool isFocused();
        void Focus();

        std::tuple<std::map<int, std::set<Renderer*>>*, std::set<Light*>*> matrixSelection();

        RONIN_API static const Vec2 ScreenToWorldPoint(Vec2 screenPoint);
        RONIN_API static const Vec2 WorldToScreenPoint(Vec2 worldPoint);
        RONIN_API static const Vec2 ViewportToWorldPoint(Vec2 viewportPoint);
        RONIN_API static const Vec2 WorldToViewport(Vec2 worldPoint);
        RONIN_API static const Vec2 WorldToViewportClamp(Vec2 worldPoint);
        RONIN_API static Camera* mainCamera();
    };

} // namespace RoninEngine::Runtime
