#include "ronin.h"

namespace RoninEngine::Runtime
{
    Texture::Texture() { m_native = nullptr; }

    Texture::~Texture()
    {
        if (m_native != nullptr) {
            SDL_DestroyTexture(m_native);
            m_native = nullptr;
        }
    }

    const bool Texture::valid() { return !(!m_native || width() <= 0 || height() <= 0); }

    const int Texture::width()
    {
        int w;
        if (SDL_QueryTexture(m_native, nullptr, nullptr, &w, nullptr))
            Application::back_fail();
        return w;
    }
    const int Texture::height()
    {
        int h;
        if (SDL_QueryTexture(m_native, nullptr, nullptr, nullptr, &h))
            Application::back_fail();
        return h;
    }

    const SDL_PixelFormatEnum Texture::format()
    {
        uint32_t f;
        if (SDL_QueryTexture(m_native, &f, nullptr, nullptr, nullptr))
            Application::back_fail();
        return static_cast<SDL_PixelFormatEnum>(f);
    }

    const SDL_BlendMode Texture::blendMode()
    {
        SDL_BlendMode bmode;

        if (SDL_GetTextureBlendMode(m_native, &bmode))
            Application::back_fail();

        return bmode;
    }
    const void Texture::blendMode(const SDL_BlendMode blendMode)
    {
        if (SDL_SetTextureBlendMode(m_native, blendMode))
            Application::fail("Error unsupported operation");
    }
    const SDL_ScaleMode Texture::scaleMode()
    {
        SDL_ScaleMode smode;

        if (SDL_GetTextureScaleMode(m_native, &smode))
            Application::back_fail();

        return smode;
    }
    const void Texture::scaleMode(const SDL_ScaleMode scaleMode)
    {
        if (SDL_SetTextureScaleMode(m_native, scaleMode))
            Application::back_fail();
    }

    const SDL_TextureAccess Texture::access()
    {
        int ac;
        if (SDL_QueryTexture(m_native, nullptr, &ac, nullptr, nullptr))
            Application::back_fail();
        return static_cast<SDL_TextureAccess>(ac);
    }

    const Color Texture::color()
    {
        Color c;
        if (!(!SDL_GetTextureColorMod(m_native, &c.r, &c.g, &c.b) && !SDL_GetTextureAlphaMod(m_native, &c.a)))
            Application::back_fail();

        return c;
    }
    const void Texture::color(const Color value)
    {
        if (!(!SDL_SetTextureColorMod(m_native, value.r, value.g, value.b) && !SDL_SetTextureAlphaMod(m_native, value.a)))
            Application::back_fail();
    }

    void Texture::lockTexture(const SDL_Rect* rect, void** pixels, int* pitch) { SDL_LockTexture(m_native, rect, pixels, pitch); }

    void Texture::unlockTexture() { SDL_UnlockTexture(m_native); }

    SDL_Texture* Texture::native() { return m_native; }

    const SDL_Texture* Texture::cnative() { return m_native; }

    const Rect Texture::getRect() { return { 0, 0, width(), height() }; }

    const Texture* Texture::clone() { return clone(Application::GetRenderer()); }

    const Texture* Texture::clone(SDL_Renderer* renderer)
    {
        SDL_Texture* __t = SDL_GetRenderTarget(renderer);
        Texture* _n;
        GC::gc_alloc_texture(&_n, width(), height(), format(), access());
        _n->blendMode(blendMode());
        _n->scaleMode(scaleMode());
        _n->color(color());
        SDL_CreateTextureFromSurface(renderer, nullptr);
        SDL_SetRenderTarget(renderer, _n->m_native);
        SDL_RenderCopy(renderer, m_native, nullptr, nullptr);
        SDL_SetRenderTarget(renderer, __t);
        return _n;
    }

    const std::string Texture::name() { return std::string(_name.empty() ? "Unknown" : _name); }
} // namespace RoninEngine::Runtime
