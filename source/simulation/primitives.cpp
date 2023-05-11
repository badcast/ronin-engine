#include "ronin.h"

using namespace RoninEngine;
namespace RoninEngine::Runtime
{

    GameObject* Primitive::CreateEmptyGameObject(Vec2 position) { return CreateGameObject(); }

    Camera2D* Primitive::CreateCamera2D(Vec2 position) { return CreateEmptyGameObject(position)->addComponent<Camera2D>(); }

    Sprite* Primitive::CreateSprite2D(Vec2Int size, Color fillColor)
    {
        Sprite* sprite;
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, sdl_default_pixelformat);
        SDL_LockSurface(surface);
        SDL_memset4(surface->pixels, fillColor, size.x * size.y);
        SDL_UnlockSurface(surface);
        GC::gc_alloc_sprite_with(&sprite, surface, { 0, 0, size.x, size.y });
        return sprite;
    }
}
