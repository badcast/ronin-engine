#pragma once
#include "Camera.h"

namespace RoninEngine::Runtime {
   class Camera2D final : public Camera {
      protected:
         void render(SDL_Renderer* renderer, Rect rect, GameObject* root);

      public:
         bool visibleBorders;
         bool visibleGrids;
         bool visibleObjects;
         Vec2 scale;

         Camera2D();
         Camera2D(const std::string& name) : Camera(name){}
         Camera2D(const Camera2D&);
   };
}  // namespace RoninEngine::Runtime
