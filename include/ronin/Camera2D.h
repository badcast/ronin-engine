#pragma once
#include "Camera.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Camera2D final : public Camera
    {
    public:
        std::uint16_t distanceEvcall;

        bool visibleBorders;
        bool visibleGrids;
        bool visibleObjects;
        bool visibleNames;
        Vec2 scale;

        Camera2D();
        Camera2D(const std::string &name);
        Camera2D(const Camera2D &other);
    };

} // namespace RoninEngine::Runtime
