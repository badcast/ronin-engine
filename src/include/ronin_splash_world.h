#pragma once

#include "ronin.h"

namespace RoninEngine::Runtime
{
    class RoninSplashWorld : public World
    {
    public:
        World * nextWorld;
        Sprite * logo;
        SpriteRenderer * objCenter;
        UI::uid loadBar;

        float delay = 5.0f;

        RoninSplashWorld();

        void OnAwake();

        void OnUpdate();
    };

}
