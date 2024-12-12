#include "ronin.h"

using namespace RoninEngine;

using namespace RoninEngine::Runtime;

GameObjectRef Primitive::CreateEmptyGameObject(Vec2 position, float angle)
{
    GameObjectRef obj = create_game_object();
    obj->transform()->position(position);
    obj->transform()->angle(angle);
    return obj;
}

GameObjectRef Primitive::CreateEmptyGameObject(const std::string &name, Vec2 position, float angle)
{
    GameObjectRef obj = create_game_object();
    obj->transform()->position(position);
    obj->transform()->angle(angle);
    obj->name(name);
    return obj;
}

GameObjectRef Primitive::CreateBox2D(Vec2 position, float angle, Vec2 size, Color fillColor)
{
    GameObjectRef obj = CreateEmptyGameObject(position);
    obj->AddComponent<SpriteRenderer>()->setSprite(CreateSpriteRectangle(Vec2::one, fillColor));
    obj->GetComponent<SpriteRenderer>()->setSize(size);
    obj->transform()->angle(angle);
    return obj;
}

Camera2DRef Primitive::CreateCamera2D(Vec2 position)
{
    return CreateEmptyGameObject(position)->AddComponent<Camera2D>();
}

SpriteRef Primitive::CreateEmptySprite()
{
    Sprite *sprite;
    RoninMemory::alloc_self(sprite);
    return SpriteRef{sprite};
}

SpriteRef Primitive::CreateSpriteRectangle(Vec2 size, Color fillColor)
{
    Sprite *sprite;

    size = Vec2::Scale(size, currentWorld->irs->metricPixelsPerPoint);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, defaultPixelFormat);
    if(surface == nullptr)
    {
        RoninSimulator::Kill();
    }
    SDL_LockSurface(surface);
    SDL_memset4(surface->pixels, static_cast<std::uint32_t>(fillColor), surface->w * surface->h);
    SDL_UnlockSurface(surface);
    currentWorld->irs->preloadeSurfaces.push_back(surface);
    RoninMemory::alloc_self(sprite, surface, Rect(0, 0, size.x, size.y));
    return SpriteRef{sprite};
}

SpriteRef Primitive::CreateSpriteCircle(Vec2 size, float radius, Color fillColor)
{
    Sprite *sprite;

    size = Vec2::Scale(size, currentWorld->irs->metricPixelsPerPoint);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, defaultPixelFormat);
    if(surface == nullptr)
    {
        RoninSimulator::Kill();
    }
    currentWorld->irs->preloadeSurfaces.push_back(surface);
    SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);

    // draw circle
    filledCircleColor(renderer, size.x / 2, size.y / 2, static_cast<std::uint16_t>(radius * currentWorld->irs->metricPixelsPerPoint.x / 2) - 1, fillColor);

    SDL_DestroyRenderer(renderer);
    RoninMemory::alloc_self(sprite, surface, Rect(0, 0, size.x, size.y));
    return SpriteRef{sprite};
}

SpriteRef Primitive::CreateSpriteTriangle(Vec2 size, float height, Color fillColor)
{
    Sprite *sprite;

    size = Vec2::Scale(size, currentWorld->irs->metricPixelsPerPoint);
    height *= currentWorld->irs->metricPixelsPerPoint.y;
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, defaultPixelFormat);
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
    currentWorld->irs->preloadeSurfaces.push_back(surface);
    RoninMemory::alloc_self(sprite, surface, Rect {0, 0, static_cast<int>(size.x), static_cast<int>(size.y)});
    return SpriteRef{sprite};
}

SpriteRef Primitive::CreateSpriteFrom(Image *surface)
{
    SpriteRef sprite = CreateEmptySprite();
    sprite->setSurface(surface);
    return sprite;
}
