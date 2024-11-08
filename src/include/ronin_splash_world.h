#pragma once

#include "ronin.h"

namespace RoninEngine::Runtime
{
    class RoninSplashWorld : public World
    {
    public:
        World *nextWorld;
        SpriteRef logo;
        SpriteRendererRef objCenter;

        float delay = 5.0f;

        RoninSplashWorld();

        void OnAwake();

        void OnUpdate();
    };

} // namespace RoninEngine::Runtime
