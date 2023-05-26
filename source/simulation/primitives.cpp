#include "ronin.h"

using namespace RoninEngine;
namespace RoninEngine::Runtime
{

    GameObject* Primitive::create_empty_game_object(Vec2 position) { return create_game_object(); }

    GameObject* Primitive::create_box2d(Vec2 position, float angle)
    {
        GameObject* obj = create_empty_game_object(position);

        obj->add_component<SpriteRenderer>()->set_sprite(create_sprite2D_box());

        return obj;
    }

    Camera2D* Primitive::create_camera2D(Vec2 position) { return create_empty_game_object(position)->add_component<Camera2D>(); }

    Sprite* Primitive::create_sprite2D_box(Vec2 size, Color fillColor)
    {
        Sprite* sprite;

        size *= pixelsPerPoint;
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, sdl_default_pixelformat);
        if (surface == nullptr) {
            Application::fail_oom_kill();
        }
        SDL_LockSurface(surface);
        SDL_memset4(surface->pixels, fillColor, (int)size.x * (int)size.y);
        SDL_UnlockSurface(surface);
        ResourceManager::gc_alloc_sprite_with(&sprite, surface, { 0, 0, size.x, size.y });
        return sprite;
    }

    Sprite* Primitive::create_sprite2D_circle(Vec2 size, float radius, Color fillColor)
    {
        Sprite* sprite;

        size *= pixelsPerPoint;
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, sdl_default_pixelformat);
        if (surface == nullptr) {
            Application::fail_oom_kill();
        }
        SDL_LockSurface(surface);
        // clear
        memset(surface->pixels, 0, surface->h * surface->pitch);
        SDL_UnlockSurface(surface);

        SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);

        // draw circle
        filledCircleColor(renderer, size.x / 2, size.y / 2, static_cast<std::uint16_t>(radius * pixelsPerPoint / 2) - 1, fillColor);

        SDL_DestroyRenderer(renderer);

        ResourceManager::gc_alloc_sprite_with(&sprite, surface, { 0, 0, size.x, size.y });
        return sprite;
    }
}
