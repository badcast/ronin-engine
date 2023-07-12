#pragma once

#include "begin.h"

namespace RoninEngine::Runtime
{
    class RONIN_API Texture
    {
    private:
        native_texture_t *m_native;
        std::string _name;

    public:
        Texture() = default;
        Texture(native_texture_t *native);
        ~Texture();

        const bool valid();

        const int width();
        const int height();
        const std::uint32_t format();

        const std::uint32_t blendMode();
        const void blendMode(const int blendMode);
        const int scaleMode();
        const void scaleMode(const int scaleMode);

        const int access();
        native_texture_t *native();
        const native_texture_t *cnative();
        // Create identity texture
        const Color color();
        const void color(const Color value);

        int lockTexture(const native_rect_t *rect, void **pixels, int *pitch);
        void unlockTexture();

        const Rect getRect();

        const std::string name();
    };
} // namespace RoninEngine::Runtime
