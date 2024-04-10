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

    void render_texture_extension(SDL_Texture *texture, const Vec2Int *baseSize, const SDL_Rect *rect, float angleRad);

    void render_cache_unref(SDL_Texture *texture);

    void cache_render_flush_all();

    void render_sprite_renderer(RenderCommand command, SpriteRenderer *target, Rendering *rendering);

} // namespace RoninEngine::Runtime
