#pragma once

#include "ronin.h"
#include "Sprite.h"

namespace RoninEngine::Runtime
{
    /**
     * @brief render_cache_texture
     * @param sprite
     * @return
     */
    SDL_Texture *render_cache_texture(Sprite *sprite);

    void render_texture_extension(SDL_Texture *texture, const SDL_Rect *rect, float angleRad);

    void cache_render_flush_all();
} // namespace RoninEngine::Runtime
