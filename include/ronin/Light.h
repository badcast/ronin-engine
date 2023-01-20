#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
     class SHARK Light : public Component
	{
        Texture* fieldFogTexture;
	protected:
		virtual void GetLight(SDL_Renderer* renderer) = 0;
	public:
		Light();
          Light(const std::string& name);
		virtual ~Light();
		void GetLightSource(Render_info* render_info);
	};
}

