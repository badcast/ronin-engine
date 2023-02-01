#pragma once
#include "begin.h"

namespace RoninEngine::Runtime {
class SHARK Spotlight final : public Light {
   public:
    //Диапозон
    float range;
    //Интенсивность света
    float intensity;

    Spotlight();
    Spotlight(const std::string& name);
    ~Spotlight();

   protected:
    virtual void GetLight(SDL_Renderer* renderer);
};
}  // namespace RoninEngine::Runtime
