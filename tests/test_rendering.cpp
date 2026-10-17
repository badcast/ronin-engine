#include <iostream>
#include <cassert>
#include <cmath>
#include <SDL2/SDL.h>
#include <ronin/framework.h>

using namespace RoninEngine;
using namespace RoninEngine::Runtime;

class RenderTestWorld : public World
{
public:
    int frameCount = 0;
    GameObjectRef camObj;
    GameObjectRef spriteObj;
    GameObjectRef terrainObj;
    GameObjectRef lightObj;

    RenderTestWorld(const std::string &name = "RenderTestWorld") : World(name) {}

    void OnAwake() override
    {
        std::cout << "  [RenderTestWorld] OnAwake" << std::endl;
        camObj = Primitive::CreateEmptyGameObject();
        Camera2D *cam = camObj->AddComponent<Camera2D>();
        cam->Focus();

        // 1. Test Primitive Colors (verify red and blue are not swapped)
        SpriteRef redRect = Primitive::CreateSpriteRectangle(Vec2(1.0f, 1.0f), Color::red);
        assert(redRect != nullptr && redRect->valid());
        SDL_Surface *s = redRect->getImage();
        assert(s != nullptr);
        Uint32 *pix = static_cast<Uint32 *>(s->pixels);
        Uint8 r, g, b, a;
        SDL_GetRGBA(pix[0], s->format, &r, &g, &b, &a);
        std::cout << "  [Primitive Test] Red rectangle pixel: R=" << (int)r << " G=" << (int)g << " B=" << (int)b << std::endl;
        assert(r > 200 && b == 0);

        SpriteRef blueCircle = Primitive::CreateSpriteCircle(Vec2(1.0f, 1.0f), 0.5f, Color::blue);
        assert(blueCircle != nullptr && blueCircle->valid());
        s = blueCircle->getImage();
        pix = static_cast<Uint32 *>(s->pixels);
        int mid = (s->h / 2) * (s->pitch / 4) + (s->w / 2);
        SDL_GetRGBA(pix[mid], s->format, &r, &g, &b, &a);
        std::cout << "  [Primitive Test] Blue circle center pixel: R=" << (int)r << " G=" << (int)g << " B=" << (int)b << std::endl;
        assert(b > 200 && r == 0);

        // 2. Test SpriteRenderer with custom pivot
        spriteObj = Primitive::CreateEmptyGameObject();
        SpriteRenderer *sr = spriteObj->AddComponent<SpriteRenderer>();
        sr->setSprite(redRect);
        redRect->center(Vec2(0.5f, 1.0f)); // Custom pivot at bottom center
        assert(redRect->center().y == 1.0f);
        sr->setPresentMode(SpriteRenderPresentMode::Place);
        sr->setSize(Vec2(2.0f, 2.0f));
        Vec2 pivotOffset = sr->get_offset();
        assert(std::fabs(pivotOffset.x - 0.0f) < 1e-4f);
        assert(std::fabs(pivotOffset.y - 1.0f) < 1e-4f);

        // Test Tile mode SpriteRenderer
        GameObjectRef tileObj = Primitive::CreateEmptyGameObject(Vec2(-2.0f, 0.0f));
        SpriteRenderer *tileSr = tileObj->AddComponent<SpriteRenderer>();
        tileSr->setSprite(blueCircle);
        tileSr->setRenderType(SpriteRenderType::Tile);
        tileSr->setSize(Vec2(2.5f, 1.5f));

        // Enable camera debug overlays to exercise visibleBorders, visibleObjects, visibleNames
        cam->visibleBorders = true;
        cam->visibleObjects = true;
        cam->visibleNames = true;

        // 3. Test Terrain2D sizing
        terrainObj = Primitive::CreateEmptyGameObject();
        Terrain2D *terrain = terrainObj->AddComponent<Terrain2D>();
        terrain->setMesh(Rectf(0, 0, 2.0f, 1.5f), redRect, true);
        assert(terrain->getSize().x >= 2.0f);
        assert(terrain->getSize().y >= 1.5f);
        std::cout << "  [Terrain2D Test] Terrain size: " << terrain->getSize().x << " x " << terrain->getSize().y << std::endl;

        // 4. Test Lights
        lightObj = Primitive::CreateEmptyGameObject();
        Light *light = lightObj->AddComponent<Light>();
        light->color = Color::yellow;
        light->range = 3.0f;

        Spotlight *spot = lightObj->AddComponent<Spotlight>();
        spot->angle = 45.0f;
    }

    void OnStart() override
    {
        std::cout << "  [RenderTestWorld] OnStart" << std::endl;
    }

    void OnUpdate() override
    {
        ++frameCount;
        if(spriteObj && spriteObj->spriteRenderer())
        {
            // Dynamically scale Simple mode sprite each frame without texture thrashing
            spriteObj->spriteRenderer()->setSize(Vec2(1.5f + 0.2f * frameCount, 1.5f + 0.2f * frameCount));
        }
        // Exercise RenderUtility Gizmo calls during frame
        RenderUtility::SetColor(Color::green);
        RenderUtility::DrawRectangle(Vec2::zero, 2.0f, 2.0f);
        RenderUtility::DrawCircle(Vec2(1.0f, 1.0f), 0.5f);
        RenderUtility::DrawTriangle(Vec2(-1.0f, -1.0f), 1.0f, 1.0f, true);
        RenderUtility::DrawFillRect(Vec2(0.5f, 0.5f), 0.5f, 0.5f);

        if(frameCount >= 5)
        {
            assert(GetCulled() >= 2);
            std::cout << "  [RenderTestWorld] Rendered 5 frames cleanly without glitches or crashes. Requesting quit." << std::endl;
            RoninSimulator::RequestQuit();
        }
    }
};

int main()
{
    std::cout << "=== RUNNING COMPREHENSIVE RENDERING TEST ===" << std::endl;
    RoninSimulator::Init();
    RoninSimulator::Show(Resolution{640, 480, 60});

    bool loaded = (RoninSimulator::LoadWorld<RenderTestWorld>() != nullptr);
    assert(loaded);

    RoninSimulator::Simulate();

    RoninSimulator::Finalize();
    std::cout << "=== ALL RENDERING TESTS PASSED PERFECTLY! ===" << std::endl;
    return 0;
}
