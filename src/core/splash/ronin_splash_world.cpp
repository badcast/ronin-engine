#include "ronin_splash_world.h"

#include "ronin_logo.hpp"

using namespace RoninEngine::Runtime;

RoninSplashWorld::RoninSplashWorld() : World("Ronin Splash World")
{
}

void RoninSplashWorld::OnAwake()
{
    Rect rect;
    Camera2DRef cam = Primitive::CreateCamera2D();
    cam->backcolor = Color::black;
    logo = Primitive::CreateSpriteFrom(private_load_surface(RoninEngine_logo_png, RoninEngine_logo_png_len, false));
    objCenter = Primitive::CreateEmptyGameObject()->AddComponent<SpriteRenderer>();
    objCenter->setSprite(logo);
    rect.setXY(Camera::ViewportToScreenPoint({.5f, 0.9f}));
    rect.setWH(Vec2 {250, 25});
    rect.x -= rect.w / 2;
}

void RoninSplashWorld::OnUpdate()
{
    float perc;
    float t;
    perc = 0;
    t = Time::time();
    if(t < 2)
    {
        perc = Math::Clamp01(t / 2);
        StaticCast<Camera2D>(Camera2D::mainCamera())->SetZoomOut(20 * perc - 20 + 150);
    }
    else if(t < 4)
    {
        perc = 1 - Math::Clamp01((t - 2) / 2);
    }
    else if(t >= 5)
    {
        // Load Now!
        RoninEngine::RoninSimulator::LoadWorld(nextWorld);
    }
    Camera2D::mainCamera()->backcolor = Color::Lerp(Camera2D::mainCamera()->backcolor, "#3b3b3b", perc);
    objCenter->setColor(Color(objCenter->color, Math::Clamp01(perc) * 255));
}
