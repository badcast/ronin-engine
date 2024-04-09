#pragma once

#include "ronin.h"
#include "Sprite.h"

namespace RoninEngine::Runtime
{

    SDL_Texture *render_make_texture(Sprite *sprite);

    /**
     * @brief render_cache_texture
     * @param sprite
     * @return
     */
    SDL_Texture *render_cache_texture(Sprite *sprite);

    void render_texture_extension(SDL_Texture *texture, const Vec2Int *baseSize, const SDL_Rect *rect, float angleRad);

    void render_cache_unref(SDL_Texture *texture);

    void cache_render_flush_all();
} // namespace RoninEngine::Runtime
