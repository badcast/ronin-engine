#include <iostream>
#include <cassert>
#include <ronin/framework.h>

using namespace RoninEngine;
using namespace RoninEngine::Runtime;
using namespace RoninEngine::AI;
using namespace RoninEngine::UI;

class TestWorld : public World
{
public:
    TestWorld(const std::string &name = "TestWorld") : World(name) {}
    void OnAwake() override {}
    void OnStart() override {}
    void OnUpdate() override {}
};

class DummyScript : public Behaviour
{
public:
    int updateTicks = 0;
    DummyScript() : Behaviour("DummyScript") {}
    void OnUpdate() override
    {
        updateTicks++;
    }
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "Starting Comprehensive Ronin Engine Test" << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Version & Performances
    VersionInfo vInfo = RoninSimulator::GetVersion();
    std::cout << "[1/8] Engine Version: " << (int)vInfo.Engine_Version.major << "." 
              << (int)vInfo.Engine_Version.minor << "." << (int)vInfo.Engine_Version.patch << std::endl;
    std::cout << "      Version String: " << RoninSimulator::GetVersionString() << std::endl;
    assert(std::string(RoninSimulator::GetVersionString()) == "0.0.1-alpha");
    std::cout << "      Memory: " << Perfomances::GetMemoryUsed() << " bytes" << std::endl;
    std::cout << "      CPU Usage: " << Perfomances::GetCPUUsage() << "%" << std::endl;

    // 2. Math, Vec2 & Physics2D SAT Collision
    std::cout << "[2/8] Testing Math & SAT Collision..." << std::endl;
    Vec2 v1(3.0f, 4.0f);
    assert(Math::Abs(v1.magnitude() - 5.0f) < 0.001f);
    assert(Math::Abs(v1.normalized().magnitude() - 1.0f) < 0.001f);

    Rectf rect1(0, 0, 10, 10);
    Rectf rect2(5, 5, 10, 10);
    Rectf rect3(30, 30, 10, 10);
    assert(Physics2D::CheckCollision(rect1, 0.0f, rect2, 0.0f) == true);
    assert(Physics2D::CheckCollision(rect1, 0.0f, rect3, 0.0f) == false);
    // SAT Rotated collision test (45 degrees)
    assert(Physics2D::CheckCollision(rect1, 0.0f, rect2, Math::deg2rad * 45.0f) == true);

    // 3. NavMesh & AI Search
    std::cout << "[3/8] Testing NavMesh & Free Space Search..." << std::endl;
    NavMesh nav(30, 30);
    assert(nav.getWidth() == 30);
    assert(nav.getHeight() == 30);
    nav.setLock(Vec2Int(10, 10), true);
    assert(nav.hasLocked(Vec2Int(10, 10)) == true);
    assert(nav.hasLocked(Vec2Int(10, 11)) == false);

    Vec2Int freePt = nav.FindFreeSpace(Vec2Int(10, 10), 5);
    assert(nav.hasLocked(freePt) == false);

    // 4. World, Hierarchy & ObjectMoveTo
    std::cout << "[4/8] Testing World, Hierarchy & ObjectMoveTo..." << std::endl;
    TestWorld worldA("WorldA");
    TestWorld worldB("WorldB");
    GameObjectRef parentObj = Primitive::CreateEmptyGameObject("ParentObj");
    GameObjectRef childObj = Primitive::CreateEmptyGameObject("ChildObj");
    childObj->transform()->setParent(parentObj->transform());
    assert(childObj->transform()->parent() == parentObj->transform());

    Ref<DummyScript> script = childObj->AddComponent<DummyScript>();
    assert(!script.isNull());

    // Move childObj to worldB
    worldA.ObjectMoveTo(childObj, &worldB);
    assert(childObj->transform()->parent() != parentObj->transform());
    std::cout << "      Object moved between worlds successfully." << std::endl;

    // 5. Atlas & Slicing
    std::cout << "[5/8] Testing Atlas & Grid Slicing..." << std::endl;
    Atlas atlas;
    Rect rSheet{0, 0, 64, 64};
    atlas.AddSprite("sheet", rSheet);
    assert(atlas.GetCount() == 1);
    std::vector<SpriteRef> gridSlices = atlas.SliceGrid(16, 16);
    assert(gridSlices.size() == 16);
    assert(atlas.GetCount() == 17);
    SpriteRef slice0 = atlas[std::string("tile_0")];
    assert(!slice0.isNull());
    assert(slice0->width() == 16);
    assert(slice0->height() == 16);

    // 6. Terrain2D & Colliders & Surface Painting
    std::cout << "[6/8] Testing Terrain2D Mesh, Colliders & Surface Painting..." << std::endl;
    GameObjectRef terrainObj = Primitive::CreateEmptyGameObject("TerrainObj");
    Ref<Terrain2D> terrain = terrainObj->AddComponent<Terrain2D>();
    assert(!terrain.isNull());
    terrain->setSize(Vec2(64.0f, 64.0f));
    terrain->setMeshTile(0, 0, slice0);
    terrain->setMeshTile(1, 0, slice0);
    assert(!terrain->getMeshTile(0, 0).isNull());
    assert(!terrain->getMeshTile(1, 0).isNull());
    assert(terrain->removeMeshTile(1, 0) == true);
    assert(terrain->getMeshTile(1, 0).isNull());

    terrain->fillMeshTiles(0, 0, 4, 4, slice0, Vec2::one, false);
    assert(terrain->getTileCount() >= 16);

    // Surface Canvas & Texture Painting API
    terrain->createSurface(128, 128, Color(20, 40, 60, 255));
    assert(terrain->hasSurface() == true);
    assert(terrain->getSurface() != nullptr);

    terrain->paintPixel(10, 10, Color(255, 0, 128, 255));
    Color px = terrain->getPixel(10, 10);
    assert(px.r == 255 && px.b == 128);

    terrain->paintRect(Rectf(15.0f, 15.0f, 10.0f, 10.0f), Color(0, 255, 0, 255));
    terrain->paintBrush(Vec2(35.0f, 35.0f), 8.0f, Color(255, 255, 0, 255), 0.75f);
    terrain->eraseBrush(Vec2(35.0f, 35.0f), 4.0f, 0.9f);

    SpriteRef surfSpr = terrain->getSurfaceSprite();
    assert(!surfSpr.isNull());

    terrain->setCollider(Rectf(0, 0, 4, 4), true);
    assert(terrain->is_collider(Vec2(2, 2)) == true);
    assert(terrain->is_collider(Vec2(25, 25)) == false);

    // Circle collider test
    terrain->setColliderCircle(Vec2(-2, -2), 1.5f, true);
    assert(terrain->is_collider(Vec2(-2, -2)) == true);
    assert(terrain->is_collider(Vec2(-2, 0)) == false);

    // Tile collider test
    terrain->setColliderTile(3, 3, true, Vec2::one);
    assert(terrain->is_collider(3, 3, Vec2::one) == true);
    terrain->setColliderTile(3, 3, false, Vec2::one);

    // Border colliders
    terrain->setBorderColliders(true, 0.5f);

    // NavMesh locks & pathfinding integration
    AI::NavMesh *tNav = terrain->get_navmesh2D();
    assert(tNav != nullptr);

    // Clear colliders and test obstacle avoidance pathfinding
    terrain->clearColliders();
    assert(terrain->is_collider(Vec2(2, 2)) == false);

    // Place an obstacle wall between start (-4, 0) and goal (4, 0)
    terrain->setCollider(Rectf(-0.5f, -3.0f, 1.0f, 6.0f), true);
    assert(terrain->is_collider(Vec2(0, 0)) == true);

    std::vector<Vec2> worldPath;
    bool pathFound = terrain->findPath(Vec2(-3.0f, 0.0f), Vec2(3.0f, 0.0f), worldPath);
    assert(pathFound == true);
    assert(worldPath.size() >= 2);
    // Path must navigate around the obstacle wall without any point colliding
    for (const auto &p : worldPath)
    {
        assert(!terrain->is_collider(p));
    }

    // Find free space near an obstacle
    Vec2 tFreePt = terrain->findFreeSpace(Vec2(0.0f, 0.0f), 4.0f);
    assert(!terrain->is_collider(tFreePt));

    terrain->clearColliders();

    // 7. Light Components
    std::cout << "[7/8] Testing 2D Lights (SimpleLight, Spotlight, DirectLight)..." << std::endl;
    GameObjectRef lightObj = Primitive::CreateEmptyGameObject("Lights");
    Ref<SimpleLight> sLight = lightObj->AddComponent<SimpleLight>();
    sLight->range = 50.0f;
    sLight->intensity = 0.9f;
    sLight->color = Color::yellow;
    assert(sLight->range == 50.0f);
    assert(sLight->intensity == 0.9f);

    Ref<Spotlight> spLight = lightObj->AddComponent<Spotlight>();
    spLight->angle = 60.0f;
    spLight->range = 40.0f;
    assert(spLight->angle == 60.0f);

    Ref<DirectLight> dLight = lightObj->AddComponent<DirectLight>();
    dLight->intensity = 0.75f;
    assert(dLight->intensity == 0.75f);

    // 8. UI System
    std::cout << "[8/8] Testing GUI & Interactive Controls..." << std::endl;
    GUI *gui = worldA.GetGUI();
    assert(gui != nullptr);

    uid grp = gui->PushGroup(Rect(0, 0, 400, 400));
    assert(gui->IsGroup(grp) == true);

    uid btn = gui->PushButton("Start Game", Vec2Int(20, 20), nullptr, grp);
    assert(gui->ElementContains(btn) == true);
    assert(gui->IsGroup(btn) == false);

    uid lbl = gui->PushLabel("Player Status", Vec2Int(20, 60), 13, grp);
    assert(gui->ElementGetText(lbl) == "Player Status");
    gui->ElementSetText(lbl, "Updated Status");
    assert(gui->ElementGetText(lbl) == "Updated Status");

    uid sld = gui->PushSlider(0.5f, 0.0f, 100.0f, Vec2Int(20, 100), nullptr, grp);
    assert(Math::Abs(gui->SliderGetValue(sld) - 0.5f) < 0.001f);
    gui->SliderSetValue(sld, 75.0f);
    assert(Math::Abs(gui->SliderGetValue(sld) - 75.0f) < 0.001f);

    uid chk = gui->PushCheckBox(true, "Mute Music", Vec2Int(20, 140), nullptr, grp);
    assert(gui->CheckBoxGetValue(chk) == true);
    gui->CheckBoxSetValue(chk, false);
    assert(gui->CheckBoxGetValue(chk) == false);

    bool popped = gui->PopElement(btn);
    assert(popped == true);
    assert(gui->ElementContains(btn) == false);

    // 9. MultiCollider2D Component & SAT Collision System
    std::cout << "[9/9] Testing MultiCollider2D (Vertices, Presets, SAT, Raycast, ContainsPoint)..." << std::endl;
    GameObjectRef polyObjA = Primitive::CreateEmptyGameObject("PolyA");
    MultiCollider2DRef colA = polyObjA->AddComponent<MultiCollider2D>();
    assert(colA != nullptr);

    // Initial state: default 1x1 box preset (4 vertices)
    assert(colA->getPointCount() == 4);
    assert(colA->isClosed == true);

    // Clear and point manipulation
    colA->clear();
    assert(colA->getPointCount() == 0);
    colA->addPoint({0.0f, 0.0f});
    colA->addPoint({2.0f, 0.0f});
    colA->addPoint({2.0f, 2.0f});
    colA->addPoint({0.0f, 2.0f});
    assert(colA->getPointCount() == 4);
    assert(colA->getPoint(1) == Vec2(2.0f, 0.0f));

    colA->setPoint(1, {3.0f, 0.0f});
    assert(colA->getPoint(1) == Vec2(3.0f, 0.0f));
    colA->setPoint(1, {2.0f, 0.0f});

    colA->insertPoint(2, {2.5f, 1.0f});
    assert(colA->getPointCount() == 5);
    assert(colA->getPoint(2) == Vec2(2.5f, 1.0f));
    colA->removePoint(2);
    assert(colA->getPointCount() == 4);

    // Presets
    colA->setBox(2.0f, 2.0f);
    assert(colA->getPointCount() == 4);
    assert(std::abs(colA->computeArea() - 4.0f) < 0.001f);
    assert(colA->isConvex() == true);
    Vec2 centroid = colA->computeCentroid();
    assert(std::abs(centroid.x) < 0.001f && std::abs(centroid.y) < 0.001f);

    // Containment
    assert(colA->containsPoint({0.0f, 0.0f}) == true);
    assert(colA->containsPoint({0.5f, 0.5f}) == true);
    assert(colA->containsPoint({2.0f, 0.0f}) == false);

    // SAT Intersections with another MultiCollider2D
    GameObjectRef polyObjB = Primitive::CreateEmptyGameObject("PolyB");
    MultiCollider2DRef colB = polyObjB->AddComponent<MultiCollider2D>();
    colB->setBox(2.0f, 2.0f);
    polyObjB->transform()->position(Vec2(1.5f, 0.0f));

    // Overlapping boxes (colA is [-1, 1], colB is [0.5, 2.5])
    assert(colA->intersects(colB) == true);
    assert(colB->intersects(colA) == true);

    // Move away -> separated
    polyObjB->transform()->position(Vec2(5.0f, 0.0f));
    assert(colA->intersects(colB) == false);
    assert(colB->intersects(colA) == false);

    // SAT Intersections with BoxCollider2D
    GameObjectRef boxObj = Primitive::CreateEmptyGameObject("BoxObj");
    BoxCollider2DRef boxCol = boxObj->AddComponent<BoxCollider2D>();
    boxCol->size = {2.0f, 2.0f};
    boxObj->transform()->position(Vec2(0.5f, 0.5f));
    assert(colA->intersects(boxCol) == true);

    boxObj->transform()->position(Vec2(10.0f, 0.0f));
    assert(colA->intersects(boxCol) == false);

    // Circle Distance Check
    assert(colA->intersectsCircle(Vec2(0.0f, 0.0f), 0.5f) == true);
    assert(colA->intersectsCircle(Vec2(5.0f, 0.0f), 0.5f) == false);

    // Raycast
    Vec2 hitPt;
    float hitDist = 0.0f;
    bool didHit = colA->raycast(Vec2(-3.0f, 0.0f), Vec2(1.0f, 0.0f), hitPt, hitDist);
    assert(didHit == true);
    assert(std::abs(hitDist - 2.0f) < 0.01f);
    assert(std::abs(hitPt.x - -1.0f) < 0.01f);

    bool didMiss = colA->raycast(Vec2(-3.0f, 0.0f), Vec2(0.0f, 1.0f), hitPt, hitDist);
    assert(didMiss == false);

    // Triangle preset & Regular Polygon & Star presets
    colA->setTriangle(2.0f, 2.0f);
    assert(colA->getPointCount() == 3);
    assert(colA->isConvex() == true);

    colA->setRegularPolygon(6, 1.0f);
    assert(colA->getPointCount() == 6);
    assert(colA->isConvex() == true);

    colA->setStar(5, 2.0f, 1.0f);
    assert(colA->getPointCount() == 10);
    assert(colA->isConvex() == false);

    std::cout << "========================================" << std::endl;
    std::cout << "All Comprehensive Unit Tests PASSED! [9/9]" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
