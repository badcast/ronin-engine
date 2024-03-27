#include "ronin.h"

using namespace RoninEngine;
namespace RoninEngine::Runtime
{
    GameObject *Primitive::CreateEmptyGameObject(Vec2 position, float angle)
    {
        GameObject *obj = create_game_object();
        obj->transform()->position(position);
        obj->transform()->angle(angle);
        return obj;
    }

    GameObject *Primitive::CreateEmptyGameObject(const std::string &name, Vec2 position, float angle)
    {
        GameObject *obj = create_game_object();
        obj->transform()->position(position);
        obj->transform()->angle(angle);
        obj->name(name);
        return obj;
    }

    GameObject *Primitive::CreateBox2D(Vec2 position, float angle, Vec2 size, Color fillColor)
    {
        GameObject *obj = CreateEmptyGameObject(position);
        obj->AddComponent<SpriteRenderer>()->setSprite(CreateSpriteRectangle(true, Vec2::one, fillColor));
        obj->GetComponent<SpriteRenderer>()->setSize(size);
        obj->transform()->angle(angle);
        return obj;
    }

    Camera2D *Primitive::CreateCamera2D(Vec2 position)
    {
        return CreateEmptyGameObject(position)->AddComponent<Camera2D>();
    }

    Sprite *Primitive::CreateEmptySprite(bool localSprite)
    {
        Sprite *sprite;
        RoninMemory::alloc_self(sprite);
        gid_get(localSprite)->gid_sprites.push_back(sprite);
        return sprite;
    }

    Sprite *Primitive::CreateSpriteRectangle(bool localSprite, Vec2 size, Color fillColor)
    {
        Sprite *sprite;

        size *= pixelsPerPoint;
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, ronin_default_pixelformat);
        if(surface == nullptr)
        {
            RoninSimulator::Kill();
        }
        SDL_LockSurface(surface);
        SDL_memset4(surface->pixels, fillColor, surface->w * surface->h);
        SDL_UnlockSurface(surface);
        switched_world->irs->offload_surfaces.push_back(surface);
        RoninMemory::alloc_self(sprite, surface, Rect(0, 0, size.x, size.y));
        gid_get(localSprite)->gid_sprites.push_back(sprite);
        return sprite;
    }

    Sprite *Primitive::CreateSpriteCircle(bool localSprite, Vec2 size, float radius, Color fillColor)
    {
        Sprite *sprite;

        size *= pixelsPerPoint;
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, ronin_default_pixelformat);
        if(surface == nullptr)
        {
            RoninSimulator::Kill();
        }
        SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);

        // draw circle
        filledCircleColor(renderer, size.x / 2, size.y / 2, static_cast<std::uint16_t>(radius * pixelsPerPoint / 2) - 1, fillColor);

        SDL_DestroyRenderer(renderer);
        switched_world->irs->offload_surfaces.push_back(surface);
        RoninMemory::alloc_self(sprite, surface, Rect(0, 0, size.x, size.y));
        gid_get(localSprite)->gid_sprites.push_back(sprite);
        return sprite;
    }

    Sprite *Primitive::CreateSpriteTriangle(bool localSprite, Vec2 size, float height, Color fillColor)
    {
        Sprite *sprite;

        size *= pixelsPerPoint;
        height *= pixelsPerPoint;
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, ronin_default_pixelformat);
        if(surface == nullptr)
        {
            RoninSimulator::Kill();
        }

        // drawing triangle
        SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);
        filledTrigonColor(renderer, 0, size.y, size.x, size.y, size.x / 2, 0, fillColor);
        SDL_DestroyRenderer(renderer);

        //        SDL_LockSurface(surface);
        //        height *= pixelsPerPoint;
        //        int minHeight = Math::min<int>(height, surface->h);
        //        for (int y = minHeight; y >= 0; --y) {
        //            int z = (surface->w);
        //            for (int x = 0; x < z; ++x) {
        //                // NOTE: Формула пикселей для SDL :: Y Offset * (Pitch/BytesPerPixel) + X Offset
        //                SDL_memset4(surface->pixels + (y * (surface->pitch / surface->format->BytesPerPixel) + x),
        //                fillColor, sizeof(fillColor));
        //            }
        //        }
        //        SDL_UnlockSurface(surface);
        switched_world->irs->offload_surfaces.push_back(surface);
        RoninMemory::alloc_self(sprite, surface, Rect {0, 0, static_cast<int>(size.x), static_cast<int>(size.y)});
        gid_get(localSprite)->gid_sprites.push_back(sprite);
        return sprite;
    }

    Sprite *Primitive::CreateSpriteFrom(Image *surface, bool localSprite)
    {
        Sprite *sprite = CreateEmptySprite(localSprite);
        sprite->setSurface(surface);
        return sprite;
    }
} // namespace RoninEngine::Runtime
