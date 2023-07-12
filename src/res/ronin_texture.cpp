#include "ronin.h"

namespace RoninEngine::Runtime
{
    Texture::Texture(native_texture_t *native)
    {
        m_native = native;
    }

    Texture::~Texture()
    {
        if(m_native != nullptr)
        {
            SDL_DestroyTexture(m_native);
            m_native = nullptr;
        }
    }

    const bool Texture::valid()
    {
        return !(!m_native || width() <= 0 || height() <= 0);
    }

    const int Texture::width()
    {
        int w;
        if(SDL_QueryTexture(m_native, nullptr, nullptr, &w, nullptr))
            RoninSimulator::back_fail();
        return w;
    }
    const int Texture::height()
    {
        int h;
        if(SDL_QueryTexture(m_native, nullptr, nullptr, nullptr, &h))
            RoninSimulator::back_fail();
        return h;
    }

    const std::uint32_t Texture::format()
    {
        std::uint32_t f;
        if(SDL_QueryTexture(m_native, &f, nullptr, nullptr, nullptr))
            RoninSimulator::back_fail();
        return f;
    }

    const std::uint32_t Texture::blendMode()
    {
        SDL_BlendMode bmode;

        if(SDL_GetTextureBlendMode(m_native, &bmode))
            RoninSimulator::back_fail();

        return static_cast<std::uint32_t>(bmode);
    }
    const void Texture::blendMode(const int blendMode)
    {
        if(SDL_SetTextureBlendMode(m_native, static_cast<SDL_BlendMode>(blendMode)))
            RoninSimulator::fail("Error unsupported operation");
    }
    const int Texture::scaleMode()
    {
        SDL_ScaleMode smode;

        if(SDL_GetTextureScaleMode(m_native, &smode))
            RoninSimulator::back_fail();

        return smode;
    }
    const void Texture::scaleMode(const int scaleMode)
    {
        if(SDL_SetTextureScaleMode(m_native, static_cast<SDL_ScaleMode>(scaleMode)))
            RoninSimulator::back_fail();
    }

    const int Texture::access()
    {
        int ac;
        if(SDL_QueryTexture(m_native, nullptr, &ac, nullptr, nullptr))
            RoninSimulator::back_fail();
        return static_cast<SDL_TextureAccess>(ac);
    }

    const Color Texture::color()
    {
        Color c;
        if(!(!SDL_GetTextureColorMod(m_native, &c.r, &c.g, &c.b) && !SDL_GetTextureAlphaMod(m_native, &c.a)))
            RoninSimulator::back_fail();

        return c;
    }
    const void Texture::color(const Color value)
    {
        if(!(!SDL_SetTextureColorMod(m_native, value.r, value.g, value.b) &&
             !SDL_SetTextureAlphaMod(m_native, value.a)))
            RoninSimulator::back_fail();
    }

    int Texture::lockTexture(const native_rect_t *rect, void **pixels, int *pitch)
    {
        return SDL_LockTexture(m_native, rect, pixels, pitch);
    }

    void Texture::unlockTexture()
    {
        SDL_UnlockTexture(m_native);
    }

    native_texture_t *Texture::native()
    {
        return m_native;
    }

    const native_texture_t *Texture::cnative()
    {
        return m_native;
    }

    const Rect Texture::getRect()
    {
        return {0, 0, width(), height()};
    }

    const std::string Texture::name()
    {
        return std::string(_name.empty() ? "Unknown" : _name);
    }
} // namespace RoninEngine::Runtime
