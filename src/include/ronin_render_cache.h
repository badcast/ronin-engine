#pragma once

#include "ronin.h"
#include "Sprite.h"
#include "SpriteRenderer.h"

namespace RoninEngine::Runtime
{

    SDL_Texture *render_make_texture(SDL_Surface *surface);

    /**
     * @brief render_cache_texture
     * @param sprite
     * @return
     */
    SDL_Texture *render_cache_texture(Sprite *sprite);

    void render_texture_extension(SDL_Texture *texture, const Vec2Int *baseSize, const SDL_Rect *srcFrom, const SDL_Rect *rect, float angleRad);

    void render_texture_extension2(SDL_Texture *texture, const Vec2Int *baseSize, const SDL_Rect *extent, float angleRad);

    void render_cache_unref(SDL_Texture *texture);

    void cache_render_flush_all();

    void native_render_2D(Camera2D *camera);
} // namespace RoninEngine::Runtime
