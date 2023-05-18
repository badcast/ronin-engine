#pragma once

#include "begin.h"

namespace RoninEngine::Runtime {
class RONIN_API Texture {
    friend class Camera;
    friend class ResourceManager;

private:
    ::SDL_Texture* m_native;
    std::string _name;

public:
    Texture() = default;
    Texture(SDL_Texture* native);
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
    SDL_Texture* native();
    const SDL_Texture* cnative();
    // Create identity texture
    const Texture* clone(SDL_Renderer* renderer);
    const Color color();
    const void color(const Color value);

    int lockTexture(const SDL_Rect* rect, void** pixels, int* pitch);
    void unlockTexture();

    const Rect getRect();
    // Create identity texture
    const Texture* clone();

    const std::string name();
};
} // namespace RoninEngine::Runtime
