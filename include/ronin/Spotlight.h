#pragma once
#include "dependency.h"

namespace RoninEngine::Runtime
{
	class Spotlight final :
		public Light
	{
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
}



