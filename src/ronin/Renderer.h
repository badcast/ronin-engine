#pragma once

#include "dependency.h"
#include "Component.h"

namespace RoninEngine::Runtime
{
	struct Render_info
	{
          Rect src;
          Rectf_t dst;
          SDL_Renderer* renderer;
          Texture* texture;
	};

	class Renderer : public Component
	{
	public:
		Renderer();
          Renderer(const std::string& name);
		Renderer(const Renderer&) = delete;

		virtual Vec2 GetSize() = 0;

		virtual void Render(Render_info* render_info) = 0;
	}; 
}

