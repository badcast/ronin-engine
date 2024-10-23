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

        Camera2D();
        Camera2D(const std::string &name);
        Camera2D(const Camera2D &other);

        /**
         * @brief Set the out zoom in scales.
         * @param zoomLevel of the outzooming. The param default zoomLevel is 100.0 (Range from 0.0 to 100.0).
         */
        void SetZoomOut(float zoomLevel);
        float GetZoomOut();
    };

} // namespace RoninEngine::Runtime
