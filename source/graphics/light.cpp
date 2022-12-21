#include "Light.h"
#include "inputSystem.h"
#include "ronin.h"

namespace RoninEngine::Runtime {
Light::Light() : Light(typeid(Light).name()) {}
Light::Light(const std::string &name) : Component(name) { fieldFogTexture = nullptr; }

Light::~Light() {}

void Light::GetLightSource(Render_info *render) {
  auto res = Application::getResolution();
  Color c;
  SDL_Texture *target, *lastTarget;
  //TODO: not a work light sources
  return;
  if (!fieldFogTexture) {
    GC::gc_alloc_texture(&fieldFogTexture, res.width, res.height,
                         SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING);
    fieldFogTexture->blendMode(SDL_BlendMode::SDL_BLENDMODE_BLEND);

    c = fieldFogTexture->color();
    c.r = 0;
    c.g = 0;
    c.b = 0;
    c.a = 255;

    fieldFogTexture->color(c);
  }

  lastTarget = SDL_GetRenderTarget(render->renderer);

  target = SDL_CreateTexture(render->renderer, SDL_PIXELFORMAT_RGBA8888,
                             SDL_TextureAccess::SDL_TEXTUREACCESS_TARGET,
                             res.width, res.height);

  SDL_SetRenderTarget(render->renderer, target);
  SDL_SetRenderDrawColor(render->renderer, 0, 0, 0, 55);
  //SDL_RenderClear(render->renderer);

  // Get light
  this->GetLight(render->renderer);

  SDL_SetRenderTarget(render->renderer, lastTarget);
  SDL_RenderCopy(render->renderer, target, nullptr, nullptr); // copy
  SDL_DestroyTexture(target);
}
} // namespace RoninEngine::Runtime
