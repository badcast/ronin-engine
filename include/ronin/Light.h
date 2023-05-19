#pragma once

#include "Component.h"

namespace RoninEngine::Runtime
{
     class RONIN_API Light : public Component
	{
        Texture* fieldFogTexture;
	protected:
		virtual void GetLight(SDL_Renderer* renderer) = 0;
	public:
		Light();
          Light(const std::string& name);
		virtual ~Light();
		void GetLightSource(Rendering* render_info);
	};
}

