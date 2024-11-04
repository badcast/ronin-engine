#include "ronin_splash_world.h"

#include "ronin_logo.hpp"

using namespace RoninEngine::Runtime;

RoninSplashWorld::RoninSplashWorld() : World("Ronin Splash World") {}

void RoninSplashWorld::OnAwake()
{

    Camera2D * cam = Primitive::CreateCamera2D();
    cam->backcolor = Color::black;

    logo = Primitive::CreateSpriteFrom(private_load_surface(RoninEngine_logo_png, RoninEngine_logo_png_len, false));

    objCenter = Primitive::CreateEmptyGameObject()->AddComponent<SpriteRenderer>();
    objCenter->setSprite(logo);


}

void RoninSplashWorld::OnUpdate()
{
    float t = Time::time();

    float perc;

    if(t < 3)
    {
       perc = Math::Clamp01(t/1);

    }
    else if(t < 5)
    {
        perc = 1;
    }
    else
    {
        // Load Now!
        RoninEngine::RoninSimulator::LoadWorld(nextWorld);
    }


    Camera2D::mainCamera()->backcolor = Color::Lerp(Camera2D::mainCamera()->backcolor, Color("#3b3b3b"),  perc);

    this->objCenter->setColor(Color(this->objCenter->color, Math::Clamp01(.3+perc)*255));

    static_cast<Camera2D*>(Camera2D::mainCamera())->SetZoomOut(20 * perc-20 + 150);
}
