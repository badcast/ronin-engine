#pragma once

#include "dependency.h"

namespace RoninEngine::Runtime
{
	class Texture
	{
		friend class Camera;
		friend class GC;

	private:
		::SDL_Texture* m_native;
          std::string _name;

	public:
         Texture();
		~Texture();

		const bool valid();

		const int width();
		const int height();

		const SDL_PixelFormatEnum format();

		const SDL_BlendMode blendMode();
		const void blendMode(const SDL_BlendMode blendMode);
		const SDL_ScaleMode scaleMode();
		const void scaleMode(const SDL_ScaleMode scaleMode);

		const SDL_TextureAccess access();

		const Color color();
		const void color(const Color value);

		void lockTexture(const SDL_Rect* rect, void** pixels, int* pitch);
        void unlockTexture();

		SDL_Texture* native();
		const SDL_Texture* cnative();

		const SDL_Rect getRect();

		//Create identity texture
		const Texture* clone();
		//Create identity texture
		const Texture* clone(SDL_Renderer* renderer);

          const std::string name();
	};
}

