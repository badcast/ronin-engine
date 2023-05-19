#include "ronin.h"

namespace RoninEngine::Runtime
{
    static SDL_Surface* spotlightMask = nullptr;

    Spotlight::Spotlight()
        : Spotlight(DESCRIBE_TYPE(Spotlight, this, &t))
    {
    }
    Spotlight::Spotlight(const std::string& name)
        : Light(name)
    {
        range = 1.f;
        intensity = 1.f;
        if (!spotlightMask) {
            spotlightMask = ResourceManager::resource_bitmap("spotlight");
        }
    }

    Spotlight::~Spotlight() { }

    void Spotlight::GetLight(SDL_Renderer* renderer)
    {

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, spotlightMask);
        SDL_TextureAccess access;

        SDL_QueryTexture(texture, nullptr, reinterpret_cast<int*>(&access), nullptr, nullptr);

        SDL_RenderCopyEx(renderer, texture, nullptr, nullptr, transform()->angle(), nullptr, SDL_RendererFlip::SDL_FLIP_NONE);

        SDL_DestroyTexture(texture);
    }

} // namespace RoninEngine::Runtime
