#include "ronin.h"

namespace RoninEngine::Runtime
{
    static SDL_Surface *spotlightMask = nullptr;

    Spotlight::Spotlight() : Spotlight(DESCRIBE_AS_MAIN_OFF(Spotlight))
    {
    }
    Spotlight::Spotlight(const std::string &name) : Light(DESCRIBE_AS_ONLY_NAME(Spotlight))
    {
        DESCRIBE_AS_MAIN(Spotlight);
        range = 1.f;
        intensity = 1.f;
        if(!spotlightMask)
        {
            // spotlightMask = Resources::resource_bitmap("spotlight");
        }
    }

    Spotlight::~Spotlight()
    {
    }

    void Spotlight::get_light()
    {

        SDL_Texture *texture = SDL_CreateTextureFromSurface(gscope.renderer, spotlightMask);
        SDL_TextureAccess access;

        SDL_QueryTexture(texture, nullptr, reinterpret_cast<int *>(&access), nullptr, nullptr);

        SDL_RenderCopyEx(gscope.renderer, texture, nullptr, nullptr, transform()->angle(), nullptr, SDL_RendererFlip::SDL_FLIP_NONE);

        SDL_DestroyTexture(texture);
    }

} // namespace RoninEngine::Runtime
