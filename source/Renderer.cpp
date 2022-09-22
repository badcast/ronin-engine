#include "framework.h"
#include "Renderer.h"
namespace RoninEngine::Runtime {
Renderer::Renderer() : Renderer("Renderer Component") {}

Renderer::Renderer(const std::string& name) : Component(name) {}
}  // namespace RoninEngine::Runtime
