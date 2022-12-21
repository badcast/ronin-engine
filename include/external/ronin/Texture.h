#pragma once

#include "begin.h"

namespace RoninEngine::Runtime {
class SHARK Texture {
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
#ifdef RONIN_COMPILLING
    const SDL_PixelFormatEnum format();

    const SDL_BlendMode blendMode();
    const void blendMode(const SDL_BlendMode blendMode);
    const SDL_ScaleMode scaleMode();
    const void scaleMode(const SDL_ScaleMode scaleMode);

    const SDL_TextureAccess access();
    SDL_Texture* native();
    const SDL_Texture* cnative();
    // Create identity texture
    const Texture* clone(SDL_Renderer* renderer);
#else
    void* native();
    const void* cnative();
#endif
    const Color color();
    const void color(const Color value);

    void lockTexture(const SDL_Rect* rect, void** pixels, int* pitch);
    void unlockTexture();

    const Rect getRect();
    // Create identity texture
    const Texture* clone();

    const std::string name();
};
} // namespace RoninEngine::Runtime
